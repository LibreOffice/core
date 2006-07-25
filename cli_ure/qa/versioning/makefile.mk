#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2006-07-25 07:53:03 $
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


# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := test_climaker
PACKAGE = cliversion

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES = VersionTestCase.java
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



OUTDIR=$(BIN)$/qa$/versioning
EXETARGET2=$(OUTDIR)$/runtests.exe 

.IF "$(name)" != ""
TESTLIB=$(OUTDIR)$/$(name)
.ELSE
TESTLIB=$(OUTDIR)$/version_current.dll
.ENDIF
VERSIONLIBS=version_libs

.IF "$(GUI)"=="WNT"
ALLTAR: $(EXETARGET2) \
    $(TESTLIB) \
    COPYVERSIONLIBS \
    RUNINSTRUCTIONS
.ELSE
ALLTAR:
.ENDIF


MAKEOUTDIR:
     $(MKDIR) $(OUTDIR)

COPYVERSIONLIBS: MAKEOUTDIR
        -$(GNUCOPY) -p $(VERSIONLIBS)$/* $(OUTDIR)

CSFILES2 = runtests.cs
$(EXETARGET2): $(CSFILES2) MAKEOUTDIR
    +$(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET2) \
        $(CSFILES2)


CSFILESLIB = version.cs
$(TESTLIB): $(CSFILESLIB) MAKEOUTDIR
    +$(CSC) $(CSCFLAGS) -target:library -out:$(TESTLIB) \
        -reference:$(BIN)$/cli_ure.dll \
         -reference:$(BIN)$/cli_types.dll \
         -reference:$(BIN)$/cli_basetypes.dll \
        -reference:$(BIN)$/cli_cppuhelper.dll \
        $(CSFILESLIB)

#This target only checks if the the office/program/assembly directory
#contains the proper libraries.
$(OUTDIR)$/buildwithofficelibs.dll: MAKEOUTDIR
    +$(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:"$(office)"$/program$/assembly$/cli_ure.dll \
         -reference:"$(office)"$/program$/assembly$/cli_types.dll \
         -reference:"$(office)"$/program$/assembly$/cli_basetypes.dll \
        -reference:"$(office)"$/program$/assembly$/cli_cppuhelper.dll \
        $(CSFILESLIB)


#-----------------------------------------------------------------------------
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF


.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk


RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "To run the test you have to provide the path to the  office location."
    @echo Example:
    @echo dmake run office="d:\myOffice"
    @echo .
    @echo "To build a test library with a particular name run"
    @echo "dmake name=name_of_library.dll"	
    @echo ###########################   N O T E  ######################################
    @echo .
    @echo .	

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN: $(OUTDIR)$/buildwithofficelibs.dll
    +java -cp $(CLASSPATH) -DSystemRoot=$(SystemRoot) -Dcli_test_program=$(EXETARGET2) -Duno_path="$(office)"\program $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN

