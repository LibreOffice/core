;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  $RCSfile: wnt.asm,v $
;;
;;  $Revision: 1.2 $
;;
;;  last change: $Author: mh $ $Date: 2000-11-30 08:39:31 $
;;
;;  The Contents of this file are made available subject to the terms of
;;  either of the following licenses
;;
;;         - GNU Lesser General Public License Version 2.1
;;         - Sun Industry Standards Source License Version 1.1
;;
;;  Sun Microsystems Inc., October, 2000
;;
;;  GNU Lesser General Public License Version 2.1
;;  =============================================
;;  Copyright 2000 by Sun Microsystems, Inc.
;;  901 San Antonio Road, Palo Alto, CA 94303, USA
;;
;;  This library is free software; you can redistribute it and/or
;;  modify it under the terms of the GNU Lesser General Public
;;  License version 2.1, as published by the Free Software Foundation.
;;
;;  This library is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;  Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public
;;  License along with this library; if not, write to the Free Software
;;  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
;;  MA  02111-1307  USA
;;
;;
;;  Sun Industry Standards Source License Version 1.1
;;  =================================================
;;  The contents of this file are subject to the Sun Industry Standards
;;  Source License Version 1.1 (the "License"); You may not use this file
;;  except in compliance with the License. You may obtain a copy of the
;;  License at http://www.openoffice.org/license.html.
;;
;;  Software provided under this License is provided on an "AS IS" basis,
;;  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
;;  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
;;  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
;;  See the License for the specific provisions governing your rights and
;;  obligations concerning the Software.
;;
;;  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
;;
;;  Copyright: 2000 by Sun Microsystems, Inc.
;;
;;  All Rights Reserved.
;;
;;  Contributor(s): _______________________________________
;;
;;    Anmerkungen
;;        Direktaufruf von C- und PASCAL-Routinen, Windows und OS/2
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Inhalt:
; type = CallXXX (far *pProc, char *pStack, short nStack)
;
; Kopie des Basic-Stacks (nStack Bytes) auf den C-Stack
; und Aufruf der Prozedur.

		.386

_TEXT		SEGMENT	DWORD PUBLIC 'CODE' USE32

		ASSUME 	CS:_TEXT

		PUBLIC	_CallINT@12
		PUBLIC	_CallLNG@12
		PUBLIC	_CallDBL@12
		PUBLIC	_CallSTR@12
		PUBLIC	_CallFIX@12

_CallINT@12	LABEL	byte
_CallLNG@12	LABEL	byte
_CallDBL@12	LABEL	byte
_CallSTR@12	LABEL	byte

_CallFIX@12:	PUSH	EBP
		MOV	EBP,ESP
		PUSH	ESI
		PUSH	EDI

		PUSH	ECX
		PUSH	EDX

		MOV	DX,DS
		MOVZX	EAX,WORD PTR [EBP+16] ; EAX == nStack
		SUB	ESP,EAX       ; Stack um nStack Bytes vergroessern
		MOV	EDI,ESP
		MOV	AX,SS
		MOV	ES,AX	      ; ES:EDI = Startadresse des fuer
				      ; Parameter reservierten Stackbereichs
		MOV	ESI,[EBP+12]  ; DS:ESI == pStack

		MOVZX	ECX,WORD PTR [EBP+16] ; ECX == nStack
		SHR	ECX,1
		CLD
		JCXZ	$1
	REP	MOVSW		      ; Stack uebernehmen
$1:		MOV	DS,DX
		CALL	DWORD PTR [EBP+8]	; Aufruf der Prozedur
		; CLI ; unter NT nicht erlaubt (privileged instruction)
		MOV	ESP,EBP
		SUB	ESP,16	      ; wegen gepushter Register
				      ; (ESI, EDI)
		; STI
		POP	EDX
		POP	ECX
		POP	EDI
		POP	ESI
		POP	EBP
		RET	12

_TEXT		ENDS

		END
