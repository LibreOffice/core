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
# $Revision: 1.8 $
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

PRJ=..$/..$/..$/..$/..$/..$/..
PRJNAME = juhelper
PACKAGE = com$/sun$/star$/lib$/uno$/helper
TARGET  = com_sun_star_lib_uno_helper_test

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = jurt.jar ridl.jar juh.jar

JAVACLASSFILES=	\
    $(CLASSDIR)$/$(PACKAGE)$/WeakBase_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/ComponentBase_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/InterfaceContainer_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/MultiTypeInterfaceContainer_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/ProxyProvider.class \
    $(CLASSDIR)$/$(PACKAGE)$/AWeakBase.class    \
        $(CLASSDIR)$/$(PACKAGE)$/PropertySet_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/UnoUrlTest.class	\
    $(CLASSDIR)$/$(PACKAGE)$/Factory_Test.class

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

CPATH_JARS = java_uno.jar $(JARFILES)
CPATH_TMP1 = $(foreach,j,$(CPATH_JARS) $(SOLARBINDIR)$/$j)
CPATH_TMP2 = $(strip $(subst,!,$(PATH_SEPERATOR) $(CPATH_TMP1:s/ /!/)))
CPATH = $(CPATH_TMP2)$(PATH_SEPERATOR)$(OUT)$/bin$/factory_test.jar$(PATH_SEPERATOR)$(XCLASSPATH)

$(OUT)$/bin$/factory_test.jar : $(CLASSDIR)$/$(PACKAGE)$/Factory_Test.class
    -rm -f $@
    @echo RegistrationClassName: com.sun.star.lib.uno.helper.Factory_Test > $(OUT)$/bin$/manifest.mf
    -jar cvfm $@ $(OUT)$/bin$/manifest.mf -C $(CLASSDIR) $(PACKAGE)$/Factory_Test.class

run_factory_test : $(OUT)$/bin$/factory_test.jar
    -$(GNUCOPY) $(SOLARBINDIR)$/udkapi.rdb $(OUT)$/bin$/factory_test.rdb
    -java -classpath $(CPATH) com.sun.star.lib.uno.helper.Factory_Test $(OUT)$/bin$/factory_test.jar $(OUT)$/bin$/factory_test.rdb
