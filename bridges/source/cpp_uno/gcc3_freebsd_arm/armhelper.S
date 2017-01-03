@   
@   Licensed to the Apache Software Foundation (ASF) under one
@   or more contributor license agreements.  See the NOTICE file
@   distributed with this work for additional information
@   regarding copyright ownership.  The ASF licenses this file
@   to you under the Apache License, Version 2.0 (the
@   "License"); you may not use this file except in compliance
@   with the License.  You may obtain a copy of the License at
@   
@     http://www.apache.org/licenses/LICENSE-2.0
@   
@   Unless required by applicable law or agreed to in writing,
@   software distributed under the License is distributed on an
@   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
@   KIND, either express or implied.  See the License for the
@   specific language governing permissions and limitations
@   under the License.
@  

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
