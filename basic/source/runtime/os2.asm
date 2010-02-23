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
;        Direktaufruf von C- und PASCAL-Routinen, OS/2
;
; Inhalt:
; type = CallXXX (far *proc, char *stack, short nstack)
;
; Kopie des Basic-Stacks (nstack Bytes) auf den C-Stack
; und Aufruf der Prozedur.

		.386
		.MODEL	FLAT

		.CODE

		PUBLIC	CallINT
		PUBLIC	CallLNG
		PUBLIC	CallSNG
		PUBLIC	CallDBL
		PUBLIC	CallSTR
		PUBLIC	CallFIX

		PUBLIC	_CallINT
		PUBLIC	_CallLNG
		PUBLIC	_CallSNG
		PUBLIC	_CallDBL
		PUBLIC	_CallSTR
		PUBLIC	_CallFIX

_CallINT	LABEL	byte
_CallLNG	LABEL	byte
_CallSNG	LABEL	byte
_CallDBL	LABEL	byte
_CallSTR	LABEL	byte
_CallFIX	LABEL	byte

CallINT		LABEL	byte
CallLNG		LABEL	byte
CallSNG		LABEL	byte
CallDBL		LABEL	byte
CallSTR		LABEL	byte
CallFIX		PROC

p		EQU	[EBP+8]
stk		EQU	[EBP+12]
n		EQU	[EBP+16]

		PUSH	EBP
		MOV	EBP,ESP
		PUSH	ESI
		PUSH	EDI
		MOV	DX,DS
		MOVZX	ECX,word ptr [n]
		SUB	ESP,ECX
		MOV	EDI,ESP
		MOV	AX,SS
		MOV	ES,AX
		MOV	ESI,[stk]
		SHR	ECX,1
		CLD
		JCXZ	$1
	REP	MOVSW			; Stack uebernehmen
$1:		MOV	DS,DX
		CALL	LARGE [p]	; 32-bit
		MOV	ECX,EBP
		SUB	ECX,8		; wegen gepushter Register
		MOV	ESP,ECX
		POP	EDI
		POP	ESI
		POP	EBP
; Bei Borland C++ Calling Convention:
;		RET	12
; CSet System-Calling Convention
		RET
CallFIX		ENDP

		END
