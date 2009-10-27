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
# $Revision: 1.6 $
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
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/URLHelper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/HsqlDatabase.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/RowSet.java \
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
    
