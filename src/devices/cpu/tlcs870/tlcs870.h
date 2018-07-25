// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_TLCS870_TLCS870_H
#define MAME_CPU_TLCS870_TLCS870_H

#pragma once

class tlcs870_device : public cpu_device
{
protected:
	// construction/destruction
	tlcs870_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 2; }
	virtual uint32_t execute_max_cycles() const override { return 26; }
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void tmp87ph40an_mem(address_map &map);

	uint32_t m_debugger_temp;

private:

	enum _regs8
	{
		REG_A,
		REG_W,
		REG_C,
		REG_B,
		REG_E,
		REG_D,
		REG_L,
		REG_H
	};

	enum _regs16
	{
		REG_WA,
		REG_BC,
		REG_DE,
		REG_HL
	};

	enum _regs_debugger
	{
		DEBUGGER_REG_A,
		DEBUGGER_REG_W,
		DEBUGGER_REG_C,
		DEBUGGER_REG_B,
		DEBUGGER_REG_E,
		DEBUGGER_REG_D,
		DEBUGGER_REG_L,
		DEBUGGER_REG_H,
		DEBUGGER_REG_WA,
		DEBUGGER_REG_BC,
		DEBUGGER_REG_DE,
		DEBUGGER_REG_HL
	};

	enum _conditions
	{
		COND_EQ_Z,
		COND_NE_NZ,
		COND_LT_CS,
		COND_GE_CC,
		COND_LE,
		COND_GT,
		COND_T,
		COND_F
	};

	enum _srcdst_addressingmode
	{
		ADDR_IN_IMM_X,
		ADDR_IN_PC_PLUS_REG_A,
		ADDR_IN_DE,
		ADDR_IN_HL,
		ADDR_IN_HL_PLUS_IMM_D,
		ADDR_IN_HL_PLUS_REG_C,
		ADDR_IN_HLINC,
		ADDR_IN_DECHL
	};

	address_space_config m_program_config;
	address_space_config m_io_config;
	required_shared_ptr<uint8_t> m_intram;

	PAIR        m_prvpc, m_pc, m_sp;

	address_space *m_program;
	address_space *m_io;
	int     m_icount;

	// Work registers
	uint8_t m_cycles;
	uint16_t m_tmppc;
	uint32_t  m_addr;

	uint16_t m_F;

	/* CPU registers */
	uint8_t m_RBS; // register base (4-bits)

	uint8_t  RM8(const uint32_t a) { return m_program->read_byte(a); }
	uint16_t RM16(const uint32_t a) { return RM8(a) | (RM8((a + 1) & 0xffff) << 8); }

	void WM8(const uint32_t a, const uint8_t  v) { m_program->write_byte(a, v); }
	void WM16(const uint32_t a, const uint16_t v) { WM8(a, v);    WM8((a + 1) & 0xffff, v >> 8); }

	uint8_t  RX8(const uint32_t a, const uint32_t base) { return m_program->read_byte(base | a); }
	uint16_t RX16(const uint32_t a, const uint32_t base) { return RX8(a, base) | (RX8((a + 1) & 0xffff, base) << 8); }

	void WX8(const uint32_t a, const uint8_t  v, const uint32_t base) { m_program->write_byte(base | a, v); }
	void WX16(const uint32_t a, const uint16_t v, const uint32_t base) { WX8(a, v, base);   WX8((a + 1) & 0xffff, v >> 8, base); }

	uint8_t  READ8() { const uint8_t b0 = RM8(m_addr++); m_addr &= 0xffff; return b0; }
	uint16_t READ16() { const uint8_t b0 = READ8(); return b0 | (READ8() << 8); }

	uint16_t get_addr(uint16_t opbyte0, uint16_t val);

	const uint8_t get_reg8(const int reg);
	void set_reg8(const int reg, uint8_t val);
	const uint16_t get_reg16(const int reg);
	void set_reg16(const int reg, uint16_t val);

	static constexpr int FLAG_J = 0x80;
	static constexpr int FLAG_Z = 0x40;
	static constexpr int FLAG_C = 0x20;
	static constexpr int FLAG_H = 0x10;

	void clear_JF() { m_F &= ~FLAG_J; }
	void clear_ZF() { m_F &= ~FLAG_Z; }
	void clear_CF() { m_F &= ~FLAG_C; }
	void clear_HF() { m_F &= ~FLAG_H; }

	void set_JF() { m_F |= FLAG_J; }
	void set_ZF() { m_F |= FLAG_Z; }
	void set_CF() { m_F |= FLAG_C; }
	void set_HF() { m_F |= FLAG_H; }

	int is_JF() const { return ((m_F & FLAG_J) ? 1 : 0); }
	int is_ZF() const { return ((m_F & FLAG_Z) ? 1 : 0); }
	int is_CF() const { return ((m_F & FLAG_C) ? 1 : 0); }
	int is_HF() const { return ((m_F & FLAG_H) ? 1 : 0); }

	bool stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb);

	// tlcs870_ops.cpp
	void decode();

	void do_illegal(const uint8_t opbyte0);

	void do_NOP(const uint8_t opbyte0);
	void do_SWAP_A(const uint8_t opbyte0);
	void do_MUL_W_A(const uint8_t opbyte0);
	void do_DIV_WA_C(const uint8_t opbyte0);
	void do_RETI(const uint8_t opbyte0);
	void do_RET(const uint8_t opbyte0);
	void do_POP_PSW(const uint8_t opbyte0);
	void do_PUSH_PSW(const uint8_t opbyte0);

	void do_DAA_A(const uint8_t opbyte0);
	void do_DAS_A(const uint8_t opbyte0);
	void do_CLR_CF(const uint8_t opbyte0);
	void do_SET_CF(const uint8_t opbyte0);
	void do_CPL_CF(const uint8_t opbyte0);
	void do_LD_RBS_n(const uint8_t opbyte0);
	void do_INC_rr(const uint8_t opbyte0);
	void do_LD_rr_mn(const uint8_t opbyte0);
	void do_DEC_rr(const uint8_t opbyte0);
	void do_SHLC_A(const uint8_t opbyte0);
	void do_SHRC_A(const uint8_t opbyte0);
	void do_ROLC_A(const uint8_t opbyte0);
	void do_RORC_A(const uint8_t opbyte0);
	void do_INC_inx(const uint8_t opbyte0);
	void do_INC_inHL(const uint8_t opbyte0);
	void do_LD_A_inx(const uint8_t opbyte0);
	void do_LD_A_inHL(const uint8_t opbyte0);
	void do_LDW_inx_mn(const uint8_t opbyte0);
	void do_LDW_inHL_mn(const uint8_t opbyte0);
	void do_LD_inx_iny(const uint8_t opbyte0);

	void do_DEC_inx(const uint8_t opbyte0);
	void do_DEC_inHL(const uint8_t opbyte0);
	void do_LD_inx_A(const uint8_t opbyte0);
	void do_LD_inHL_A(const uint8_t opbyte0);
	void do_LD_inx_n(const uint8_t opbyte0);
	void do_LD_inHL_n(const uint8_t opbyte0);
	void do_CLR_inx(const uint8_t opbyte0);
	void do_CLR_inHL(const uint8_t opbyte0);
	void do_LD_r_n(const uint8_t opbyte0);

	void do_SET_inxbit(const uint8_t opbyte0);
	void do_CLR_inxbit(const uint8_t opbyte0);
	void do_LD_A_r(const uint8_t opbyte0);
	void do_LD_r_A(const uint8_t opbyte0);
	void do_INC_r(const uint8_t opbyte0);
	void do_DEC_r(const uint8_t opbyte0);
	void do_ALUOP_A_n(const uint8_t opbyte0);
	void do_ALUOP_A_inx(const uint8_t opbyte0);
	void do_JRS_T_a(const uint8_t opbyte0);
	void do_JRS_F_a(const uint8_t opbyte0);
	void do_CALLV_n(const uint8_t opbyte0);
	void do_JR_cc_a(const uint8_t opbyte0);
	void do_LD_CF_inxbit(const uint8_t opbyte0);

	void do_LD_SP_mn(const uint8_t opbyte0);
	void do_JR_a(const uint8_t opbyte0);
	void do_CALL_mn(const uint8_t opbyte0);
	void do_CALLP_n(const uint8_t opbyte0);
	void do_JP_mn(const uint8_t opbyte0);
	void do_ff_opcode(const uint8_t opbyte0);

	// tlcs870_ops_src.cpp

	void do_e0_opcode(const uint8_t opbyte0);
	void do_e1_to_e3_opcode(const uint8_t opbyte0);
	void do_e4_opcode(const uint8_t opbyte0);
	void do_e5_to_e7_opcode(const uint8_t opbyte0);

	void do_e0_to_e7_opcode(uint8_t opbyte0, uint16_t srcaddr);

	void do_e0_to_e7_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);

	void do_ROLD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_RORD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_rr_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_INC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_inx_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_inHL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_DEC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_MCMP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_SET_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CLR_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_insrc_inHL(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_XCH_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CPL_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_insrcbit_CF(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_XOR_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CALL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_JP_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);

	uint16_t do_alu(int op, uint16_t param1, uint16_t param2);

	// tlcs870_ops_dst.cpp

	void do_f0_opcode(const uint8_t opbyte0);
	void do_f2_to_f3_opcode(const uint8_t opbyte0);
	void do_f4_opcode(const uint8_t opbyte0);
	void do_f6_to_f7_opcode(const uint8_t opbyte0);

	void do_f0_to_f7_opcode(const uint8_t opbyte0, const uint16_t dstaddr);

	void do_f0_to_f7_oprand_illegal_opcode(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);

	void do_LD_indst_rr(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);
	void do_LD_indst_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);
	void do_LD_indst_r(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);

	// tlcs870_ops_reg.cpp

	void do_regprefixtype_opcode(const uint8_t opbyte0);

	void do_regprefixtype_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1);

	void do_SWAP_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_MUL_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DIV_gg_C(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_RETN(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_POP_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_PUSH_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DAA_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DAS_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XCH_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SHLC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SHRC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ROLC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_RORC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_WA_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_gg_mn(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SET_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CLR_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_r_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_A_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_g_A(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_g_n(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SET_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CLR_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CPL_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_inppbit_CF(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_CF_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XCH_r_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CPL_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_gbit_CF(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XOR_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_SP_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_gg_SP(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CALL_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_JP_gg(const uint8_t opbyte0, const uint8_t opbyte1);

	void handle_div(const int reg);
	void handle_mul(const int reg);
	void handle_swap(const int reg);

	uint8_t handle_SHLC(uint8_t val);
	uint8_t handle_SHRC(uint8_t val);
	uint8_t handle_DAS(uint8_t val);
	uint8_t handle_DAA(uint8_t val);
	uint8_t handle_ROLC(uint8_t val);
	uint8_t handle_RORC(uint8_t val);
	void handle_take_interrupt(int level);

	const bool check_jump_condition(int param1);

	const uint8_t get_PSW();
	void set_PSW(uint8_t data);
};


class tmp87ph40an_device : public tlcs870_device
{
public:
	// construction/destruction
	tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device)

#endif // MAME_CPU_TLCS870_TLCS870_H
