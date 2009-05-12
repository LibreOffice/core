#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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

PRJ := ..$/..$/..$/..$/..$/..$/..
PRJNAME := ridljar

TARGET := unoloader_cssl_unoloader
PACKAGE := com$/sun$/star$/lib$/unoloader

.INCLUDE: settings.mk

# Use a non-default CLASSDIR, so that the util/makefile.mk can use
# JARCLASSDIRS=com and nevertheless not include this package in ridl.jar:
CLASSDIR != $(OUT)$/class$/unoloader

JAVAFILES = \
    UnoClassLoader.java \
    UnoLoader.java

JARTARGET = unoloader.jar
JARCLASSDIRS = $(PACKAGE)
CUSTOMMANIFESTFILE = manifest

.INCLUDE: target.mk

.IF "$(SOLAR_JAVA)" != ""

.IF "$(depend)" == ""
$(JAVACLASSFILES): $(MISC)$/$(TARGET).mkdir.done
.ENDIF

$(MISC)$/$(TARGET).mkdir.done .ERRREMOVE:
    $(MKDIRHIER) $(CLASSDIR)
    $(TOUCH) $@

.ENDIF
