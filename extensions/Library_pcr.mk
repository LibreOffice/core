# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,pcr))

$(eval $(call gb_Library_set_componentfile,pcr,extensions/source/propctrlr/pcr,services))

$(eval $(call gb_Library_use_sdk_api,pcr))

$(eval $(call gb_Library_set_include,pcr,\
	-I$(SRCDIR)/extensions/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_common_precompiled_header,pcr))

$(eval $(call gb_Library_use_external,pcr,boost_headers))

$(eval $(call gb_Library_use_libraries,pcr,\
	svx \
	svxcore \
	editeng \
	sfx \
	svt \
	tk \
	vcl \
	svl \
	utl \
	tl \
	dbtools \
	i18nlangtag \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
))

$(eval $(call gb_Library_add_exception_objects,pcr,\
	extensions/source/propctrlr/MasterDetailLinkDialog \
	extensions/source/propctrlr/browserline \
	extensions/source/propctrlr/browserlistbox \
	extensions/source/propctrlr/browserpage \
	extensions/source/propctrlr/browserview \
	extensions/source/propctrlr/buttonnavigationhandler \
	extensions/source/propctrlr/cellbindinghandler \
	extensions/source/propctrlr/cellbindinghelper \
	extensions/source/propctrlr/commoncontrol \
	extensions/source/propctrlr/composeduiupdate \
	extensions/source/propctrlr/controlfontdialog \
	extensions/source/propctrlr/defaultforminspection \
	extensions/source/propctrlr/defaulthelpprovider \
	extensions/source/propctrlr/editpropertyhandler \
	extensions/source/propctrlr/eformshelper \
	extensions/source/propctrlr/eformspropertyhandler \
	extensions/source/propctrlr/eventhandler \
	extensions/source/propctrlr/fontdialog \
	extensions/source/propctrlr/formbrowsertools \
	extensions/source/propctrlr/formcomponenthandler \
	extensions/source/propctrlr/formcontroller \
	extensions/source/propctrlr/formgeometryhandler \
	extensions/source/propctrlr/formlinkdialog \
	extensions/source/propctrlr/formmetadata \
	extensions/source/propctrlr/genericpropertyhandler \
	extensions/source/propctrlr/handlerhelper \
	extensions/source/propctrlr/inspectorhelpwindow \
	extensions/source/propctrlr/inspectormodelbase \
	extensions/source/propctrlr/listselectiondlg \
	extensions/source/propctrlr/modulepcr \
	extensions/source/propctrlr/newdatatype \
	extensions/source/propctrlr/objectinspectormodel \
	extensions/source/propctrlr/pcrcommon \
	extensions/source/propctrlr/pcrunodialogs \
	extensions/source/propctrlr/propcontroller \
	extensions/source/propctrlr/propertycomposer \
	extensions/source/propctrlr/propertycontrolextender \
	extensions/source/propctrlr/propertyeditor \
	extensions/source/propctrlr/propertyhandler \
	extensions/source/propctrlr/propeventtranslation \
	extensions/source/propctrlr/pushbuttonnavigation \
	extensions/source/propctrlr/selectlabeldialog \
	extensions/source/propctrlr/sqlcommanddesign \
	extensions/source/propctrlr/standardcontrol \
	extensions/source/propctrlr/stringrepresentation \
	extensions/source/propctrlr/submissionhandler \
	extensions/source/propctrlr/taborder \
	extensions/source/propctrlr/unourl \
	extensions/source/propctrlr/usercontrol \
	extensions/source/propctrlr/xsddatatypes \
	extensions/source/propctrlr/xsdvalidationhelper \
	extensions/source/propctrlr/xsdvalidationpropertyhandler \
))

# vim:set noet sw=4 ts=4:
