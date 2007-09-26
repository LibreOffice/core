#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2007-09-26 14:27:05 $
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

PRJ = ..$/..
TARGET  = GeneralTest
PRJNAME = connectivity
PACKAGE = complex$/connectivity

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES       =\
        $(TARGET).java
        
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk


run:
    java -cp $(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar org.openoffice.Runner -TestBase java_complex -o complex.connectivity.$(TARGET)

