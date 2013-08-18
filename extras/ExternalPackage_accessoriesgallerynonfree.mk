# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgallerynonfree,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgallerynonfree,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgallerynonfree,$(LIBO_SHARE_FOLDER)/gallery,\
	accessories-nonfree/sg600.sdg \
	accessories-nonfree/sg600.sdv \
	accessories-nonfree/sg600.thm \
	accessories-nonfree/sg601.sdg \
	accessories-nonfree/sg601.sdv \
	accessories-nonfree/sg601.thm \
	accessories-nonfree/sg602.sdg \
	accessories-nonfree/sg602.sdv \
	accessories-nonfree/sg602.thm \
	accessories-nonfree/sg603.sdg \
	accessories-nonfree/sg603.sdv \
	accessories-nonfree/sg603.thm \
))

# vim: set noet sw=4 ts=4:
