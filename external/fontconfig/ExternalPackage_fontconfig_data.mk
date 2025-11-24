# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fontconfig_data,fontconfig))

$(eval $(call gb_ExternalPackage_use_external_project,fontconfig_data,fontconfig))

ifeq ($(COM),MSC)
FONTCONFIG_DATA_FOLDER=$(LIBO_BIN_FOLDER)/fonts
else
FONTCONFIG_DATA_FOLDER=$(LIBO_SHARE_FOLDER)/fontconfig
endif

$(eval $(call gb_ExternalPackage_add_files,fontconfig_data,$(FONTCONFIG_DATA_FOLDER),\
    fonts.conf \
))

# The list below is created with
# find workdir/UnpackedTarball/fontconfig/conf.d/ -name "*.conf" | LC_COLLATE=C sort | sed -e 's/^/    /' -e 's/$/ \\/' -e 's/workdir\/UnpackedTarball\/fontconfig\///'
$(eval $(call gb_ExternalPackage_add_unpacked_files,fontconfig_data,$(FONTCONFIG_DATA_FOLDER)/conf.d,\
    conf.d/10-hinting-slight.conf \
    conf.d/10-scale-bitmap-fonts.conf \
    conf.d/10-sub-pixel-none.conf \
    conf.d/10-yes-antialias.conf \
    conf.d/11-lcdfilter-default.conf \
    conf.d/20-unhint-small-vera.conf \
    conf.d/30-metric-aliases.conf \
    conf.d/35-lang-normalize.conf \
    conf.d/40-nonlatin.conf \
    conf.d/45-generic.conf \
    conf.d/45-latin.conf \
    conf.d/48-spacing.conf \
    conf.d/49-sansserif.conf \
    conf.d/60-generic.conf \
    conf.d/60-latin.conf \
    conf.d/65-fonts-persian.conf \
    conf.d/65-khmer.conf \
    conf.d/65-nonlatin.conf \
    conf.d/69-unifont.conf \
    conf.d/70-no-bitmaps-except-emoji.conf \
    conf.d/80-delicious.conf \
    conf.d/90-synthetic.conf \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
