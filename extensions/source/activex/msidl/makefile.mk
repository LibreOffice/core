#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/msidl/makefile.mk,v 1.6 2006-10-30 08:08:42 rt Exp $
#
# =========================================================================
#
#     $Date: 2006-10-30 08:08:42 $
#     $Author: rt $
#     $Revision: 1.6 $
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

.IF "$(GUI)" == "WNT" && "$(DISABLE_ACTIVEX)"==""

# --- Targets ----------------------------------

ALL : $(MISC)$/activex_component

MTL=midl.exe
MTL_PROJ=

SOURCE=so_activex.idl

MTL_SWITCHES=-tlb $(MISC)$/so_activex.tlb -h $(MISC)$/so_activex.h -iid $(MISC)$/so_activex_i.c -dlldata $(MISC)$/so_activex_dll.c -proxy $(MISC)$/so_activex_p.c -Oicf

$(MISC)$/activex_component : $(SOURCE) 
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) && $(TOUCH) $@

.ENDIF

.INCLUDE : target.mk

