#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/msidl/makefile.mk,v 1.8 2007-11-06 15:42:06 rt Exp $
#
# =========================================================================
#
#     $Date: 2007-11-06 15:42:06 $
#     $Author: rt $
#     $Revision: 1.8 $
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

.IF "$(COM)"=="GCC"
MTL_SWITCHES+=-cpp_cmd gcc -cpp_opt "-x c -E -D__stdcall= -D_stdcall= -nostdinc -I $(shell @cygpath -w $(PSDK_HOME)/include)"

$(MISC)$/activex_component : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE) 
.ELSE
$(MISC)$/activex_component : $(SOURCE) 
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) && $(TOUCH) $@
.ENDIF

.ENDIF

.INCLUDE : target.mk

