;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;    WINOS2.ASM
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
;;        $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/basic/source/runtime/win.asm,v 1.1.1.1 2000-09-18 16:12:11 hr Exp $
;;
;;    Copyright (c) 1990,95 by STAR DIVISION GmbH
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
