#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

$(eval $(call gb_Library_Library,dbui))
$(eval $(call gb_Library_add_precompiled_header,dbui,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbui))

$(eval $(call gb_Library_set_componentfile,dbui,dbaccess/util/dbu))

$(eval $(call gb_Library_set_include,dbui,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/ui/inc \
	-I$(SRCDIR)/dbaccess/source/ui/dlg \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(SOLARINC) \
))

$(eval $(call gb_Library_add_api,dbui,\
    offapi \
    udkapi \
))

ifeq ($(SYSTEM_ODBC_HEADERS),TRUE)
$(eval $(call gb_Library_add_cflags,dbui,\
	-DSYSTEM_ODBC_HEADERS \
))
endif

ifneq ($(WINDOWS_VISTA_PSDK),)
ifeq ($(PROF_EDITION),)
$(eval $(call gb_Library_add_defs,dbui,\
	-DDBACCESS_DLLIMPLEMENTATION\
	-DWINDOWS_VISTA_PSDK \
))
endif
endif

ifeq ($(GUI)$(COM),WNTGCC)
$(eval $(call gb_Library_add_cflags,dbui,\
	-fpermissive \
))
endif

ifeq ($(PROF_EDITION),)
ifneq ($(WINDOWS_VISTA_PSDK),)
DISABLE_ADO=TRUE
endif
endif

ifeq ($(OS),WNT)
ifeq ($(DISABLE_ADO),)
$(eval $(call gb_Library_add_exception_objects,dbui,\
	dbaccess/source/ui/dlg/adodatalinks \
))
endif
endif

$(eval $(call gb_Library_add_defs,dbui,\
	-DDBACCESS_DLLIMPLEMENTATION\
))

$(eval $(call gb_Library_add_linked_libs,dbui,\
	comphelper \
	cppuhelper \
	cppu \
	dbtools \
	editeng \
	i18nisolang1 \
	fwe \
	sal \
	salhelper \
	sfx \
	svl \
	svt \
	vcl \
	tl \
	utl \
	ucbhelper \
	svxcore \
	svx \
	tk \
	sot \
	dba \
    $(gb_STDLIBS) \
))
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,dbui,\
	ole32 \
	oleaut32 \
	uuid \
))
endif

$(eval $(call gb_Library_add_exception_objects,dbui,\
	dbaccess/source/ui/app/AppDetailView \
	dbaccess/source/ui/app/AppController \
	dbaccess/source/ui/app/AppControllerDnD \
	dbaccess/source/ui/app/AppControllerGen \
	dbaccess/source/ui/app/AppDetailPageHelper \
	dbaccess/source/ui/app/AppSwapWindow \
	dbaccess/source/ui/app/AppTitleWindow \
	dbaccess/source/ui/app/AppView \
	dbaccess/source/ui/app/closeveto \
	dbaccess/source/ui/app/subcomponentmanager \
	dbaccess/source/ui/browser/genericcontroller \
	dbaccess/source/ui/browser/dsbrowserDnD \
	dbaccess/source/ui/browser/dataview \
	dbaccess/source/ui/browser/dbexchange \
	dbaccess/source/ui/browser/dbloader \
	dbaccess/source/ui/browser/sbagrid \
	dbaccess/source/ui/browser/formadapter \
	dbaccess/source/ui/browser/brwview \
	dbaccess/source/ui/browser/brwctrlr \
	dbaccess/source/ui/browser/unodatbr \
	dbaccess/source/ui/browser/sbamultiplex \
	dbaccess/source/ui/browser/exsrcbrw \
	dbaccess/source/ui/browser/dbtreemodel \
	dbaccess/source/ui/browser/dsEntriesNoExp \
	dbaccess/source/ui/browser/dbtreeview \
	dbaccess/source/ui/control/statusbarontroller \
	dbaccess/source/ui/control/RelationControl \
	dbaccess/source/ui/control/toolboxcontroller \
	dbaccess/source/ui/control/tabletree \
	dbaccess/source/ui/control/TableGrantCtrl \
	dbaccess/source/ui/control/dbtreelistbox \
	dbaccess/source/ui/control/sqledit \
	dbaccess/source/ui/control/ColumnControlWindow \
	dbaccess/source/ui/control/FieldDescControl \
	dbaccess/source/ui/control/opendoccontrols \
	dbaccess/source/ui/dlg/directsql \
	dbaccess/source/ui/dlg/dbwiz \
	dbaccess/source/ui/dlg/dbwizsetup \
	dbaccess/source/ui/dlg/generalpage \
	dbaccess/source/ui/dlg/indexfieldscontrol \
	dbaccess/source/ui/dlg/indexdialog \
	dbaccess/source/ui/dlg/RelationDlg \
	dbaccess/source/ui/dlg/adtabdlg \
	dbaccess/source/ui/dlg/dlgsave \
	dbaccess/source/ui/dlg/admincontrols \
	dbaccess/source/ui/dlg/adminpages \
	dbaccess/source/ui/dlg/queryorder \
	dbaccess/source/ui/dlg/queryfilter \
	dbaccess/source/ui/dlg/paramdialog \
	dbaccess/source/ui/dlg/dbadmin \
	dbaccess/source/ui/dlg/detailpages \
	dbaccess/source/ui/dlg/sqlmessage \
	dbaccess/source/ui/dlg/finteraction \
	dbaccess/source/ui/dlg/ConnectionPage \
	dbaccess/source/ui/dlg/ConnectionHelper \
	dbaccess/source/ui/dlg/TextConnectionHelper \
	dbaccess/source/ui/dlg/ConnectionPageSetup \
	dbaccess/source/ui/dlg/DBSetupConnectionPages \
	dbaccess/source/ui/dlg/UserAdminDlg \
	dbaccess/source/ui/dlg/UserAdmin \
	dbaccess/source/ui/dlg/AdabasStat \
	dbaccess/source/ui/dlg/tablespage \
	dbaccess/source/ui/dlg/DbAdminImpl \
	dbaccess/source/ui/dlg/TablesSingleDlg \
	dbaccess/source/ui/dlg/CollectionView \
	dbaccess/source/ui/dlg/dsselect \
	dbaccess/source/ui/dlg/dbfindex \
	dbaccess/source/ui/dlg/DriverSettings \
	dbaccess/source/ui/dlg/odbcconfig \
	dbaccess/source/ui/dlg/advancedsettings \
	dbaccess/source/ui/dlg/textconnectionsettings \
	dbaccess/source/ui/misc/asyncmodaldialog \
	dbaccess/source/ui/misc/imageprovider \
	dbaccess/source/ui/misc/singledoccontroller \
	dbaccess/source/ui/misc/dbsubcomponentcontroller \
	dbaccess/source/ui/misc/datasourceconnector \
	dbaccess/source/ui/misc/linkeddocuments \
	dbaccess/source/ui/misc/indexcollection \
	dbaccess/source/ui/misc/UITools \
	dbaccess/source/ui/misc/WCPage \
	dbaccess/source/ui/misc/WCopyTable \
	dbaccess/source/ui/misc/WTypeSelect \
	dbaccess/source/ui/misc/TokenWriter \
	dbaccess/source/ui/misc/HtmlReader \
	dbaccess/source/ui/misc/RtfReader \
	dbaccess/source/ui/misc/propertysetitem \
	dbaccess/source/ui/misc/databaseobjectview \
	dbaccess/source/ui/misc/DExport \
	dbaccess/source/ui/misc/uiservices \
	dbaccess/source/ui/misc/RowSetDrop \
	dbaccess/source/ui/misc/TableCopyHelper \
	dbaccess/source/ui/misc/moduledbu \
	dbaccess/source/ui/misc/WColumnSelect \
	dbaccess/source/ui/misc/WExtendPages \
	dbaccess/source/ui/misc/WNameMatch \
	dbaccess/source/ui/misc/ToolBoxHelper \
	dbaccess/source/ui/misc/stringlistitem \
	dbaccess/source/ui/misc/charsets \
	dbaccess/source/ui/misc/defaultobjectnamecheck \
	dbaccess/source/ui/misc/dsmeta \
	dbaccess/source/ui/misc/controllerframe \
	dbaccess/source/ui/misc/propertystorage \
	dbaccess/source/ui/misc/dbaundomanager \
	dbaccess/source/ui/querydesign/ConnectionLineAccess \
	dbaccess/source/ui/querydesign/TableWindowAccess \
	dbaccess/source/ui/querydesign/JAccess \
	dbaccess/source/ui/querydesign/JoinDesignView \
	dbaccess/source/ui/querydesign/JoinController \
	dbaccess/source/ui/querydesign/QueryDesignView \
	dbaccess/source/ui/querydesign/SelectionBrowseBox \
	dbaccess/source/ui/querydesign/querydlg \
	dbaccess/source/ui/querydesign/QueryTabWinUndoAct \
	dbaccess/source/ui/querydesign/QueryMoveTabWinUndoAct \
	dbaccess/source/ui/querydesign/QueryTabConnUndoAction \
	dbaccess/source/ui/querydesign/TableFieldDescription \
	dbaccess/source/ui/querydesign/JoinTableView \
	dbaccess/source/ui/querydesign/QueryViewSwitch \
	dbaccess/source/ui/querydesign/QueryTableView \
	dbaccess/source/ui/querydesign/TableWindowData \
	dbaccess/source/ui/querydesign/QTableWindow \
	dbaccess/source/ui/querydesign/TableWindow \
	dbaccess/source/ui/querydesign/JoinExchange \
	dbaccess/source/ui/querydesign/TableWindowListBox \
	dbaccess/source/ui/querydesign/TableWindowTitle \
	dbaccess/source/ui/querydesign/QueryTextView \
	dbaccess/source/ui/querydesign/QTableConnection \
	dbaccess/source/ui/querydesign/querycontroller \
	dbaccess/source/ui/querydesign/querycontainerwindow \
	dbaccess/source/ui/querydesign/queryview \
	dbaccess/source/ui/querydesign/TableConnection \
	dbaccess/source/ui/querydesign/TableConnectionData \
	dbaccess/source/ui/relationdesign/RTableConnection \
	dbaccess/source/ui/relationdesign/RTableConnectionData \
	dbaccess/source/ui/relationdesign/RelationTableView \
	dbaccess/source/ui/relationdesign/RelationDesignView \
	dbaccess/source/ui/relationdesign/RelationController \
	dbaccess/source/ui/tabledesign/TableDesignControl \
	dbaccess/source/ui/tabledesign/TableFieldDescWin \
	dbaccess/source/ui/tabledesign/TableController \
	dbaccess/source/ui/tabledesign/TableDesignView \
	dbaccess/source/ui/tabledesign/TEditControl \
	dbaccess/source/ui/tabledesign/TableFieldControl \
	dbaccess/source/ui/tabledesign/TableDesignHelpBar \
	dbaccess/source/ui/tabledesign/FieldDescGenWin \
	dbaccess/source/ui/tabledesign/FieldDescriptions \
	dbaccess/source/ui/tabledesign/TableRow \
	dbaccess/source/ui/tabledesign/TableRowExchange \
	dbaccess/source/ui/tabledesign/TableUndo \
	dbaccess/source/ui/uno/composerdialogs \
	dbaccess/source/ui/uno/unosqlmessage \
	dbaccess/source/ui/uno/unoadmin \
	dbaccess/source/ui/uno/admindlg \
	dbaccess/source/ui/uno/TableFilterDlg \
	dbaccess/source/ui/uno/AdvancedSettingsDlg \
	dbaccess/source/ui/uno/unoDirectSql \
	dbaccess/source/ui/uno/DBTypeWizDlg \
	dbaccess/source/ui/uno/DBTypeWizDlgSetup \
	dbaccess/source/ui/uno/UserSettingsDlg \
	dbaccess/source/ui/uno/ColumnModel \
	dbaccess/source/ui/uno/ColumnControl \
	dbaccess/source/ui/uno/ColumnPeer \
	dbaccess/source/ui/uno/dbinteraction \
	dbaccess/source/ui/uno/textconnectionsettings_uno \
))

$(eval $(call gb_Library_add_noexception_objects,dbui, \
	dbaccess/source/ui/control/ScrollHelper		\
	dbaccess/source/ui/control/VertSplitView		\
	dbaccess/source/ui/control/SqlNameEdit			\
	dbaccess/source/ui/control/listviewitems		\
	dbaccess/source/ui/control/undosqledit			\
	dbaccess/source/ui/control/marktree			\
	dbaccess/source/ui/control/curledit            \
	dbaccess/source/ui/control/charsetlistbox		\
	dbaccess/source/ui/app/AppIconControl \
	dbaccess/source/ui/browser/AsyncronousLink \
	dbaccess/source/ui/dlg/dlgsize \
	dbaccess/source/ui/dlg/dlgattr \
	dbaccess/source/ui/dlg/optionalboolitem \
	dbaccess/source/ui/querydesign/TableFieldInfo \
	dbaccess/source/ui/querydesign/QTableConnectionData \
	dbaccess/source/ui/querydesign/QTableWindowData \
	dbaccess/source/ui/querydesign/ConnectionLine \
	dbaccess/source/ui/querydesign/ConnectionLineData \
	dbaccess/source/shared/dbu_reghelper \
	dbaccess/source/shared/dbustrings \
))

# the following source file can't be compiled with optimization by some compilers (crash or endless loop):
# Solaris Sparc with Sun compiler, gcc on MacOSX and Linux PPC
# the latter is currently not supported by gbuild and needs a fix here later
ifeq ($(COM),$(filter-out GCC,$(COM)))
$(eval $(call gb_Library_add_exception_objects,dbui,\
    dbaccess/source/ui/uno/copytablewizard \
))
else
$(eval $(call gb_Library_add_cxxobjects,dbui,\
    dbaccess/source/ui/uno/copytablewizard \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
