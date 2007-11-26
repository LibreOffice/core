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
        stmfd sp!, {r0-r3}         @  follow other parameters on stack
        stmfd sp!, {fp,ip,lr}
	add fp, sp, #28

	mov   r0, ip               @ r0 points to functionoffset/vtable
        add   r1, sp, #12           @ r1 points to this and params
        bl    cpp_vtable_call(PLT)

        add  sp, sp, #28           @ restore stack
        ldr  fp, [sp, #-28]        @ restore fp
        ldr  pc, [sp, #-20]        @ return
