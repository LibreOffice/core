;*************************************************************************
;
;   OpenOffice.org - a multi-platform office productivity suite
;
;   $RCSfile: os2.asm,v $
;
;   $Revision: 1.3 $
;
;   last change: $Author: rt $ $Date: 2005-09-07 21:40:27 $
;
;   The Contents of this file are made available subject to
;   the terms of GNU Lesser General Public License Version 2.1.
;
;
;     GNU Lesser General Public License Version 2.1
;     =============================================
;     Copyright 2005 by Sun Microsystems, Inc.
;     901 San Antonio Road, Palo Alto, CA 94303, USA
;
;     This library is free software; you can redistribute it and/or
;     modify it under the terms of the GNU Lesser General Public
;     License version 2.1, as published by the Free Software Foundation.
;
;     This library is distributed in the hope that it will be useful,
;     but WITHOUT ANY WARRANTY; without even the implied warranty of
;     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;     Lesser General Public License for more details.
;
;     You should have received a copy of the GNU Lesser General Public
;     License along with this library; if not, write to the Free Software
;     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
;     MA  02111-1307  USA
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
