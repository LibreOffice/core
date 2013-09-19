# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_InternalUnoApi_InternalUnoApi,touch,touch/idl))

$(eval $(call gb_InternalUnoApi_use_api,touch,\
	udkapi \
))

$(eval $(call gb_InternalUnoApi_define_api_dependencies,touch, \
	offapi,\
	udkapi \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,touch,org/libreoffice/touch,\
	Document \
	XDocument \
	XDocumentRenderCallback \
))

# vim: set noet sw=4 ts=4:
