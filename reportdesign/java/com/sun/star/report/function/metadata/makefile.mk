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
# $Revision: 1.3.22.2 $
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

PRJ	= ..$/..$/..$/..$/..$/..$/..
PRJNAME = reportdesign
TARGET= rpt_java_css_metadata
PACKAGE = com$/sun$/star$/report$/function$/metadata

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

.IF "$(SYSTEM_JFREEREPORT)" == "YES"
CLASSPATH!:==$(CLASSPATH)$(PATH_SEPERATOR)$(JCOMMON_JAR)$(PATH_SEPERATOR)$(LIBFORMULA_JAR)
.ELSE
JARFILES += jcommon-1.0.10.jar libformula-0.1.14.jar
.ENDIF

JAVAFILES       :=	AuthorFunction.java \
                    AuthorFunctionDescription.java \
                    TitleFunction.java \
                    TitleFunctionDescription.java \
                    MetaDataFunctionCategory.java

# PROPERTYFILES := $(shell @$(FIND) . -name "*.properties")
PROPERTYFILES := $(CLASSDIR)$/$(PACKAGE)$/category.properties \
         $(CLASSDIR)$/$(PACKAGE)$/category_en_US.properties \
                 $(CLASSDIR)$/$(PACKAGE)$/Author-Function.properties \
         $(CLASSDIR)$/$(PACKAGE)$/Author-Function_en_US.properties \
                 $(CLASSDIR)$/$(PACKAGE)$/Title-Function.properties	\
         $(CLASSDIR)$/$(PACKAGE)$/Title-Function_en_US.properties
                 
# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

ALLTAR : $(PROPERTYFILES)

$(CLASSDIR)$/$(PACKAGE)$/%.properties : %.properties
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
