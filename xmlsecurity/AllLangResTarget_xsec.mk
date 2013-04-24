# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,xmlsec))

$(eval $(call gb_AllLangResTarget_set_reslocation,xmlsec,xmlsecurity))

$(eval $(call gb_AllLangResTarget_add_srs,xmlsec,\
	xmlsec/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,xmlsec/res))

$(eval $(call gb_SrsTarget_set_include,xmlsec/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
	-I$(SRCDIR)/xmlsecurity/source/dialogs \
))

$(eval $(call gb_SrsTarget_add_files,xmlsec/res,\
	xmlsecurity/source/dialogs/certificateviewer.src \
	xmlsecurity/source/dialogs/macrosecurity.src \
	xmlsecurity/source/dialogs/digitalsignaturesdialog.src \
	xmlsecurity/source/component/warnbox.src \
))

# vim: set noet sw=4 ts=4:
