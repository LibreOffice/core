# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_branding_custom,$(CUSTOM_BRAND_DIR)))

$(eval $(call gb_Package_set_outdir,desktop_branding_custom,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files,desktop_branding_custom,$(gb_PROGRAMDIRNAME),\
    $(filter $(BRAND_INTRO_IMAGES),$(CUSTOM_BRAND_IMAGES)) \
))

$(eval $(call gb_Package_add_files,desktop_branding_custom,$(gb_PROGRAMDIRNAME)/shell,\
    $(filter-out $(BRAND_INTRO_IMAGES),$(CUSTOM_BRAND_IMAGES)) \
))

# vim: set noet sw=4 ts=4:
