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

PRJ=..$/..

PRJNAME := extensions
PACKAGE := 
TARGET  := test_com_sun_star_pgp

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# Files --------------------------------------------------------

APPLICATRDB := $(SOLARBINDIR)$/applicat.rdb
RDB := $(APPLICATRDB)

JARFILES= jurt.jar

GENJAVACLASSFILES= \
    $(CLASSDIR)$/com$/sun$/star$/beans$/PropertyValue.class				\
    $(CLASSDIR)$/com$/sun$/star$/beans$/PropertyState.class				\
    $(CLASSDIR)$/com$/sun$/star$/container$/XSet.class				\

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/TestPGP.class


TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) $(GENJAVAFILES) 

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : $(GENJAVAFILES) ALLTAR 
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(GENJAVAFILES) : $(RDB)
    javamaker @$(mktmp -BUCR -O$(OUT) $(TYPES) $(RDB))

$(JAVACLASSFILES) : $(GENJAVAFILES)
