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

PRJNAME := jurt
PACKAGE := com$/sun$/star$/demo
TARGET  := com_sun_star_demo

RDB=$(SOLARBINDIR)$/applicat.rdb

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#Files --------------------------------------------------------

JARFILES = unoil.jar ridl.jar

GENJAVACLASSFILES = 

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/DemoServer.class	\
    $(CLASSDIR)$/$(PACKAGE)$/TestOffice.class

TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}

JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) $(GENJAVAFILES) 

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
