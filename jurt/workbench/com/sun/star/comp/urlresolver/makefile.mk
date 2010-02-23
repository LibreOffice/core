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

PRJ := ..$/..$/..$/..$/..$/..
PRJNAME := jurt
TARGET := workbench_com_sun_star_comp_urlresolver

PACKAGE := com$/sun$/star$/comp$/urlresolver
JAVAFILES := UrlResolver_Test.java
JARFILES := ridl.jar juh.jar

.INCLUDE: settings.mk

# Put the generated class files into <platform>/class/workbench/ instead of
# <platform>/class/ (otherwise, these class files might end up being zipped into
# the generated jurt.jar):
.IF "$(XCLASSPATH)" == ""
XCLASSPATH := $(CLASSDIR)
.ELSE
XCLASSPATH !:= $(XCLASSPATH)$(PATH_SEPERATOR)$(CLASSDIR)
.ENDIF
CLASSDIR !:= $(CLASSDIR)$/workbench

.INCLUDE: target.mk

$(JAVAFILES): $(MISC)$/$(TARGET).createdclassdir
$(MISC)$/$(TARGET).createdclassdir:
    - $(MKDIR) $(CLASSDIR)
    $(TOUCH) $@
