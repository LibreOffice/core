#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/msidl/makefile.mk,v 1.2 2003-03-25 16:01:04 hr Exp $
#
# =========================================================================
#
#     $Date: 2003-03-25 16:01:04 $
#     $Author: hr $
#     $Revision: 1.2 $
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

MTL_SWITCHES=/tlb $(MISC)\so_activex.tlb /h $(MISC)\so_activex.h /iid $(MISC)\so_activex_i.c /dlldata $(MISC)\so_activex_dll.c /proxy $(MISC)\so_activex_p.c /Oicf

activex_component : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) $(SOURCE) 

.ENDIF

.INCLUDE : target.mk

