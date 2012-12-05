# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallsystem,$(SRCDIR)/extras/source/gallery/gallery_system))

$(eval $(call gb_Package_set_outdir,extras_gallsystem,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallsystem,share/gallery,\
	sg1.sdg \
	sg1.sdv \
	sg1.thm \
	sg3.sdg \
	sg3.sdv \
	sg3.thm \
	sg4.sdg \
	sg4.sdv \
	sg4.thm \
	sg9.sdg \
	sg9.sdv \
	sg9.thm \
	sg10.sdg \
	sg10.sdv \
	sg10.thm \
	sg11.sdg \
	sg11.sdv \
	sg11.thm \
	sg12.sdg \
	sg12.sdv \
	sg12.thm \
	sg13.sdg \
	sg13.sdv \
	sg13.thm \
	sg14.sdg \
	sg14.sdv \
	sg14.thm \
	sg15.sdg \
	sg15.sdv \
	sg15.thm \
	sg16.sdg \
	sg16.sdv \
	sg16.thm \
	sg17.sdg \
	sg17.sdv \
	sg17.thm \
	sg18.sdg \
	sg18.sdv \
	sg18.thm \
	sg19.sdg \
	sg19.sdv \
	sg19.thm \
	sg24.sdg \
	sg24.sdv \
	sg24.thm \
	sg36.sdg \
	sg36.sdv \
	sg36.thm \
))

# vim: set noet sw=4 ts=4:
