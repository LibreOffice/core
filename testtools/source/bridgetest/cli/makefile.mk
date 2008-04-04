#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: obo $ $Date: 2008-04-04 13:09:51 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=testtools
TARGET=cli_cpp_bridgetest.uno
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# disable caching to avoid stale objects
# on version changes
CCACHE_DISABLE=TRUE
.EXPORT : CCACHE_DISABLE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =
CFLAGSENABLESYMBOLS:=-Z7

.IF "$(COM)" == "MSC"

# ------------------------------------------------------------------
.IF "$(GUI)"=="WNT"
MY_DLLPOSTFIX=.dll
DESTDIR=$(BIN)
BATCH_SUFFIX=.bat
GIVE_EXEC_RIGHTS=@echo
WINTARGETS=  \
    $(DESTDIR)$/regcomp.exe \
    $(DESTDIR)$/uno.exe \
    $(DESTDIR)$/regcomp.exe.config \
    $(DESTDIR)$/uno.exe.config

.ELIF "$(GUI)"=="OS2"
MY_DLLPOSTFIX=.dll
DESTDIR=$(BIN)
BATCH_SUFFIX=.cmd
GIVE_EXEC_RIGHTS=@echo
WINTARGETS=  \
    $(DESTDIR)$/regcomp.exe \
    $(DESTDIR)$/uno.exe \
    $(DESTDIR)$/regcomp.exe.config \
    $(DESTDIR)$/uno.exe.config

.ELSE

MY_DLLPOSTFIX=.so
DESTDIR=$(OUT)$/lib
BATCH_INPROCESS=bridgetest_inprocess
GIVE_EXEC_RIGHTS=chmod +x
.ENDIF		$(DESTDIR)$/bridgetest_server$(BATCH_SUFFIX) \
        $(DESTDIR)$/bridgetest_client$(BATCH_SUFFIX) \
        $(JAVATARGETS)


UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

#-----------------------
.IF "$(CCNUMVER)" >= "001399999999"
CFLAGSCXX += -clr:oldSyntax -AI $(OUT)$/bin -AI $(SOLARBINDIR)
SHL1STDLIBS = \
    mscoree.lib \
    msvcmrt.lib
.ELSE
CFLAGSCXX += -clr -AI $(OUT)$/bin -AI $(SOLARBINDIR)
SHL1STDLIBS = \
    mscoree.lib
.ENDIF
SLOFILES= \
    $(SLO)$/cli_cpp_bridgetest.obj

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)


SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)


# --- Targets ------------------------------------------------------

.ENDIF

.INCLUDE :	target.mk

.IF "$(COM)" == "MSC"

ALLTAR : $(DESTDIR)$/cli_bridgetest_inprocess.exe

#################################################################

CLI_URE = $(SOLARBINDIR)$/cli_ure.dll
CLI_TYPES = $(SOLARBINDIR)$/cli_types.dll
CLI_BASETYPES = $(SOLARBINDIR)$/cli_basetypes.dll
CLI_CPPUHELPER = $(SOLARBINDIR)$/cli_cppuhelper.dll
CLI_TYPES_BRIDGETEST = $(BIN)$/cli_types_bridgetest.dll

CSCFLAGS = -warnaserror+ 
.IF "$(CCNUMVER)" <= "001399999999"
VBC_FLAGS = -warnaserror+
.ELSE
VBC_FLAGS = -nowarn:42030 -warnaserror+
.ENDIF
.IF "$(debug)" != ""
CSCFLAGS += -debug+ -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -debug+ -define:DEBUG=TRUE -define:TRACE=TRUE
.ELSE
CSCFLAGS += -optimize
VBC_FLAGS += -optimize
.ENDIF


# C# ----------------------------------------------
$(DESTDIR)$/cli_cs_testobj.uno.dll : \
        cli_cs_testobj.cs \
        cli_cs_multi.cs \
        $(CLI_BASETYPES) \
        $(CLI_TYPES) \
        $(CLI_URE)
    $(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_BASETYPES) \
         -reference:$(CLI_TYPES) \
        cli_cs_testobj.cs cli_cs_multi.cs

$(DESTDIR)$/cli_cs_bridgetest.uno.dll : \
        cli_cs_bridgetest.cs \
        $(CLI_BASETYPES) \
        $(CLI_TYPES) \
        $(CLI_URE)
    $(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URE) \
        -reference:System.dll \
        cli_cs_bridgetest.cs

# Visual Basic ------------------------------------------
$(DESTDIR)$/cli_vb_bridgetest.uno.dll : \
        cli_vb_bridgetest.vb \
        $(CLI_BASETYPES) \
        $(CLI_TYPES) \
        $(CLI_URE)
    $(VBC) $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_bridgetest.vb

$(DESTDIR)$/cli_vb_testobj.uno.dll : \
        cli_vb_testobj.vb \
        $(CLI_BASETYPES) \
        $(CLI_TYPES) \
        $(CLI_URE)
    $(VBC) $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_testobj.vb


$(DESTDIR)$/cli_bridgetest_inprocess.exe : \
        cli_bridgetest_inprocess.cs \
        $(DESTDIR)$/cli_cs_bridgetest.uno.dll \
        $(DESTDIR)$/cli_cs_testobj.uno.dll \
        $(DESTDIR)$/cli_vb_bridgetest.uno.dll \
        $(DESTDIR)$/cli_vb_testobj.uno.dll \
        $(DESTDIR)$/cli_cpp_bridgetest.uno.dll \
        $(CLI_BASETYPES) \
        $(CLI_TYPES) \
        $(CLI_URE) \
        $(CLI_CPPUHELPER)
    $(CSC) $(CSCFLAGS) -target:exe -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_CPPUHELPER) \
        -reference:$(DESTDIR)$/cli_cs_bridgetest.uno.dll \
        -reference:$(DESTDIR)$/cli_cs_testobj.uno.dll \
        -reference:$(DESTDIR)$/cli_vb_bridgetest.uno.dll \
        -reference:$(DESTDIR)$/cli_vb_testobj.uno.dll \
        -reference:$(DESTDIR)$/cli_cpp_bridgetest.uno.dll \
        cli_bridgetest_inprocess.cs
    $(GNUCOPY) -p cli_bridgetest_inprocess.ini $(DESTDIR)
    $(GNUCOPY) -p $(CLI_CPPUHELPER) $(DESTDIR)$/$(CLI_CPPUHELPER:f)
    $(GNUCOPY) -p $(CLI_BASETYPES) $(DESTDIR)$/$(CLI_BASETYPES:f)
    $(GNUCOPY) -p $(CLI_TYPES) $(DESTDIR)$/$(CLI_TYPES:f)
    $(GNUCOPY) -p $(CLI_URE) $(DESTDIR)$/$(CLI_URE:f)

.ENDIF

