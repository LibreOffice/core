#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
