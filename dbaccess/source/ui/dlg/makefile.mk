 #########################################################################
 #
 #  $RCSfile: makefile.mk,v $
 #
 #  $Revision: 1.26 $
 #
 #  last change: $Author: hr $ $Date: 2004-08-02 15:49:43 $
 #
 #  The Contents of this file are made available subject to the terms of
 #  either of the following licenses
 #
 #         - GNU Lesser General Public License Version 2.1
 #         - Sun Industry Standards Source License Version 1.1
 #
 #  Sun Microsystems Inc., October, 2000
 #
 #  GNU Lesser General Public License Version 2.1
 #  =============================================
 #  Copyright 2000 by Sun Microsystems, Inc.
 #  901 San Antonio Road, Palo Alto, CA 94303, USA
 #
 #  This library is free software; you can redistribute it and/or
 #  modify it under the terms of the GNU Lesser General Public
 #  License version 2.1, as published by the Free Software Foundation.
 #
 #  This library is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 #  Lesser General Public License for more details.
 #
 #  You should have received a copy of the GNU Lesser General Public
 #  License along with this library; if not, write to the Free Software
 #  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 #  MA  02111-1307  USA
 #
 #
 #  Sun Industry Standards Source License Version 1.1
 #  =================================================
 #  The contents of this file are subject to the Sun Industry Standards
 #  Source License Version 1.1 (the "License"); You may not use this file
 #  except in compliance with the License. You may obtain a copy of the
 #  License at http://www.openoffice.org/license.html.
 #
 #  Software provided under this License is provided on an "AS IS" basis,
 #  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 #  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 #  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 #  See the License for the specific provisions governing your rights and
 #  obligations concerning the Software.
 #
 #  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 #
 #  Copyright: 2000 by Sun Microsystems, Inc.
 #
 #  All Rights Reserved.
 #
 #  Contributor(s): _______________________________________
 #
 #
 ########################################################################/

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uidlg

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        ConnectionPage.src	\
        UserAdmin.src	\
        directsql.src	\
        AdabasStat.src		\
        AdabasPage.src		\
        indexdialog.src		\
        RelationDlg.src		\
        adtabdlg.src		\
        dlgsave.src			\
        queryorder.src		\
        queryfilter.src		\
        paramdialog.src		\
        dsselect.src		\
        dbadmin2.src		\
        dbadmin.src			\
        dbfindex.src		\
        dlgsize.src			\
        CollectionView.src	\
        dlgattr.src			\
        AdvancedPageDlg.src \
        AdabasStatDlg.src	\
        UserAdminDlg.src	\
        sqlmessage.src

# ... object files ............................
EXCEPTIONSFILES=						\
        $(SLO)$/directsql.obj			\
        $(SLO)$/dbwiz.obj				\
        $(SLO)$/generalpage.obj			\
        $(SLO)$/indexfieldscontrol.obj	\
        $(SLO)$/indexdialog.obj			\
        $(SLO)$/RelationDlg.obj			\
        $(SLO)$/adtabdlg.obj			\
        $(SLO)$/dlgsave.obj				\
        $(SLO)$/queryorder.obj			\
        $(SLO)$/queryfilter.obj			\
        $(SLO)$/paramdialog.obj			\
        $(SLO)$/dbadmin.obj				\
        $(SLO)$/detailpages.obj			\
        $(SLO)$/sqlmessage.obj			\
        $(SLO)$/finteraction.obj		\
        $(SLO)$/ConnectionPage.obj		\
        $(SLO)$/AdvancedPageDlg.obj		\
        $(SLO)$/AdabasStatDlg.obj		\
        $(SLO)$/UserAdminDlg.obj		\
        $(SLO)$/UserAdmin.obj			\
        $(SLO)$/AdabasStat.obj			\
        $(SLO)$/tablespage.obj			\
        $(SLO)$/DbAdminImpl.obj			\
        $(SLO)$/TablesSingleDlg.obj		\
        $(SLO)$/CollectionView.obj		\
        $(SLO)$/dsselect.obj			\
        $(SLO)$/dbfindex.obj

SLOFILES=								\
        $(EXCEPTIONSFILES)				\
        $(SLO)$/DriverSettings.obj		\
        $(SLO)$/dlgsize.obj				\
        $(SLO)$/dlgattr.obj				\
        $(SLO)$/odbcconfig.obj			\
        $(SLO)$/adminpages.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=		$(SLO)$/adodatalinks.obj
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk


