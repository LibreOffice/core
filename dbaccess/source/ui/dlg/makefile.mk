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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uidlg

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

.IF "$(WINDOWS_VISTA_PSDK)"!="" && "$(PROF_EDITION)"==""
CDEFS+=-DWINDOWS_VISTA_PSDK
.ENDIF

.IF "$(GUI)$(COM)"=="WNTGCC" # for adoint.h
CFLAGS+=-fpermissive
.ENDIF

# --- Files -------------------------------------

# ... resource files ............................


LOCALIZE_ME=AutoControls_tmpl.hrc

SRS1NAME=$(TARGET)
SRC1FILES =	\
        AutoControls.src	\
        ConnectionPage.src	\
        UserAdmin.src		\
        admincontrols.src   \
        directsql.src		\
        AdabasStat.src		\
        indexdialog.src		\
        RelationDlg.src		\
        adtabdlg.src		\
        dlgsave.src			\
        queryorder.src		\
        queryfilter.src		\
        paramdialog.src		\
        dsselect.src		\
        dbadmin.src			\
        dbadmin2.src    	\
        dbadminsetup.src	\
        dbfindex.src		\
        dlgsize.src			\
        CollectionView.src	\
        dlgattr.src			\
        advancedsettings.src\
        UserAdminDlg.src	\
        sqlmessage.src      \
        textconnectionsettings.src


# ... object files ............................
EXCEPTIONSFILES=						\
        $(SLO)$/AdabasStat.obj			\
        $(SLO)$/CollectionView.obj		\
        $(SLO)$/ConnectionHelper.obj		\
        $(SLO)$/ConnectionPage.obj		\
        $(SLO)$/ConnectionPageSetup.obj		\
        $(SLO)$/DBSetupConnectionPages.obj 	\
        $(SLO)$/DbAdminImpl.obj			\
        $(SLO)$/DriverSettings.obj      	\
        $(SLO)$/RelationDlg.obj			\
        $(SLO)$/TablesSingleDlg.obj		\
        $(SLO)$/TextConnectionHelper.obj	\
        $(SLO)$/UserAdmin.obj			\
        $(SLO)$/UserAdminDlg.obj		\
        $(SLO)$/admincontrols.obj       	\
        $(SLO)$/adminpages.obj			\
        $(SLO)$/adtabdlg.obj			\
        $(SLO)$/advancedsettings.obj    	\
        $(SLO)$/dbadmin.obj			\
        $(SLO)$/dbfindex.obj            	\
        $(SLO)$/dbwiz.obj			\
        $(SLO)$/dbwizsetup.obj			\
        $(SLO)$/detailpages.obj			\
        $(SLO)$/directsql.obj			\
        $(SLO)$/dlgattr.obj             	\
        $(SLO)$/dlgsave.obj			\
        $(SLO)$/dlgsize.obj			\
        $(SLO)$/dsselect.obj			\
        $(SLO)$/finteraction.obj		\
        $(SLO)$/generalpage.obj			\
        $(SLO)$/indexdialog.obj			\
        $(SLO)$/indexfieldscontrol.obj		\
        $(SLO)$/odbcconfig.obj          	\
        $(SLO)$/paramdialog.obj			\
        $(SLO)$/queryfilter.obj			\
        $(SLO)$/queryorder.obj			\
        $(SLO)$/sqlmessage.obj			\
        $(SLO)$/tablespage.obj			\
        $(SLO)$/textconnectionsettings.obj

SLOFILES=					\
        $(EXCEPTIONSFILES)			\
        $(SLO)$/optionalboolitem.obj 

.IF "$(WINDOWS_VISTA_PSDK)"!="" && "$(PROF_EDITION)"==""
DISABLE_ADO=TRUE
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" && "$(PROF_EDITION)"==""

.IF "$(GUI)"=="WNT" && "$(DISABLE_ADO)"==""
SLOFILES+=		$(SLO)$/adodatalinks.obj
.ENDIF

# --- Targets ----------------------------------
.INCLUDE : target.mk

$(SLO)$/ConnectionHelper.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/ConnectionPageSetup.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/DBSetupConnectionPages.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/TextConnectionHelper.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/detailpages.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/tablespage.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/ConnectionPage.obj : $(LOCALIZE_ME_DEST)
