#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.34 $
#
#   last change: $Author: obo $ $Date: 2008-03-10 10:56:05 $
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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uimisc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        dbumiscres.src	\
        WizardPages.src

# ... object files ............................

SLOFILES=	\
        $(SLO)$/asyncmodaldialog.obj        \
        $(SLO)$/imageprovider.obj           \
        $(SLO)$/singledoccontroller.obj		\
        $(SLO)$/datasourceconnector.obj		\
        $(SLO)$/linkeddocuments.obj			\
        $(SLO)$/indexcollection.obj			\
        $(SLO)$/UITools.obj					\
        $(SLO)$/WCPage.obj					\
        $(SLO)$/WCopyTable.obj				\
        $(SLO)$/WTypeSelect.obj				\
        $(SLO)$/TokenWriter.obj				\
        $(SLO)$/HtmlReader.obj				\
        $(SLO)$/RtfReader.obj				\
        $(SLO)$/propertysetitem.obj			\
        $(SLO)$/databaseobjectview.obj      \
        $(SLO)$/DExport.obj					\
        $(SLO)$/uiservices.obj				\
        $(SLO)$/RowSetDrop.obj				\
        $(SLO)$/TableCopyHelper.obj			\
        $(SLO)$/moduledbu.obj               \
        $(SLO)$/WColumnSelect.obj			\
        $(SLO)$/WExtendPages.obj			\
        $(SLO)$/WNameMatch.obj				\
        $(SLO)$/documentcontroller.obj      \
        $(SLO)$/ToolBoxHelper.obj			\
        $(SLO)$/stringlistitem.obj			\
        $(SLO)$/charsets.obj				\
        $(SLO)$/dsntypes.obj                \
        $(SLO)$/defaultobjectnamecheck.obj  \
        $(SLO)$/dsmeta.obj                  \
        $(SLO)$/controllerframe.obj			\
        $(SLO)$/propertystorage.obj
# --- Targets ----------------------------------

.INCLUDE : target.mk

