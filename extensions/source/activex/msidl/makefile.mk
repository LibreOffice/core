#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/msidl/makefile.mk,v 1.1 2002-08-14 14:33:50 mav Exp $
#
# =========================================================================
#
#     $Date: 2002-08-14 14:33:50 $
#     $Author: mav $
#     $Revision: 1.1 $
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

MTL_SWITCHES=/tlb $(MISC)\so_activex.tlb /h $(MISC)\so_activex.h /iid $(MISC)\so_activex_i.c /Oicf

activex_component : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) $(SOURCE) 

.ENDIF

.INCLUDE : target.mk

