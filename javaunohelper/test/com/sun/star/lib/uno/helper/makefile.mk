#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:47:22 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
