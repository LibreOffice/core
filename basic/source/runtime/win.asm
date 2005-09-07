;*************************************************************************
;
;   OpenOffice.org - a multi-platform office productivity suite
;
;   $RCSfile: win.asm,v $
;
;   $Revision: 1.3 $
;
;   last change: $Author: rt $ $Date: 2005-09-07 21:42:58 $
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
