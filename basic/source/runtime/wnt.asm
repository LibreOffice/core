;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;    WNT.ASM
;;
;;    Ersterstellung  MD 26.02.91
;;
;;    Stand
;;        XX in Arbeit
;;        XX fertiggestellt
;;        __ abgenommen
;;        __ freigegeben
;;
;;    Anmerkungen
;;        Direktaufruf von C- und PASCAL-Routinen, Windows und OS/2
;;
;;    Source Code Control System - Header
;;        $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/basic/source/runtime/wnt.asm,v 1.1.1.1 2000-09-18 16:12:11 hr Exp $
;;
;;    Copyright (c) 1990,95 by STAR DIVISION GmbH
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
