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

PRJ=..$/..$/..$/..$/..
PRJNAME=testtools
PACKAGE=com$/sun$/star$/comp$/bridge
TARGET=com_sun_star_comp_bridge

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(ENABLE_JAVA)" != ""

JARFILES 		= ridl.jar jurt.jar juh.jar

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/CurrentContextChecker.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponent.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponentMain.class

JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

JARCLASSDIRS	= $(PACKAGE) test$/testtools$/bridgetest
JARTARGET		= testComponent.jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
