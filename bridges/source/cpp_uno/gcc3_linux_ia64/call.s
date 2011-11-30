/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

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
