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

PRJPCH=

PRJNAME=scp2
TARGET=gnome
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

SCP_PRODUCT_TYPE=osl

.IF "$(ENABLE_SYSTRAY_GTK)" != ""
SCPDEFS+=-DENABLE_SYSTRAY_GTK
.ENDIF

.IF "$(ENABLE_GCONF)"!="" || "$(ENABLE_GNOMEVFS)"!="" || "$(ENABLE_GIO)"!=""

.IF "$(ENABLE_GCONF)" != ""
SCPDEFS+=-DENABLE_GCONF
.ENDIF

.IF "$(ENABLE_LOCKDOWN)" == "YES"
SCPDEFS+=-DENABLE_LOCKDOWN
.ENDIF

.IF "$(ENABLE_GNOMEVFS)" != ""
SCPDEFS+=-DENABLE_GNOMEVFS
.ENDIF

.IF "$(ENABLE_GIO)" != ""
SCPDEFS+=-DENABLE_GIO
.ENDIF

PARFILES =                   \
        module_gnome.par     \
        file_gnome.par

ULFFILES= \
        module_gnome.ulf

.ENDIF

# --- File ---------------------------------------------------------

.INCLUDE :  target.mk
