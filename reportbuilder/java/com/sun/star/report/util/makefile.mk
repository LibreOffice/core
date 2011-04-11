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

PRJ	= ..$/..$/..$/..$/..$/..
PRJNAME = reportbuilder
TARGET= rpt_java_css_util
PACKAGE = com$/sun$/star$/report$/util

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

.IF "$(SYSTEM_JFREEREPORT)" == "YES"
EXTRAJARFILES   = $(LIBBASE_JAR) $(LIBXML_JAR) $(JFREEREPORT_JAR)
.ELSE
.INCLUDE :  $(SOLARBINDIR)/jfreereport_version.mk
JARFILES += \
    libbase-$(LIBBASE_VERSION).jar                      \
    libxml-$(LIBXML_VERSION).jar						\
    flow-engine-$(FLOW_ENGINE_VERSION).jar
.ENDIF

JAVAFILES       = DefaultJobProperties.java DefaultParameterMap.java DefaultReportJobDefinition.java ManifestWriter.java
# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
