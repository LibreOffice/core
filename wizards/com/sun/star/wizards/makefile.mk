#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2004-11-15 13:43:28 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

JARCLASSDIRS	= com$/sun$/star$/wizards
JARTARGET	= commonwizards.jar

# --- Files --------------------------------------------------------

JAVAFILES=							\
    common$/Desktop.java	                		\
    common$/FileAccess.java                 		\
    common$/Helper.java                     		\
    common$/InvalidQueryException.java      		\
    common$/JavaTools.java                  		\
    common$/NoValidPathException.java       		\
    common$/Properties.java                 		\
    common$/Resource.java                   		\
    common$/SystemDialog.java				\
    common$/TaskList.java					\
    common$/Configuration.java				\
    common$/ConfigNode.java					\
    common$/ConfigGroup.java				\
    common$/ConfigSet.java					\
    common$/XMLProvider.java				\
    common$/XMLHelper.java					\
    common$/Renderer.java					\
    common$/Indexable.java					\
    common$/TerminateWizardException.java			\
    common$/UCB.java					\
    db$/DBMetaData.java					\
    db$/CommandMetaData.java				\
    db$/QueryMetaData.java					\
    db$/BlindtextCreator.java				\
    db$/FieldColumn.java					\
    db$/RecordParser.java					\
    db$/SQLQueryComposer.java				\
    db$/TableDescriptor.java				\
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
    ui$/PathSelection.java					\
    ui$/PeerConfig.java					\
    ui$/PeerConfigHelper.java				\
    ui$/SortingComponent.java				\
    ui$/TitlesComponent.java				\
    ui$/WizardDialog.java					\
    ui$/UIConsts.java					\
    ui$/UnoDialog.java					\
    ui$/UnoDialog2.java					\
    ui$/XCommandSelectionListener.java			\
    ui$/XFieldSelectionListener.java			\
    ui$/XPathSelectionListener.java				\
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


JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java/.class/))

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
