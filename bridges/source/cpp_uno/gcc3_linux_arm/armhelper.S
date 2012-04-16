@ ARM support code for OpenOffice C++/UNO bridging
@
@ Written by Peter Naulls <peter@chocky.org>
@ Modified by Caolan McNamara <caolanm@redhat.com>
@ Fixed by Michael Casadevall <mcasadevall@kubuntu.org>

#ifdef __ARM_EABI__
#  define UNWIND
#else
#  define UNWIND @
#endif

@ If the VFP ABI variant (armhf in Debian/Ubuntu) is used, an additional extra 64 bytes
@ are taken up on the stack (the equivalent of the 8 double precision VFP registers)

#ifdef __ARM_PCS_VFP
#  define PAD 80
#  define DISCARDED 84
#else
#  define PAD 16
#  define DISCARDED 20
#endif

	.file	"armhelper.s"
	.text
	.align	4
	.global privateSnippetExecutor
	.type privateSnippetExecutor, %function
privateSnippetExecutor:
	UNWIND .fnstart            @ start of unwinder entry

	stmfd sp!, {r0-r3}         @ follow other parameters on stack
	mov   r0, ip               @ r0 points to functionoffset/vtable
	mov   r1, sp               @ r1 points to this and params
#ifdef __ARM_PCS_VFP
	vpush {d0-d7}              @ floating point parameter on stack
#endif
	UNWIND .pad  #PAD          @ throw this data away on exception
	                           @ (see cppuno.cxx:codeSnippet())
	stmfd sp!, {r4,lr}         @ save return address 
	                           @ (r4 pushed to preserve stack alignment)
	UNWIND .save {r4,lr}       @ restore these regs on exception

	bl    cpp_vtable_call(PLT)

	add   sp, sp, #4           @ no need to restore r4 (we didn't touch it)
	ldr   pc, [sp], #DISCARDED @ return, discarding function arguments

	UNWIND .fnend              @ end of unwinder entry

	.size privateSnippetExecutor, . - privateSnippetExecutor
        .section        .note.GNU-stack,"",%progbits
