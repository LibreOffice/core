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

PRJ = ../../../..
TARGET  = TransientDocument
PRJNAME = $(TARGET)
PACKAGE = complex/tdoc/interfaces

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES = _XChild.java                         \
            _XCommandInfoChangeNotifier.java     \
            _XCommandProcessor.java              \
            _XComponent.java                     \
            _XContent.java                       \
            _XPropertiesChangeNotifier.java      \
            _XPropertyContainer.java             \
            _XPropertySetInfoChangeNotifier.java \
            _XServiceInfo.java                   \
            _XTypeProvider.java

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS  = $(PACKAGE)
JARTARGET     = $(TARGET).jar
JARCOMPRESS   = TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


