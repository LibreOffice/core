# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,fps_office))

$(eval $(call gb_Library_set_componentfile,fps_office,fpicker/source/office/fps_office,services))

$(eval $(call gb_Library_set_include,fps_office,\
    $$(INCLUDE) \
    -I$(SRCDIR)/fpicker/inc \
))

$(eval $(call gb_Library_use_external,fps_office,boost_headers))

$(eval $(call gb_Library_use_custom_headers,fps_office,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,fps_office))

$(eval $(call gb_Library_use_libraries,fps_office,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	svl \
	svt \
	tk \
	tl \
	ucbhelper \
	utl \
	vcl \
	i18nlangtag \
))

$(eval $(call gb_Library_add_exception_objects,fps_office,\
	fpicker/source/office/asyncfilepicker \
	fpicker/source/office/autocmpledit \
	fpicker/source/office/breadcrumb \
	fpicker/source/office/commonpicker \
    fpicker/source/office/contentenumeration \
	fpicker/source/office/fileview \
    fpicker/source/office/foldertree \
	fpicker/source/office/fpinteraction \
	fpicker/source/office/fpsmartcontent \
	fpicker/source/office/iodlg \
	fpicker/source/office/iodlgimp \
	fpicker/source/office/OfficeControlAccess \
	fpicker/source/office/OfficeFilePicker \
	fpicker/source/office/OfficeFolderPicker \
	fpicker/source/office/PlacesListBox \
	fpicker/source/office/RemoteFilesDialog \
))

# vim: set noet sw=4 ts=4:
