#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: wnt.asm,v $
#
# $Revision: 1.4 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY# without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#    Anmerkungen
#        Direktaufruf von C- und PASCAL-Routinen, Windows und OS/2
#
# Inhalt:
# type = CallXXX (far *pProc, char *pStack, short nStack)
#
# Kopie des Basic-Stacks (nStack Bytes) auf den C-Stack
# und Aufruf der Prozedur.

		.intel_syntax
		#.386

#_TEXT		SEGMENT	DWORD PUBLIC 'CODE' USE32

		#ASSUME 	CS:_TEXT

.globl		_CallINT
.globl		_CallLNG
.globl		_CallDBL
.globl		_CallSTR
.globl		_CallFIX

_CallINT:
_CallLNG:
_CallDBL:
_CallSTR:

_CallFIX:	PUSH	EBP
		MOV	EBP,ESP
		PUSH	ESI
		PUSH	EDI

		PUSH	ECX
		PUSH	EDX

		MOV	DX,DS
		MOVZX	EAX,WORD PTR [EBP+16] # EAX == nStack
		SUB	ESP,EAX       # Stack um nStack Bytes vergroessern
		MOV	EDI,ESP
		MOV	AX,SS
		MOV	ES,AX	      # ES:EDI = Startadresse des fuer
				      # Parameter reservierten Stackbereichs
		MOV	ESI,[EBP+12]  # DS:ESI == pStack

		MOVZX	ECX,WORD PTR [EBP+16] # ECX == nStack
		SHR	ECX,1
		CLD
		JCXZ	$1
	REP	MOVSW		      # Stack uebernehmen
$1:		MOV	DS,DX
		CALL	DWORD PTR [EBP+8]	# Aufruf der Prozedur
		# CLI # unter NT nicht erlaubt (privileged instruction)
		MOV	ESP,EBP
		SUB	ESP,16	      # wegen gepushter Register
				      # (ESI, EDI)
		# STI
		POP	EDX
		POP	ECX
		POP	EDI
		POP	ESI
		POP	EBP
		RET	12

#_TEXT		ENDS

		#END
