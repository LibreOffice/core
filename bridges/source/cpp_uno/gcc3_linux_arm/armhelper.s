@ ARM support code for OpenOffice C++/UNO bridging
@
@ Written by Peter Naulls <peter@chocky.org>
@ Modified by Caolan McNamara <caolanm@redhat.com>
	.file	"armhelper.s"
	.text
	.align	4
	.global privateSnippetExecutor
	.type privateSnippetExecutor, %function
privateSnippetExecutor:
        stmfd sp!, {r0-r3}         @ follow other parameters on stack
	mov   r0, ip               @ r0 points to functionoffset/vtable
        mov   ip, sp		   @ fix up the ip
        stmfd sp!, {fp,ip,lr,pc}   @ 8 x 4 => stack remains 8 aligned
	sub   fp, ip, #4	   @ set frame pointer

        add   r1, sp, #16          @ r1 points to this and params
        bl    cpp_vtable_call(PLT)

        add  sp, sp, #32           @ restore stack
        ldr  fp, [sp, #-32]	   @ restore frame pointer
        ldr  pc, [sp, #-24]        @ return
