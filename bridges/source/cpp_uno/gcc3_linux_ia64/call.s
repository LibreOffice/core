/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* ia64 support code for OpenOffice C++/UNO bridging
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
