# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,dbu))

$(eval $(call gb_Library_set_include,dbu,\
    $$(INCLUDE) \
    -I$(SRCDIR)/dbaccess/inc \
    -I$(SRCDIR)/dbaccess/source/inc \
    -I$(SRCDIR)/dbaccess/source/ui/inc \
    -I$(call gb_SrsTemplateTarget_get_include_dir,dbaccess) \
))

$(eval $(call gb_Library_use_sdk_api,dbu))

$(eval $(call gb_Library_add_defs,dbu,\
    -DDBACCESS_DLLIMPLEMENTATION \
))

ifeq ($(GUI)$(COM),WNTGCC) # for adoint.h
$(eval $(call gb_Library_add_cxxflags,dbu,\
    -fpermissive \
))
endif

$(eval $(call gb_Library_use_external,dbu,odbc_headers))

$(eval $(call gb_Library_use_libraries,dbu,\
    comphelper \
    cppu \
    cppuhelper \
    dba \
    dbtools \
    editeng \
    fwe \
    sal \
    salhelper \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_use_system_win32_libs,dbu,\
    ole32 \
    oleaut32 \
    uuid \
))
endif

$(eval $(call gb_Library_set_componentfile,dbu,dbaccess/util/dbu))

$(eval $(call gb_Library_add_exception_objects,dbu,\
    dbaccess/source/shared/dbu_reghelper \
    dbaccess/source/shared/dbustrings \
    dbaccess/source/ui/app/AppController \
    dbaccess/source/ui/app/AppControllerDnD \
    dbaccess/source/ui/app/AppControllerGen \
    dbaccess/source/ui/app/AppDetailPageHelper \
    dbaccess/source/ui/app/AppDetailView \
    dbaccess/source/ui/app/AppIconControl \
    dbaccess/source/ui/app/AppSwapWindow \
    dbaccess/source/ui/app/AppTitleWindow \
    dbaccess/source/ui/app/AppView \
    dbaccess/source/ui/app/subcomponentmanager \
    dbaccess/source/ui/browser/AsyncronousLink \
    dbaccess/source/ui/browser/brwctrlr \
    dbaccess/source/ui/browser/brwview \
    dbaccess/source/ui/browser/dataview \
    dbaccess/source/ui/browser/dbexchange \
    dbaccess/source/ui/browser/dbloader \
    dbaccess/source/ui/browser/dbtreemodel \
    dbaccess/source/ui/browser/dbtreeview \
    dbaccess/source/ui/browser/dsbrowserDnD \
    dbaccess/source/ui/browser/dsEntriesNoExp \
    dbaccess/source/ui/browser/exsrcbrw \
    dbaccess/source/ui/browser/formadapter \
    dbaccess/source/ui/browser/genericcontroller \
    dbaccess/source/ui/browser/sbagrid \
    dbaccess/source/ui/browser/sbamultiplex \
    dbaccess/source/ui/browser/unodatbr \
    dbaccess/source/ui/control/charsetlistbox \
    dbaccess/source/ui/control/ColumnControlWindow \
    dbaccess/source/ui/control/curledit \
    dbaccess/source/ui/control/dbtreelistbox \
    dbaccess/source/ui/control/FieldDescControl \
    dbaccess/source/ui/control/listviewitems \
    dbaccess/source/ui/control/marktree \
    dbaccess/source/ui/control/opendoccontrols \
    dbaccess/source/ui/control/RelationControl \
    dbaccess/source/ui/control/sqledit \
    dbaccess/source/ui/control/SqlNameEdit \
    dbaccess/source/ui/control/statusbarontroller \
    dbaccess/source/ui/control/TableGrantCtrl \
    dbaccess/source/ui/control/tabletree \
    dbaccess/source/ui/control/toolboxcontroller \
    dbaccess/source/ui/control/undosqledit \
    dbaccess/source/ui/control/VertSplitView \
    dbaccess/source/ui/dlg/admincontrols \
    dbaccess/source/ui/dlg/adminpages \
    dbaccess/source/ui/dlg/adtabdlg \
    dbaccess/source/ui/dlg/advancedsettings \
    dbaccess/source/ui/dlg/CollectionView \
    dbaccess/source/ui/dlg/ConnectionHelper \
    dbaccess/source/ui/dlg/ConnectionPage \
    dbaccess/source/ui/dlg/ConnectionPageSetup \
    dbaccess/source/ui/dlg/dbadmin \
    dbaccess/source/ui/dlg/DbAdminImpl \
    dbaccess/source/ui/dlg/dbfindex \
    dbaccess/source/ui/dlg/DBSetupConnectionPages \
    dbaccess/source/ui/dlg/dbwiz \
    dbaccess/source/ui/dlg/dbwizsetup \
    dbaccess/source/ui/dlg/detailpages \
    dbaccess/source/ui/dlg/directsql \
    dbaccess/source/ui/dlg/dlgattr \
    dbaccess/source/ui/dlg/dlgsave \
    dbaccess/source/ui/dlg/dlgsize \
    dbaccess/source/ui/dlg/DriverSettings \
    dbaccess/source/ui/dlg/dsselect \
    dbaccess/source/ui/dlg/finteraction \
    dbaccess/source/ui/dlg/generalpage \
    dbaccess/source/ui/dlg/indexdialog \
    dbaccess/source/ui/dlg/indexfieldscontrol \
    dbaccess/source/ui/dlg/odbcconfig \
    dbaccess/source/ui/dlg/optionalboolitem \
    dbaccess/source/ui/dlg/paramdialog \
    dbaccess/source/ui/dlg/queryfilter \
    dbaccess/source/ui/dlg/queryorder \
    dbaccess/source/ui/dlg/RelationDlg \
    dbaccess/source/ui/dlg/sqlmessage \
    dbaccess/source/ui/dlg/tablespage \
    dbaccess/source/ui/dlg/TablesSingleDlg \
    dbaccess/source/ui/dlg/TextConnectionHelper \
    dbaccess/source/ui/dlg/textconnectionsettings \
    dbaccess/source/ui/dlg/UserAdmin \
    dbaccess/source/ui/dlg/UserAdminDlg \
    dbaccess/source/ui/misc/asyncmodaldialog \
    dbaccess/source/ui/misc/charsets \
    dbaccess/source/ui/misc/controllerframe \
    dbaccess/source/ui/misc/databaseobjectview \
    dbaccess/source/ui/misc/datasourceconnector \
    dbaccess/source/ui/misc/dbaundomanager \
    dbaccess/source/ui/misc/dbsubcomponentcontroller \
    dbaccess/source/ui/misc/defaultobjectnamecheck \
    dbaccess/source/ui/misc/DExport \
    dbaccess/source/ui/misc/dsmeta \
    dbaccess/source/ui/misc/HtmlReader \
    dbaccess/source/ui/misc/imageprovider \
    dbaccess/source/ui/misc/indexcollection \
    dbaccess/source/ui/misc/linkeddocuments \
    dbaccess/source/ui/misc/moduledbu \
    dbaccess/source/ui/misc/propertysetitem \
    dbaccess/source/ui/misc/propertystorage \
    dbaccess/source/ui/misc/RowSetDrop \
    dbaccess/source/ui/misc/RtfReader \
    dbaccess/source/ui/misc/singledoccontroller \
    dbaccess/source/ui/misc/stringlistitem \
    dbaccess/source/ui/misc/TableCopyHelper \
    dbaccess/source/ui/misc/TokenWriter \
    dbaccess/source/ui/misc/ToolBoxHelper \
    dbaccess/source/ui/misc/uiservices \
    dbaccess/source/ui/misc/UITools \
    dbaccess/source/ui/misc/WColumnSelect \
    dbaccess/source/ui/misc/WCopyTable \
    dbaccess/source/ui/misc/WCPage \
    dbaccess/source/ui/misc/WExtendPages \
    dbaccess/source/ui/misc/WNameMatch \
    dbaccess/source/ui/misc/WTypeSelect \
    dbaccess/source/ui/querydesign/ConnectionLine \
    dbaccess/source/ui/querydesign/ConnectionLineAccess \
    dbaccess/source/ui/querydesign/ConnectionLineData \
    dbaccess/source/ui/querydesign/JAccess \
    dbaccess/source/ui/querydesign/JoinController \
    dbaccess/source/ui/querydesign/JoinDesignView \
    dbaccess/source/ui/querydesign/JoinExchange \
    dbaccess/source/ui/querydesign/JoinTableView \
    dbaccess/source/ui/querydesign/QTableConnection \
    dbaccess/source/ui/querydesign/QTableConnectionData \
    dbaccess/source/ui/querydesign/QTableWindow \
    dbaccess/source/ui/querydesign/QTableWindowData \
    dbaccess/source/ui/querydesign/querycontainerwindow \
    dbaccess/source/ui/querydesign/querycontroller \
    dbaccess/source/ui/querydesign/QueryDesignView \
    dbaccess/source/ui/querydesign/querydlg \
    dbaccess/source/ui/querydesign/QueryMoveTabWinUndoAct \
    dbaccess/source/ui/querydesign/QueryTabConnUndoAction \
    dbaccess/source/ui/querydesign/QueryTableView \
    dbaccess/source/ui/querydesign/QueryTabWinUndoAct \
    dbaccess/source/ui/querydesign/QueryTextView \
    dbaccess/source/ui/querydesign/queryview \
    dbaccess/source/ui/querydesign/QueryViewSwitch \
    dbaccess/source/ui/querydesign/SelectionBrowseBox \
    dbaccess/source/ui/querydesign/TableConnection \
    dbaccess/source/ui/querydesign/TableConnectionData \
    dbaccess/source/ui/querydesign/TableFieldDescription \
    dbaccess/source/ui/querydesign/TableWindow \
    dbaccess/source/ui/querydesign/TableWindowAccess \
    dbaccess/source/ui/querydesign/TableWindowData \
    dbaccess/source/ui/querydesign/TableWindowListBox \
    dbaccess/source/ui/querydesign/TableWindowTitle \
    dbaccess/source/ui/relationdesign/RelationController \
    dbaccess/source/ui/relationdesign/RelationDesignView \
    dbaccess/source/ui/relationdesign/RelationTableView \
    dbaccess/source/ui/relationdesign/RTableConnection \
    dbaccess/source/ui/relationdesign/RTableConnectionData \
    dbaccess/source/ui/tabledesign/FieldDescGenWin \
    dbaccess/source/ui/tabledesign/FieldDescriptions \
    dbaccess/source/ui/tabledesign/TableController \
    dbaccess/source/ui/tabledesign/TableDesignControl \
    dbaccess/source/ui/tabledesign/TableDesignHelpBar \
    dbaccess/source/ui/tabledesign/TableDesignView \
    dbaccess/source/ui/tabledesign/TableFieldControl \
    dbaccess/source/ui/tabledesign/TableFieldDescWin \
    dbaccess/source/ui/tabledesign/TableRow \
    dbaccess/source/ui/tabledesign/TableRowExchange \
    dbaccess/source/ui/tabledesign/TableUndo \
    dbaccess/source/ui/tabledesign/TEditControl \
    dbaccess/source/ui/uno/admindlg \
    dbaccess/source/ui/uno/AdvancedSettingsDlg \
    dbaccess/source/ui/uno/ColumnControl \
    dbaccess/source/ui/uno/ColumnModel \
    dbaccess/source/ui/uno/ColumnPeer \
    dbaccess/source/ui/uno/composerdialogs \
    dbaccess/source/ui/uno/dbinteraction \
    dbaccess/source/ui/uno/DBTypeWizDlg \
    dbaccess/source/ui/uno/DBTypeWizDlgSetup \
    dbaccess/source/ui/uno/TableFilterDlg \
    dbaccess/source/ui/uno/textconnectionsettings_uno \
    dbaccess/source/ui/uno/unoadmin \
    dbaccess/source/ui/uno/unoDirectSql \
    dbaccess/source/ui/uno/unosqlmessage \
    dbaccess/source/ui/uno/UserSettingsDlg \
))

$(eval $(call gb_Library_add_noexception_objects,dbu,\
    dbaccess/source/ui/control/ScrollHelper \
    dbaccess/source/ui/querydesign/TableFieldInfo \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_exception_objects,dbu,\
    dbaccess/source/ui/dlg/adodatalinks \
))
endif

# workaround for issue http://qa.openoffice.org/issues/show_bug.cgi?id=102305 Linux specific
ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxobjects,dbu,\
    dbaccess/source/ui/uno/copytablewizard \
    , $(gb_LinkTarget_EXCEPTIONFLAGS) $(gb_COMPILERNOOPTFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,dbu,\
    dbaccess/source/ui/uno/copytablewizard \
))
endif

$(call gb_Library_get_headers_target,dbu) : $(call gb_SrsTemplateTarget_get_include_dir,dbaccess)/AutoControls.hrc

# vim: set noet sw=4 ts=4:
