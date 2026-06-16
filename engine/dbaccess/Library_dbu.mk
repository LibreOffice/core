# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbu))

$(eval $(call gb_Library_set_include,dbu,\
    $$(INCLUDE) \
    -I$(SRCDIR)/dbaccess/inc \
    -I$(SRCDIR)/dbaccess/source/inc \
    -I$(SRCDIR)/dbaccess/source/ui/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_set_precompiled_header,dbu,dbaccess/inc/pch/precompiled_dbu))

$(eval $(call gb_Library_use_custom_headers,dbu,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,dbu))

$(eval $(call gb_Library_add_defs,dbu,\
    -DDBACCESS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,dbu,\
	boost_headers \
	odbc_headers \
))

$(eval $(call gb_Library_use_libraries,dbu,\
    comphelper \
    cppu \
    cppuhelper \
    dba \
    dbtools \
    editeng \
    fwk \
    sal \
    salhelper \
	i18nlangtag \
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
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_cxxflags,dbu,\
	-DADO_SUPPRESS_MESSAGE \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,dbu,\
    ole32 \
    oleaut32 \
    uuid \
))
endif

$(eval $(call gb_Library_set_componentfile,dbu,dbaccess/util/dbu,services))

$(eval $(call gb_Library_add_exception_objects,dbu,\
    dbaccess/source/ui/browser/AsynchronousLink \
    dbaccess/source/ui/browser/brwctrlr \
    dbaccess/source/ui/browser/brwview \
    dbaccess/source/ui/browser/dataview \
    dbaccess/source/ui/browser/dbexchange \
    dbaccess/source/ui/browser/dbloader \
    dbaccess/source/ui/browser/dbtreemodel \
    dbaccess/source/ui/browser/dsbrowserDnD \
    dbaccess/source/ui/browser/dsEntriesNoExp \
    dbaccess/source/ui/browser/genericcontroller \
    dbaccess/source/ui/browser/sbagrid \
    dbaccess/source/ui/browser/sbamultiplex \
    dbaccess/source/ui/browser/unodatbr \
    dbaccess/source/ui/control/charsetlistbox \
    dbaccess/source/ui/control/ColumnControlWindow \
    dbaccess/source/ui/control/curledit \
    dbaccess/source/ui/control/dbtreelistbox \
    dbaccess/source/ui/control/FieldControls \
    dbaccess/source/ui/control/FieldDescControl \
    dbaccess/source/ui/control/SqlNameEdit \
    dbaccess/source/ui/dlg/admincontrols \
    dbaccess/source/ui/dlg/adminpages \
    dbaccess/source/ui/dlg/CollectionView \
    dbaccess/source/ui/dlg/ConnectionHelper \
    dbaccess/source/ui/dlg/ConnectionPage \
    dbaccess/source/ui/dlg/dbadmin \
    dbaccess/source/ui/dlg/DbAdminImpl \
    dbaccess/source/ui/dlg/dbfindex \
    dbaccess/source/ui/dlg/dbwiz \
    dbaccess/source/ui/dlg/detailpages \
    dbaccess/source/ui/dlg/dlgattr \
    dbaccess/source/ui/dlg/dlgsave \
    dbaccess/source/ui/dlg/dlgsize \
    dbaccess/source/ui/dlg/dsselect \
    dbaccess/source/ui/dlg/finteraction \
    dbaccess/source/ui/dlg/generalpage \
    dbaccess/source/ui/dlg/odbcconfig \
    dbaccess/source/ui/dlg/optionalboolitem \
    dbaccess/source/ui/dlg/paramdialog \
    dbaccess/source/ui/dlg/queryfilter \
    dbaccess/source/ui/dlg/queryorder \
    dbaccess/source/ui/dlg/sqlmessage \
    dbaccess/source/ui/dlg/TextConnectionHelper \
    dbaccess/source/ui/dlg/textconnectionsettings \
    dbaccess/source/ui/misc/charsets \
    dbaccess/source/ui/misc/controllerframe \
    dbaccess/source/ui/misc/datasourceconnector \
    dbaccess/source/ui/misc/defaultobjectnamecheck \
    dbaccess/source/ui/misc/DExport \
    dbaccess/source/ui/misc/dsmeta \
    dbaccess/source/ui/misc/FieldDescriptions \
    dbaccess/source/ui/misc/HtmlReader \
    dbaccess/source/ui/misc/imageprovider \
    dbaccess/source/ui/misc/propertystorage \
    dbaccess/source/ui/misc/RowSetDrop \
    dbaccess/source/ui/misc/RtfReader \
    dbaccess/source/ui/misc/stringlistitem \
    dbaccess/source/ui/misc/TableCopyHelper \
    dbaccess/source/ui/misc/TokenWriter \
    dbaccess/source/ui/misc/UITools \
    dbaccess/source/ui/misc/WColumnSelect \
    dbaccess/source/ui/misc/WCopyTable \
    dbaccess/source/ui/misc/WCPage \
    dbaccess/source/ui/misc/WExtendPages \
    dbaccess/source/ui/misc/WNameMatch \
    dbaccess/source/ui/misc/WTypeSelect \
    dbaccess/source/ui/uno/admindlg \
    dbaccess/source/ui/uno/ColumnControl \
    dbaccess/source/ui/uno/ColumnPeer \
    dbaccess/source/ui/uno/composerdialogs \
	dbaccess/source/ui/uno/copytablewizard \
    dbaccess/source/ui/uno/dbinteraction \
    dbaccess/source/ui/uno/DBTypeWizDlg \
    dbaccess/source/ui/uno/textconnectionsettings_uno \
    dbaccess/source/ui/uno/unoadmin \
    dbaccess/source/ui/uno/unosqlmessage \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,dbu,\
    dbaccess/source/ui/dlg/adodatalinks \
))
endif

# vim: set noet sw=4 ts=4:
