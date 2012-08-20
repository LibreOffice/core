# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_fonts,$(SRCDIR)/extras))

$(eval $(call gb_Package_add_file,extras_fonts,pck/opens___.ttf,source/truetype/symbol/opens___.ttf))
$(eval $(call gb_Package_add_file,extras_fonts,pck/openoffice/fonts.dir,unx/source/fonts/fonts.dir))

# vim: set noet sw=4 ts=4:
