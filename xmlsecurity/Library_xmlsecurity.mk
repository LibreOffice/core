# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,xmlsecurity))

$(eval $(call gb_Library_set_componentfile,xmlsecurity,xmlsecurity/util/xmlsecurity))

$(eval $(call gb_Library_set_include,xmlsecurity,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
))

$(eval $(call gb_Library_use_sdk_api,xmlsecurity))

$(eval $(call gb_Library_use_libraries,xmlsecurity,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	svl \
	svt \
	svxcore \
	tl \
	ucbhelper \
	utl \
	vcl \
	xo \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,xmlsecurity,\
	xmlsecurity/source/component/certificatecontainer \
	xmlsecurity/source/component/documentdigitalsignatures \
	xmlsecurity/source/component/registerservices \
	xmlsecurity/source/dialogs/certificatechooser \
	xmlsecurity/source/dialogs/certificateviewer \
	xmlsecurity/source/dialogs/digitalsignaturesdialog \
	xmlsecurity/source/dialogs/macrosecurity \
	xmlsecurity/source/dialogs/resourcemanager \
	xmlsecurity/source/helper/documentsignaturehelper \
	xmlsecurity/source/helper/xmlsignaturehelper2 \
	xmlsecurity/source/helper/xmlsignaturehelper \
	xmlsecurity/source/helper/xsecctl \
	xmlsecurity/source/helper/xsecparser \
	xmlsecurity/source/helper/xsecsign \
	xmlsecurity/source/helper/xsecverify \
))

# vim: set noet sw=4 ts=4:
