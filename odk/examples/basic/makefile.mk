#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:15:07 $
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

PRJ=..$/..
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

DRAWING_FILES=\
    $(DESTDIRBASICEXAMPLES)$/drawing$/dirtree.txt \
    $(DESTDIRBASICEXAMPLES)$/drawing$/importexportofasciifiles.odg

FORMSANDCONTROLS_FILES=\
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/beef.wmf \
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/burger.wmf \
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/burger_factory.odt \
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/chicken.wmf \
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/fish.wmf \
    $(DESTDIRBASICEXAMPLES)$/forms_and_controls$/vegetable.wmf

SHEET_FILES=\
    $(DESTDIRBASICEXAMPLES)$/sheet$/adapting_to_euroland.ods

STOCKQUOTEUPDATER_FILES=\
    $(DESTDIRBASICEXAMPLES)$/stock_quotes_updater$/stock.ods

TEXT_FILES=\
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index$/index.odt \
    $(DESTDIRBASICEXAMPLES)$/text$/creating_an_index$/indexlist.txt \
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/changing_appearance.odt \
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/inserting_bookmarks.odt \
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/replacing_text.odt \
    $(DESTDIRBASICEXAMPLES)$/text$/modifying_text_automatically$/using_regular_expressions.odt

DIR_FILE_LIST= \
    $(DRAWING_FILES) \
    $(FORMSANDCONTROLS_FILES) \
    $(SHEET_FILES) \
    $(STOCKQUOTEUPDATER_FILES) \
    $(TEXT_FILES) \

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/ex_basic_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/ex_basic_files.txt

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
    

