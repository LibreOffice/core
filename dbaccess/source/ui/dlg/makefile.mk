#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.41 $
#
#   last change: $Author: obo $ $Date: 2008-03-10 09:00:55 $
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

SRS1NAME=$(TARGET)
SRC1FILES =	\
        AutoControls.src	\
        ConnectionPage.src	\
        UserAdmin.src		\
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
        AdabasStatDlg.src	\
        UserAdminDlg.src	\
        sqlmessage.src      \
        ExtensionNotPresent.src \
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
        $(SLO)$/AdabasStatDlg.obj		\
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
        $(SLO)$/ExtensionNotPresent.obj \
        $(SLO)$/advancedsettings.obj    \
        $(SLO)$/datasourceui.obj	\
        $(SLO)$/textconnectionsettings.obj

SLOFILES=								\
        $(EXCEPTIONSFILES)				\
        $(SLO)$/dlgsize.obj				\
        $(SLO)$/dlgattr.obj 

.IF "$(WINDOWS_VISTA_PSDK)"!="" && "$(PROF_EDITION)"==""
DISABLE_ADO=TRUE
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" && "$(PROF_EDITION)"==""

.IF "$(GUI)"=="WNT" && "$(DISABLE_ADO)"==""
SLOFILES+=		$(SLO)$/adodatalinks.obj
.ENDIF

# --- Targets ----------------------------------
LOCALIZE_ME=AutoControls_tmpl.hrc

.INCLUDE : target.mk

$(SLO)$/ConnectionHelper.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/ConnectionPageSetup.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/DBSetupConnectionPages.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/TextConnectionHelper.obj : $(LOCALIZE_ME_DEST)

$(SLO)$/detailpages.obj : $(LOCALIZE_ME_DEST)

