;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;    OS2.ASM
;;
;;    Ersterstellung  MD 30.05.94
;;
;;    Anmerkungen
;;        Direktaufruf von C- und PASCAL-Routinen, OS/2
;;
;;    Source Code Control System - Header
;;        $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/basic/source/runtime/os2.asm,v 1.1.1.1 2000-09-18 16:12:11 hr Exp $
;;
;;    Copyright (c) 1990,95 by STAR DIVISION GmbH
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
