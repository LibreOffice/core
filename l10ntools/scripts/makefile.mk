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

# Copy *.py files into output tree and call a script once to
# force python to create the *.pyc files.

PRJ=..
TARGET = l10ntools_dummy_pyc

.INCLUDE: settings.mk

.IF "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(SOLARBINDIR)/python
.ELSE                   # "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) python
.ENDIF                  # "$(SYSTEM_PYTHON)"!="YES"

PYFILES = $(BIN)$/const.py \
          $(BIN)$/l10ntool.py \
          $(BIN)$/pseudo.py \
          $(BIN)$/sdf.py \
          $(BIN)$/xhtex.py \
          $(BIN)$/xtxex.py   

.INCLUDE: target.mk

.IGNORE : create_pyc 
ALLTAR : create_pyc 
create_pyc : $(PYFILES)
    @$(PYTHON) $(BIN)/xtxex.py >& /dev/null

$(BIN)$/%.py : tool/%.py
    @$(COPY) $< $@


