#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:18:35 $
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

PRJ := ..$/..$/..$/..$/..$/..
PRJNAME := ucb
PACKAGE := com$/sun$/star$/comp$/ucb
TARGET := test_com_sun_star_comp_ucb

.INCLUDE : settings.mk

JAVAFILES = $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

CLASSDIR !:= $(CLASSDIR)$/test

JARFILES = OOoRunner.jar \
           ridl.jar \
           unoil.jar \
           juh.jar \
           jurt.jar

JAVACLASSFILES = $(CLASSDIR)$/$(PACKAGE)$/GlobalTransfer_Test.class

.INCLUDE: target.mk

$(JAVAFILES): $(MISC)$/$(TARGET).createdclassdir

$(MISC)$/$(TARGET).createdclassdir:
    - $(MKDIR) $(CLASSDIR)
    $(TOUCH) $@

#ALLTAR .PHONY:
runtest:
    java -classpath $(CLASSPATH) org.openoffice.Runner -TestBase java_complex \
        -AppExecutionCommand "c:\staroffice8.m13\program\soffice -accept=socket,host=localhost,port=8100;urp;" \
        -o $(subst,$/,. $(PACKAGE)$/GlobalTransfer_Test)

