#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2003-09-04 09:17:35 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=testtools
TARGET=cli_cpp_bridgetest.uno
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
DLLPRE =
.INCLUDE :  sv.mk

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

CFLAGS += -clr /AI $(OUT)$/bin /AI $(SOLARBINDIR)

SLOFILES= \
    $(SLO)$/cli_cpp_bridgetest.obj		

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1STDLIBS = \
    mscoree.lib

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)


# --- Targets ------------------------------------------------------
ALL : \
        ALLTAR \
        $(DESTDIR)$/cli_bridgetest_inprocess.exe

.INCLUDE :	target.mk

#################################################################

CLI_URE = $(SOLARBINDIR)$/cli_ure.dll
CLI_TYPES = $(SOLARBINDIR)$/cli_types.dll
CLI_CPPUHELPER = $(SOLARBINDIR)$/cli_cppuhelper.dll

CSCFLAGS = -warnaserror+ -incremental-
VBC_FLAGS = -warnaserror+
.IF "$(debug)" == ""
.IF "$(product)" == ""
CSCFLAGS += -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -define:DEBUG=1 -define:TRACE=1
.ELSE
CSCFLAGS += -o
VBC_FLAGS += -o
.ENDIF
.ELSE # DEBUG
CSCFLAGS += -debug+ -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -debug+ -define:DEBUG=1 -define:TRACE=1
.ENDIF

# C# ----------------------------------------------
$(DESTDIR)$/cli_cs_testobj.uno.dll : \
        cli_cs_testobj.cs \
        $(CLI_TYPES) \
        $(CLI_URE)
    +csc $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        cli_cs_testobj.cs

$(DESTDIR)$/cli_cs_bridgetest.uno.dll : \
        cli_cs_bridgetest.cs \
        $(CLI_TYPES) \
        $(CLI_URE)
    +csc $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        -reference:System.dll \
        cli_cs_bridgetest.cs

$(DESTDIR)$/cli_bridgetest_inprocess.exe : \
        cli_bridgetest_inprocess.cs \
        $(DESTDIR)$/cli_cs_bridgetest.uno.dll \
        $(DESTDIR)$/cli_cs_testobj.uno.dll \
        $(DESTDIR)$/cli_vb_bridgetest.uno.dll \
        $(DESTDIR)$/cli_vb_testobj.uno.dll \
        $(DESTDIR)$/cli_cpp_bridgetest.uno.dll \
        $(CLI_TYPES) \
        $(CLI_URE) \
        $(CLI_CPPUHELPER)
    +csc $(CSCFLAGS) -target:exe -out:$@ \
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
    $(GNUCOPY) -p $(CLI_CPPUHELPER) $(DESTDIR)
    $(GNUCOPY) -p $(CLI_TYPES) $(DESTDIR)
    $(GNUCOPY) -p $(CLI_URE) $(DESTDIR)

# Visual Basic ------------------------------------------
$(DESTDIR)/cli_vb_bridgetest.uno.dll : \
        cli_vb_bridgetest.vb \
        $(CLI_TYPES) \
        $(CLI_URE)
    +vbc $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_bridgetest.vb

$(DESTDIR)/cli_vb_testobj.uno.dll : \
        cli_vb_testobj.vb \
        $(CLI_TYPES) \
        $(CLI_URE)
    +vbc $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_TYPES) \
        -reference:$(CLI_URE) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_testobj.vb
