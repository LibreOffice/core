#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/msidl/makefile.mk,v 1.3 2004-06-16 10:53:56 rt Exp $
#
# =========================================================================
#
#     $Date: 2004-06-16 10:53:56 $
#     $Author: rt $
#     $Revision: 1.3 $
#
# =========================================================================
#
#     Created: 1999/08/23
#     Creator: obr
#
#     Copyright (c) 1999 StarOffice Software Entwicklungs GmbH
#
#**************************************************************************

PRJ=..$/..$/..
PRJNAME=extensions
TARGET=msidl

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT"

# --- Targets ----------------------------------

ALL : activex_component

MTL=midl.exe
MTL_PROJ=

SOURCE=so_activex.idl

MTL_SWITCHES=-tlb $(MISC)$/so_activex.tlb -h $(MISC)$/so_activex.h -iid $(MISC)$/so_activex_i.c -dlldata $(MISC)$/so_activex_dll.c -proxy $(MISC)$/so_activex_p.c -Oicf

activex_component : $(SOURCE) 
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) 

.ENDIF

.INCLUDE : target.mk

