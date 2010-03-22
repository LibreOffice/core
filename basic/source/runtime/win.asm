;*************************************************************************
;
; DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
; 
; Copyright 2000, 2010 Oracle and/or its affiliates.
;
; OpenOffice.org - a multi-platform office productivity suite
;
; This file is part of OpenOffice.org.
;
; OpenOffice.org is free software: you can redistribute it and/or modify
; it under the terms of the GNU Lesser General Public License version 3
; only, as published by the Free Software Foundation.
;
; OpenOffice.org is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU Lesser General Public License version 3 for more details
; (a copy is included in the LICENSE file that accompanied this code).
;
; You should have received a copy of the GNU Lesser General Public License
; version 3 along with OpenOffice.org.  If not, see
; <http://www.openoffice.org/license.html>
; for a copy of the LGPLv3 License.
;
;*************************************************************************

;    Anmerkungen
;        Direktaufruf von C- und PASCAL-Routinen, Windows und OS/2
;
; Inhalt:
; type = CallXXX (far *proc, char *stack, short nstack)
;
; Kopie des Basic-Stacks (nstack Bytes) auf den C-Stack
; und Aufruf der Prozedur.

		.MODEL	LARGE,C

		.CODE

		PUBLIC	CallINT
		PUBLIC	CallLNG
		PUBLIC	CallSNG
		PUBLIC	CallDBL
		PUBLIC	CallSTR
		PUBLIC	CallFIX

CallINT		LABEL	byte
CallLNG		LABEL	byte
CallSNG		LABEL	byte
CallDBL		LABEL	byte
CallSTR		LABEL	byte
CallFIX		PROC	p:PTR,stk:PTR,n:WORD

		PUSH	SI
		PUSH	DI
		MOV	DX,DS
		SUB	SP,[n]
		MOV	DI,SP
		MOV	AX,SS
		MOV	ES,AX
		LDS	SI,[stk]
		MOV	CX,[n]
		SHR	CX,1
		CLD
		JCXZ	$1
	REP	MOVSW			; Stack uebernehmen
$1:		MOV	DS,DX
		CALL	[p]		; Aufruf der Prozedur
		CLI
		MOV	SP,BP
		SUB	SP,4		; wegen gepushter Register
		STI
		POP	DI
		POP	SI
		RET

CallFIX		ENDP

		END
