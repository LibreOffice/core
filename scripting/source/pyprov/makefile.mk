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

PRJ=..$/..

PRJNAME=scripting
TARGET=pyprov

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(ENABLE_SCRIPTING_PYTHON)" != "YES"
@all:
    @echo "Script Provider for Python build disabled."
.ENDIF


# --- Targets ------------------------------------------------------
ALL : ALLTAR \
    $(DLLDEST)$/officehelper.py	\
        $(DLLDEST)$/mailmerge.py

$(DLLDEST)$/%.py: %.py
    cp $? $@

# scripting provider extension
.IF "$(L10N_framework)"==""

EXTENSIONNAME:=ScriptProviderForPython
EXTENSION_ZIPNAME:=script-provider-for-python

COMPONENT_FILES=$(EXTENSIONDIR)$/pythonscript.py

.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

.ELSE

.INCLUDE : target.mk

.ENDIF
