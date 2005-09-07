#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 15:43:46 $
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

PRJNAME	= accessibility
PRJ		= ..$/..$/..$/..$/..
TARGET	= java_accessibility
PACKAGE	= org$/openoffice$/java$/accessibility

USE_JAVAVER:=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(JAVANUMVER:s/.//)" >= "000100040000" 
JAVADIR = $(OUT)$/misc$/java
JARFILES = sandbox.jar jurt.jar unoil.jar ridl.jar
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

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class) $(CLASSDIR)$/$(PACKAGE)$/Build.class

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE)

.ENDIF			# "$(JAVANUMVER:s/.//)" >= "000100040000" 

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

.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
$(JAVADIR)$/$(PACKAGE)$/%.java: makefile.mk
    @-+$(MKDIRHIER) $(JAVADIR)$/$(PACKAGE) >& $(NULLDEV) 
    @-+echo package org.openoffice.java.accessibility\; > $@
    @-+echo public class Build { >> $@
    @-+echo public static final boolean DEBUG = $(DEBUGSWITCH)\; >> $@
    @-+echo public static final boolean PRODUCT = $(PRODUCTSWITCH)\; >> $@
    @-+echo } >> $@
.ELSE
$(JAVADIR)$/$(PACKAGE)$/%.java: makefile.mk
    @-+$(MKDIRHIER) $(JAVADIR)$/$(PACKAGE) >& $(NULLDEV) 
    @-+echo package org.openoffice.java.accessibility; > $@
    @-+echo public class Build { >> $@
    @-+echo public static final boolean DEBUG = $(DEBUGSWITCH); >> $@
    @-+echo public static final boolean PRODUCT = $(PRODUCTSWITCH); >> $@
    @-+echo } >> $@
.ENDIF


$(CLASSDIR)$/$(PACKAGE)$/Build.class : $(JAVADIR)$/$(PACKAGE)$/Build.java
    -+$(JAVAC) -d $(CLASSDIR) $(JAVADIR)$/$(PACKAGE)$/Build.java

