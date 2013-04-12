# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,liborcus,orcus))

$(eval $(call gb_ExternalPackage_use_external_project,liborcus,liborcus))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,liborcus,lib/orcus.lib,vsprojects/liborcus-static/Release/liborcus-static.lib))
$(eval $(call gb_ExternalPackage_add_file,liborcus,lib/orcus-parser.lib,vsprojects/liborcus-parser-static/Release/liborcus-parser-static.lib))
else
$(eval $(call gb_ExternalPackage_add_file,liborcus,lib/liborcus.a,src/liborcus/.libs/liborcus-0.6.a))
$(eval $(call gb_ExternalPackage_add_file,liborcus,lib/liborcus-parser.a,src/parser/.libs/liborcus-parser-0.6.a))
endif

# vim: set noet sw=4 ts=4:
