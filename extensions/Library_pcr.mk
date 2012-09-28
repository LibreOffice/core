# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# [ Copyright (C) 2011 Red Hat, Inc., Michael Stahl <mstahl@redhat.com> (initial developer) ]
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
#

$(eval $(call gb_Library_Library,pcr))

$(eval $(call gb_Library_set_componentfile,pcr,extensions/source/propctrlr/pcr))

$(eval $(call gb_Library_use_sdk_api,pcr))

$(eval $(call gb_Library_set_include,pcr,\
	-I$(SRCDIR)/extensions/inc \
	$$(INCLUDE) \
))

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
	i18nisolang1 \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	$(gb_UWINAPI) \
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
	extensions/source/propctrlr/formstrings \
	extensions/source/propctrlr/genericpropertyhandler \
	extensions/source/propctrlr/handlerhelper \
	extensions/source/propctrlr/inspectorhelpwindow \
	extensions/source/propctrlr/inspectormodelbase \
	extensions/source/propctrlr/listselectiondlg \
	extensions/source/propctrlr/modulepcr \
	extensions/source/propctrlr/newdatatype \
	extensions/source/propctrlr/objectinspectormodel \
	extensions/source/propctrlr/pcrcommon \
	extensions/source/propctrlr/pcrcomponentcontext \
	extensions/source/propctrlr/pcrservices \
	extensions/source/propctrlr/pcrstrings \
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
