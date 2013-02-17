# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,xpdf,xpdf))

$(eval $(call gb_ExternalPackage_use_external_project,xpdf,xpdf))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/fofi.lib,fofi/fofi.lib))
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/Goo.lib,goo/Goo.lib))
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/xpdf.lib,xpdf/xpdf.lib))
else
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/libfofi.a,fofi/libfofi.a))
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/libGoo.a,goo/libGoo.a))
$(eval $(call gb_ExternalPackage_add_file,xpdf,lib/libxpdf.a,xpdf/libxpdf.a))
endif

# vim: set noet sw=4 ts=4:
