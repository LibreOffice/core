#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-08-02 15:18:11 $
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
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbaxml

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	svpre.mk
.INCLUDE :  	settings.mk
.INCLUDE :  	sv.mk

# --- Types -------------------------------------

UNOTYPES=\
    com.sun.star.uno.RuntimeException							\
    com.sun.star.uno.TypeClass									\
    com.sun.star.uno.XInterface									\
    com.sun.star.uno.XWeak										\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.io.XInputStream								\
    com.sun.star.io.XOutputStream								\
    com.sun.star.lang.XComponent								\
    com.sun.star.lang.XInitialization							\
    com.sun.star.lang.XMultiServiceFactory						\
    com.sun.star.lang.XSingleServiceFactory						\
    com.sun.star.lang.XServiceInfo								\
    com.sun.star.loader.XImplementationLoader					\
    com.sun.star.registry.XImplementationRegistration			\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.registry.XSimpleRegistry						\
    com.sun.star.document.XFilter								\
    com.sun.star.document.XExporter								\
    com.sun.star.container.XIndexAccess
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
            $(SLO)$/xmlservices.obj

# --- Library -----------------------------------
LIB1TARGET=$(SLB)$/dba_flt.lib
LIB1FILES=\
        $(SLB)$/fltshared.lib	\
        $(SLB)$/$(TARGET).lib
        
SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(SVXLIB)			\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(UCBHELPERLIB)		\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(SOTLIB)			\
    $(SO2LIB)			\
    $(SALLIB)


SHL1DEPN=
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=	i$(SHL1TARGET)
# SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
