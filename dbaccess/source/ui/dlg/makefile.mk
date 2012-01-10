#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        $(SLO)$/directsql.obj			\
        $(SLO)$/dbwiz.obj				\
        $(SLO)$/dbwizsetup.obj				\
        $(SLO)$/generalpage.obj			\
        $(SLO)$/indexfieldscontrol.obj	\
        $(SLO)$/indexdialog.obj			\
        $(SLO)$/RelationDlg.obj			\
        $(SLO)$/adtabdlg.obj			\
        $(SLO)$/dlgsave.obj				\
        $(SLO)$/admincontrols.obj       \
        $(SLO)$/adminpages.obj			\
        $(SLO)$/queryorder.obj			\
        $(SLO)$/queryfilter.obj			\
        $(SLO)$/paramdialog.obj			\
        $(SLO)$/dbadmin.obj				\
        $(SLO)$/detailpages.obj			\
        $(SLO)$/sqlmessage.obj			\
        $(SLO)$/finteraction.obj		\
        $(SLO)$/ConnectionPage.obj		\
        $(SLO)$/ConnectionHelper.obj	\
        $(SLO)$/TextConnectionHelper.obj	\
        $(SLO)$/ConnectionPageSetup.obj	\
        $(SLO)$/DBSetupConnectionPages.obj 	\
        $(SLO)$/UserAdminDlg.obj		\
        $(SLO)$/UserAdmin.obj			\
        $(SLO)$/AdabasStat.obj			\
        $(SLO)$/tablespage.obj			\
        $(SLO)$/DbAdminImpl.obj			\
        $(SLO)$/TablesSingleDlg.obj		\
        $(SLO)$/CollectionView.obj		\
        $(SLO)$/dsselect.obj			\
        $(SLO)$/dbfindex.obj            \
        $(SLO)$/DriverSettings.obj      \
        $(SLO)$/odbcconfig.obj          \
        $(SLO)$/advancedsettings.obj    \
        $(SLO)$/textconnectionsettings.obj

SLOFILES=								\
        $(EXCEPTIONSFILES)				\
        $(SLO)$/dlgsize.obj				\
        $(SLO)$/dlgattr.obj             \
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
