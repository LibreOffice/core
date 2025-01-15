# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extensions_quicklookpreviewappex,$(BUILDDIR)/extensions/source/macosx/quicklookpreview/appex))

$(eval $(call gb_Package_add_files,extensions_quicklookpreviewappex,PlugIns/QuickLookPreview.appex/Contents,\
    Info.plist \
))

$(eval $(call gb_Package_add_files,extensions_quicklookpreviewappex,PlugIns/QuickLookPreview.appex/Contents/Resources,\
))

# vim: set noet sw=4 ts=4:
