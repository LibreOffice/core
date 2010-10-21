#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ = ../../..
PRJNAME = extensions
TARGET = qa_complex_extensions

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/extensions
JAVATESTFILES = \
    OfficeResourceLoader.java

JAVAFILES = $(JAVATESTFILES)

JARFILES = OOoRunner.jar ridl.jar test.jar unoil.jar jurt.jar ConnectivityTools.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)
.END



.IF "$(GUI)"=="WNT"
command_seperator=&&
.ELSE
command_seperator=;
.ENDIF


.INCLUDE: settings.mk

#----- resource files for the OfficeResourceLoader test ------------
RES_TARGET = orl

SRS1NAME=$(RES_TARGET)_A_
SRC1FILES= \
    $(RES_TARGET)_en-US.src

RES1FILELIST=\
    $(SRS)$/$(RES_TARGET)_A_.srs

RESLIB1NAME=$(RES_TARGET)_A_
RESLIB1SRSFILES=$(RES1FILELIST)



SRS2NAME=$(RES_TARGET)_B_
SRC2FILES= \
    $(RES_TARGET)_de.src

RES2FILELIST=\
    $(SRS)$/$(RES_TARGET)_B_.srs

RESLIB2NAME=$(RES_TARGET)_B_
RESLIB2SRSFILES=$(RES2FILELIST)

.INCLUDE: target.mk
.INCLUDE: installationtest.mk


#----- resource files for the OfficeResourceLoader test ------------


ALLTAR : copy_resources javatest

copy_resources: $(RESLIB1TARGETN) $(RESLIB2TARGETN)
    $(foreach,i,$(RESLIB1TARGETN) $(COPY) $i $(i:s/de/invalid/:s/_A_//) $(command_seperator)) echo
    $(foreach,i,$(RESLIB2TARGETN) $(COPY) $i $(i:s/en-US/invalid/:s/_B_//) $(command_seperator)) echo


.END

