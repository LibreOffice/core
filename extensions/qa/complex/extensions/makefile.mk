#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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



.IF "$(OS)"=="WNT"
command_separator=&&
.ELSE
command_separator=;
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
    $(foreach,i,$(RESLIB1TARGETN) $(COPY) $i $(i:s/de/invalid/:s/_A_//) $(command_separator)) echo
    $(foreach,i,$(RESLIB2TARGETN) $(COPY) $i $(i:s/en-US/invalid/:s/_B_//) $(command_separator)) echo


.END

