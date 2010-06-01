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

	.file	"armhelper.s"
	.text
	.align	4
	.global privateSnippetExecutor
	.type privateSnippetExecutor, %function
privateSnippetExecutor:
	UNWIND .fnstart            @ start of unwinder entry

	stmfd sp!, {r0-r3}         @ follow other parameters on stack
	UNWIND .pad  #16           @ throw this data away on exception
	mov   r0, ip               @ r0 points to functionoffset/vtable
	mov   r1, sp               @ r1 points to this and params
	                           @ (see cppuno.cxx:codeSnippet())
	stmfd sp!, {r4,lr}         @ save return address 
	                           @ (r4 pushed to preserve stack alignment)
	UNWIND .save {r4,lr}       @ restore these regs on exception

	bl    cpp_vtable_call(PLT)

	add   sp, sp, #4           @ no need to restore r4 (we didn't touch it)
	ldr   pc, [sp], #20        @ return, discarding function arguments

	UNWIND .fnend              @ end of unwinder entry

	.size privateSnippetExecutor, . - privateSnippetExecutor
        .section        .note.GNU-stack,"",%progbits
