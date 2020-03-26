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
PRJNAME = sc
TARGET = qa_complex_tdoc

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/tdoc

JAVATESTFILES = \
   CheckContentProvider.java \
   CheckTransientDocumentsContent.java \
   CheckTransientDocumentsContentProvider.java \
   CheckTransientDocumentsDocumentContent.java

JAVAFILES = $(JAVATESTFILES) \
    TestDocument.java \
    _XChild.java \
    _XCommandInfoChangeNotifier.java \
    _XCommandProcessor.java \
    _XComponent.java \
    _XContent.java \
    _XPropertiesChangeNotifier.java \
    _XPropertyContainer.java \
    _XPropertySetInfoChangeNotifier.java \
    _XServiceInfo.java \
    _XTypeProvider.java

JARFILES = OOoRunner.jar libreoffice.jar test.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

# Sample how to debug
# JAVAIFLAGS=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : javatest

.END

