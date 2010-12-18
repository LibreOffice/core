/* ia64 support code for OpenOffice C++/UNO bridging
 *
 * Caolan McNamara <caolanm@redhat.com>
 */
	.text
	.align 16
	.global privateSnippetExecutor#
	.proc privateSnippetExecutor#
privateSnippetExecutor:
	adds r15 = 8, gp	/* r15 now points to real gp value*/
	;;
	ld8 r14 = [gp]		/* load nOffsetAndIndex into a handy register */
	ld8 gp = [r15]		/* load real gp value into gp */
	;;
	/* store the address where large structs are "returned" into a handy register */
	mov r15 = r8
	;;
	br cpp_vtable_call#	/* call cpp_vtable_call which'll suck out r14 */
	;;
	.endp privateSnippetExecutor#
