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



PRJNAME	= accessibility
PRJ		= ..$/..$/..$/..$/..
TARGET	= java_accessibility
PACKAGE	= org$/openoffice$/java$/accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JAVADIR = $(OUT)$/misc$/java
JARFILES = jurt.jar unoil.jar ridl.jar
JAVAFILES = \
    logging$/XAccessibleEventLog.java \
    logging$/XAccessibleHypertextLog.java \
    logging$/XAccessibleTextLog.java \
    AbstractButton.java \
    AccessibleActionImpl.java \
    AccessibleComponentImpl.java \
    AccessibleEditableTextImpl.java \
    AccessibleExtendedState.java \
    AccessibleHypertextImpl.java \
    AccessibleIconImpl.java \
    AccessibleKeyBinding.java \
    AccessibleObjectFactory.java \
    AccessibleRoleAdapter.java \
    AccessibleSelectionImpl.java \
    AccessibleStateAdapter.java \
    AccessibleTextImpl.java \
    AccessibleValueImpl.java \
    Alert.java \
    Application.java \
    Button.java \
    CheckBox.java \
    ComboBox.java \
    Component.java \
    Container.java \
    DescendantManager.java \
    Dialog.java \
    FocusTraversalPolicy.java \
    Frame.java \
    Icon.java \
    Label.java \
    List.java \
    Menu.java \
    MenuItem.java \
    MenuContainer.java \
    NativeFrame.java \
    Paragraph.java \
    RadioButton.java \
    ScrollBar.java \
    Separator.java \
    Table.java \
    TextComponent.java \
    ToggleButton.java \
    ToolTip.java \
    Tree.java \
    Window.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java//).class) $(CLASSDIR)$/$(PACKAGE)$/Build.class

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

# Enable logging in non-product only
.IF "$(PRODUCT)"!=""
DEBUGSWITCH = false
PRODUCTSWITCH = true
.ELSE
PRODUCTSWITCH = false
DEBUGSWITCH = true
.ENDIF

$(JAVADIR)$/$(PACKAGE)$/%.java: makefile.mk
    @@-$(MKDIRHIER) $(JAVADIR)$/$(PACKAGE)
    @-echo package org.openoffice.java.accessibility\; > $@
    @-echo public class Build { >> $@
    @-echo public static final boolean DEBUG = $(DEBUGSWITCH)\; >> $@
    @-echo public static final boolean PRODUCT = $(PRODUCTSWITCH)\; >> $@
    @-echo } >> $@

$(CLASSDIR)$/$(PACKAGE)$/Build.class : $(JAVADIR)$/$(PACKAGE)$/Build.java
    -$(JAVAC) -d $(CLASSDIR) $(JAVADIR)$/$(PACKAGE)$/Build.java

