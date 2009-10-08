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
# $Revision: 1.10.68.1 $
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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbaxml

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/xmlfilter.obj				\
            $(SLO)$/dbloader2.obj				\
            $(SLO)$/xmlDatabase.obj				\
            $(SLO)$/xmlDataSource.obj			\
            $(SLO)$/xmlTableFilterPattern.obj	\
            $(SLO)$/xmlTableFilterList.obj		\
            $(SLO)$/xmlDataSourceInfo.obj		\
            $(SLO)$/xmlDataSourceSetting.obj	\
            $(SLO)$/xmlDataSourceSettings.obj	\
            $(SLO)$/xmlDocuments.obj			\
            $(SLO)$/xmlComponent.obj			\
            $(SLO)$/xmlHierarchyCollection.obj	\
            $(SLO)$/xmlLogin.obj				\
            $(SLO)$/xmlExport.obj				\
            $(SLO)$/xmlQuery.obj				\
            $(SLO)$/xmlTable.obj				\
            $(SLO)$/xmlColumn.obj				\
            $(SLO)$/xmlStyleImport.obj			\
            $(SLO)$/xmlHelper.obj				\
            $(SLO)$/xmlAutoStyle.obj			\
            $(SLO)$/xmlConnectionData.obj		\
            $(SLO)$/xmlDatabaseDescription.obj	\
            $(SLO)$/xmlFileBasedDatabase.obj	\
            $(SLO)$/xmlServerDatabase.obj		\
            $(SLO)$/xmlConnectionResource.obj	\
            $(SLO)$/xmlJavaClassPath.obj		\
            $(SLO)$/xmlservices.obj

# --- Library -----------------------------------
LIB1TARGET=$(SLB)$/dba_flt.lib
LIB1FILES=\
        $(SLB)$/fltshared.lib	\
        $(SLB)$/$(TARGET).lib
        
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(TKLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(UCBHELPERLIB)		\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(SVTOOLLIB)		\
    $(SOTLIB)			\
    $(SO2LIB)			\
    $(SALLIB)

.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL1STDLIBS+= \
        -ldba$(DLLPOSTFIX)
.ELSE
SHL1STDLIBS+= \
        idba.lib
.ENDIF


SHL1DEPN=
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=	i$(SHL1TARGET)
# SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
