#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:12:17 $
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

PRJ=..$/..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

FORMS_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/BooleanValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ButtonOperator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ComponentTreeTraversal.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlLock.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlValidation.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DataAwareness.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DateValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentBasedExample.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentHelper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentType.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentViewHelper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/FLTools.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/FormLayer.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/GridFieldValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/InteractionRequest.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/KeyGenerator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ListSelectionValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/NumericValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ProgrammaticScriptAssignment.odt \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SalesFilter.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SingleControlValidation.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetDocument.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetValueBinding.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetView.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TableCellTextBinding.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TextValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TimeValidator.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/UNO.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ValueBinding.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/WaitForInput.java

DIR_FILE_LIST= \
    $(FORMS_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_forms_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_forms.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk
    
