# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libodfgen,odfgen))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libodfgen,inc/external/libodfgen,\
	src/libodfgen.hxx \
	src/OdfDocumentHandler.hxx \
	src/OdtGenerator.hxx \
	src/OdgGenerator.hxx \
))

$(eval $(call gb_ExternalPackage_use_external_project,libodfgen,libodfgen))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,lib/odfgen-0.0.lib,src/.libs/libodfgen-0.0.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libodfgen,lib/libodfgen-0.0.a,src/.libs/libodfgen-0.0.a))
endif


# vim: set noet sw=4 ts=4:
