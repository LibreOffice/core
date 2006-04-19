#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2006-04-19 15:12:58 $
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

PRJ		= ..$/..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = web_data
PACKAGE = com$/sun$/star$/wizards$/web$/data

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

#.INCLUDE :  $(PRJ)$/util$/makefile.pmk
JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge commonwizards.jar 

.IF "$(SYSTEM_XALAN)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XALAN_JAR)
.ELSE
JARFILES += xalan.jar
.ENDIF

.IF "$(SYSTEM_XERCES)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XERCES_JAR)
.ELSE
JARFILES += xercesImpl.jar
.ENDIF

.IF "$(SYSTEM_XML_APIS)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XML_APIS_JAR)
.ELSE
JARFILES += xml-apis.jar
.ENDIF

#JARCLASSDIRS	= com$/sun$/star$/wizards$/web
#JARTARGET		= $(TARGET).jar

# --- Files --------------------------------------------------------

JAVAFILES=			\
    CGArgument.java \
    CGContent.java \
    CGDesign.java \
    CGDocument.java \
    CGExporter.java \
    CGFilter.java \
    CGGeneralInfo.java \
    CGIconSet.java \
    CGImage.java \
    CGLayout.java \
    CGPublish.java \
    CGSession.java \
    CGSessionName.java \
    CGSettings.java \
    CGStyle.java \
    ConfigSetItem.java \
    TypeDetection.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
