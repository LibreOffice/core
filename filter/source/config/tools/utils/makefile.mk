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
# $Revision: 1.9 $
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

PRJ = ..$/..$/..$/..
TARGET  = FCFGUtils
PRJNAME = filter
PACKAGE = com$/sun$/star$/filter$/config$/tools$/utils

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        =   \
                    ridl.jar        \
                    unoil.jar       \
                    jurt.jar        \
                    juh.jar         \
                    jut.jar         \
                    java_uno.jar

.IF "$(SYSTEM_XML_APIS)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XML_APIS_JAR)
.ELSE
JARFILES += xml-apis.jar
.ENDIF

.IF "$(SYSTEM_XERCES)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XERCES_JAR)
.ELSE
JARFILES += xercesImpl.jar
.ENDIF

JAVACLASSFILES  =   \
                    $(CLASSDIR)$/$(PACKAGE)$/AnalyzeStartupLog.class              \
                    $(CLASSDIR)$/$(PACKAGE)$/ConfigHelper.class                   \
                    $(CLASSDIR)$/$(PACKAGE)$/Logger.class                         \
                    $(CLASSDIR)$/$(PACKAGE)$/FileHelper.class                     \
                    $(CLASSDIR)$/$(PACKAGE)$/MalformedCommandLineException.class  \
                    $(CLASSDIR)$/$(PACKAGE)$/Cache.class                          \
                    $(CLASSDIR)$/$(PACKAGE)$/XMLHelper.class

MAXLINELENGTH   =   100000

# --- targets -----------------------------------------------------

.INCLUDE :  target.mk
