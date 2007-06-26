#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2007-06-26 11:11:32 $
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

PRJ := ..$/..$/..
PRJNAME := bean
TARGET := oooapplet
PACKAGE = oooapplet

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar 
JAVAFILES       = OOoViewer.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)


JARCLASSDIRS    = \
    oooapplet

JARTARGET       = $(TARGET).jar
JARCOMPRESS     = TRUE

#----- make a jar from compiled files ------------------------------


.INCLUDE: target.mk


ALLTAR : \
    COPY_FILES \
    RUNINSTRUCTIONS


COPY_FILES: example.html 
    $(GNUCOPY) -p $< $(CLASSDIR)
# --- Targets ------------------------------------------------------


.IF "$(GUI)"=="WNT"
RUN:
    firefox "$(CLASSDIR)$/example.html?$(office)"
.ELSE
TESTURL="file:///$(PWD)$/$(CLASSDIR)$/example.html?$(office)"
RUN:
    firefox ${TESTURL:s/\///}
.ENDIF

run: RUN



RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "Add to the java runtime settings for applets in the control panel these lines:"
    @echo "-Djava.security.policy=$(PWD)$/bean.policy"
    @echo "To run the test you have to provide the office location."
    @echo Example:
    @echo dmake run office="d:\\myOffice"
    @echo .
  

