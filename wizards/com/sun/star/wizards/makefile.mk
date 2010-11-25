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
PRJ		= ..$/..$/..$/..
PRJNAME = wizards
TARGET  = wizards_common
PACKAGE = com$/sun$/star$/wizards

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge 

.IF "$(SYSTEM_SAXON)" == "YES"
EXTRAJARFILES = $(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF


JARCLASSDIRS	=                                               \
    com$/sun$/star$/wizards$/common	       	 		\
    com$/sun$/star$/wizards$/db	       	 		\
    com$/sun$/star$/wizards$/ui	       	 		\
    com$/sun$/star$/wizards$/document     	 		\
    com$/sun$/star$/wizards$/text

JARTARGET	= commonwizards.jar

# --- Files --------------------------------------------------------

JAVAFILES=							\
    common$/HelpIds.java	                		\
    common$/Desktop.java	                		\
    common$/NumberFormatter.java                 		\
    common$/FileAccess.java                 		\
    common$/Helper.java                     		\
    common$/InvalidQueryException.java      		\
    common$/JavaTools.java                  		\
    common$/NoValidPathException.java       		\
    common$/Properties.java                 		\
    common$/PropertyNames.java                 		\
    common$/Resource.java                   		\
    common$/SystemDialog.java				\
    common$/Configuration.java				\
    common$/ConfigNode.java					\
    common$/ConfigGroup.java				\
    common$/ConfigSet.java					\
    common$/XMLProvider.java				\
    common$/XMLHelper.java					\
    common$/IRenderer.java					\
    common$/Indexable.java					\
    common$/TerminateWizardException.java			\
    common$/UCB.java					\
    common$/DebugHelper.java					\
    common$/PropertySetHelper.java					\
    common$/NumericalHelper.java					\
    common$/NamedValueCollection.java       \
    db$/DBMetaData.java					\
    db$/CommandMetaData.java				\
    db$/QueryMetaData.java					\
    db$/BlindtextCreator.java				\
    db$/FieldColumn.java					\
    db$/CommandName.java					\
    db$/RecordParser.java					\
    db$/TypeInspector.java					\
    db$/ColumnPropertySet.java				\
    db$/RelationController.java				\
    db$/TableDescriptor.java				\
    db$/SQLQueryComposer.java				\
    db$/DatabaseObjectWizard.java					\
    ui$/event$/AbstractListener.java			\
    ui$/event$/CommonListener.java				\
    ui$/event$/DataAware.java				\
    ui$/event$/DataAwareFields.java				\
    ui$/event$/EventNames.java				\
    ui$/event$/ListModelBinder.java				\
    ui$/event$/MethodInvocation.java			\
    ui$/event$/RadioDataAware.java				\
    ui$/event$/SimpleDataAware.java				\
    ui$/event$/Task.java					\
    ui$/event$/TaskEvent.java				\
    ui$/event$/TaskListener.java				\
    ui$/event$/UnoDataAware.java				\
    ui$/AggregateComponent.java				\
    ui$/CommandFieldSelection.java				\
    ui$/ControlScroller.java				\
    ui$/DBLimitedFieldSelection.java			\
    ui$/DocumentPreview.java				\
    ui$/FilterComponent.java				\
    ui$/FieldSelection.java					\
    ui$/ImageList.java					\
    ui$/ButtonList.java					\
    ui$/PathSelection.java					\
    ui$/PeerConfig.java					\
    ui$/SortingComponent.java				\
    ui$/TitlesComponent.java				\
    ui$/WizardDialog.java					\
    ui$/UIConsts.java					\
    ui$/UnoDialog.java					\
    ui$/UnoDialog2.java					\
    ui$/XCommandSelectionListener.java			\
    ui$/XFieldSelectionListener.java			\
    ui$/XPathSelectionListener.java				\
    ui$/XCompletion.java					\
    document$/Control.java					\
    document$/DatabaseControl.java				\
    document$/FormHandler.java				\
    document$/GridControl.java				\
    document$/OfficeDocument.java				\
    document$/Shape.java					\
    document$/TimeStampControl.java				\
    text$/TextDocument.java					\
    text$/TextFieldHandler.java				\
    text$/TextFrameHandler.java				\
    text$/TextSectionHandler.java				\
    text$/TextStyleHandler.java				\
    text$/TextTableHandler.java				\
    text$/ViewHandler.java


#JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java/.class/))

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
