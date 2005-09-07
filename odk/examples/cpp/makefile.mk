#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:15:25 $
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

COUNTER_FILES=\
    $(DESTDIRCPPEXAMPLES)$/counter$/counter.cxx \
    $(DESTDIRCPPEXAMPLES)$/counter$/counter.uno.xml \
    $(DESTDIRCPPEXAMPLES)$/counter$/countermain.cxx \
    $(DESTDIRCPPEXAMPLES)$/counter$/Makefile \
    $(DESTDIRCPPEXAMPLES)$/counter$/XCountable.idl

DOUCMENTLOADER_FILES=\
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/DocumentLoader.cxx \
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/Makefile \
    $(DESTDIRCPPEXAMPLES)$/DocumentLoader$/test.odt

REMOTECLIENT_FILES=\
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/Makefile \
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/remoteclient.cxx \
    $(DESTDIRCPPEXAMPLES)$/remoteclient$/remoteclientsample.uno.xml

DIR_FILE_LIST= \
    $(COUNTER_FILES) \
    $(DOUCMENTLOADER_FILES) \
    $(REMOTECLIENT_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/ex_cpp_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/ex_cpp_files.txt

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


