#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 16:47:49 $
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
        -ldbu$(DLLPOSTFIX)
.ELSE
SHL1STDLIBS+= \
        idbu.lib
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
