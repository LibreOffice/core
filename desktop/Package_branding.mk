# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_branding,$(SRCDIR)/icon-themes/galaxy))

$(eval $(call gb_Package_set_outdir,desktop_branding,$(INSTDIR)))

$(eval $(call gb_Package_add_files,desktop_branding,$(LIBO_ETC_FOLDER),\
    $(foreach image,$(filter $(BRAND_INTRO_IMAGES),$(DEFAULT_BRAND_IMAGES)),\
		$(if $(filter intro.png,$(image)),\
			$(if $(filter TRUE,$(ENABLE_RELEASE_BUILD)),brand,brand_dev)/$(image),\
			brand/$(image) \
		) \
	) \
))

$(eval $(call gb_Package_add_files,desktop_branding,$(LIBO_ETC_FOLDER)/shell,\
    $(addprefix brand/shell/,$(filter-out $(BRAND_INTRO_IMAGES),$(DEFAULT_BRAND_IMAGES))) \
))

# vim: set noet sw=4 ts=4:
