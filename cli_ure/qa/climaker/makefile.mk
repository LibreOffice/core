#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2005-09-23 11:51:33 $
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
#*************************************************************************

PRJ := ..$/..
PRJNAME := cli_ure
TARGET := test_climaker
PACKAGE = climaker

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES       = ClimakerTestCase.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE



CSCFLAGS = -incr
.IF "$(debug)" != ""
CSCFLAGS += -checked+ -define:DEBUG -define:TRACE -debug+
.ELSE
CSCFLAGS += -optimize+
.ENDIF


OUTDIR=$(BIN)$/qa
EXETARGET=$(OUTDIR)$/test_climaker.exe

ALLTAR: $(EXETARGET)

CSFILES = climaker.cs testobjects.cs


$(EXETARGET): $(CSFILES) $(OUTDIR)$/cli_test_types.dll
    $(GNUCOPY) -p $(BIN)$/cli_cppuhelper.dll $(OUTDIR)$/cli_cppuhelper.dll
    $(GNUCOPY) -p $(BIN)$/cli_types.dll $(OUTDIR)$/cli_types.dll
    $(GNUCOPY) -p $(BIN)$/cli_basetypes.dll $(OUTDIR)$/cli_basetypes.dll
    $(GNUCOPY) -p $(BIN)$/cli_ure.dll $(OUTDIR)$/cli_ure.dll
    $(GNUCOPY) -p $(BIN)$/climaker.exe $(OUTDIR)
    +$(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET) \
        -reference:$(BIN)$/cli_ure.dll \
         -reference:$(BIN)$/cli_types.dll \
         -reference:$(BIN)$/cli_basetypes.dll \
        -reference:$(OUTDIR)$/cli_test_types.dll \
        $(CSFILES)



#-----------------------------------------------------------------------------
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF




$(OUTDIR)$/types.urd: types.idl
    - $(MKDIR) $(OUTDIR)
    $(IDLC) -O$(OUTDIR) -I$(SOLARIDLDIR) -cid -we $<

$(OUTDIR)$/types.rdb: $(OUTDIR)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(OUTDIR)$/cli_test_types.dll: $(OUTDIR)$/types.rdb $(BIN)$/climaker.exe $(BIN)$/cli_types.dll
    $(BIN)$/$(CLIMAKER) $(CLIMAKERFLAGS) --out $@  \
        -r $(BIN)$/cli_types.dll \
        -X $(SOLARBINDIR)$/types.rdb \
        $(OUTDIR)$/types.rdb



.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN:
    +java -cp $(CLASSPATH) -Dcli_ure_test=$(EXETARGET) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN
