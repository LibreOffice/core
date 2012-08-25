# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,extras))

$(eval $(call gb_Module_add_targets,extras,\
	AllLangZip_autotextshare \
	CustomTarget_autocorr \
	Package_fonts \
	Package_newfiles \
	Zip_autocorr \
	Zip_autotextuser \
	Zip_cfgsrvbitmapunx \
	Zip_cfgsrvnolang \
	Zip_cfgusr \
	Zip_database \
	Zip_databasebiblio \
	Zip_gallbullets \
	Zip_gallhtmlexpo \
	Zip_gallroot \
	Zip_gallrulers \
	Zip_gallsound \
	Zip_gallsounds \
	Zip_gallsystem \
	Zip_gallwwwback \
	Zip_gallwwwgraf \
	Zip_palettes \
	Zip_tpllayoutimpr \
	Zip_tplwizagenda \
	Zip_tplwizbitmap \
	Zip_tplwizdesktop \
	Zip_tplwizfax \
	Zip_tplwizletter \
	Zip_tplwizreport \
	Zip_tplwizstyles \
	Zip_wordbook \
))

ifeq ($(WITH_EXTRA_EXTENSIONS),YES)
$(eval $(call gb_Module_add_targets,extras,\
	Package_extensions \
))
endif

ifeq ($(WITH_EXTRA_FONT),YES)
$(eval $(call gb_Module_add_targets,extras,\
	Package_extra_fonts \
	UnpackedTarball_fonts_pack \
))
endif

ifeq ($(WITH_EXTRA_GALLERY),YES)
$(eval $(call gb_Module_add_targets,extras,\
	UnpackedTarball_gallery_pack \
	Zip_accessoriesgallery \
	Zip_accessoriesgallerydrawspeople \
	Zip_accessoriesgalleryelementsbullets2 \
	Zip_accessoriesgallerynonfree \
	Zip_accessoriesgalleryphotosbuildings \
	Zip_accessoriesgalleryphotoscelebration \
	Zip_accessoriesgalleryphotoscities \
	Zip_accessoriesgalleryphotosfauna \
	Zip_accessoriesgalleryphotosflowers \
	Zip_accessoriesgalleryphotosfoodsanddrinks \
	Zip_accessoriesgalleryphotoshumans \
	Zip_accessoriesgalleryphotoslandscapes \
	Zip_accessoriesgalleryphotosobjects \
	Zip_accessoriesgalleryphotosplants \
	Zip_accessoriesgalleryphotosspace \
	Zip_accessoriesgalleryphotosstatues \
	Zip_accessoriesgalleryphotostravel \
))
endif

ifeq ($(WITH_EXTRA_SAMPLE),YES)
$(eval $(call gb_Module_add_targets,extras,\
	AllLangZip_accessoriessamples \
	AllLangZip_accessoriessamplesadvertisement \
	AllLangZip_accessoriessamplesdocumentation \
	AllLangZip_accessoriessamplesnonfree \
	AllLangZip_accessoriessamplesnonfreeadvertisement \
	AllLangZip_accessoriessamplesnonfreedocumentation \
	UnpackedTarball_samples_pack \
))
endif

ifeq ($(WITH_EXTRA_TEMPLATE),YES)
$(eval $(call gb_Module_add_targets,extras,\
	AllLangZip_accessoriestemplates \
	AllLangZip_accessoriestemplateseducate \
	AllLangZip_accessoriestemplatesfinance \
	AllLangZip_accessoriestemplatesforms \
	AllLangZip_accessoriestemplateslabels \
	AllLangZip_accessoriestemplateslayout \
	AllLangZip_accessoriestemplatesmisc \
	AllLangZip_accessoriestemplatesnonfree \
	AllLangZip_accessoriestemplatesnonfreeeducate \
	AllLangZip_accessoriestemplatesnonfreefinance \
	AllLangZip_accessoriestemplatesnonfreeforms \
	AllLangZip_accessoriestemplatesnonfreelabels \
	AllLangZip_accessoriestemplatesnonfreelayout \
	AllLangZip_accessoriestemplatesnonfreemisc \
	AllLangZip_accessoriestemplatesnonfreeofficorr \
	AllLangZip_accessoriestemplatesnonfreeoffimisc \
	AllLangZip_accessoriestemplatesnonfreepersonal \
	AllLangZip_accessoriestemplatesnonfreepresent \
	AllLangZip_accessoriestemplatesofficorr \
	AllLangZip_accessoriestemplatesoffimisc \
	AllLangZip_accessoriestemplatespersonal \
	AllLangZip_accessoriestemplatespresent \
	UnpackedTarball_templates_pack \
	Zip_accessoriestemplatescommon \
	Zip_accessoriestemplatescommoneducate \
	Zip_accessoriestemplatescommonfinance \
	Zip_accessoriestemplatescommonforms \
	Zip_accessoriestemplatescommonlabels \
	Zip_accessoriestemplatescommonlayout \
	Zip_accessoriestemplatescommonmisc \
	Zip_accessoriestemplatescommonofficorr \
	Zip_accessoriestemplatescommonoffimisc \
	Zip_accessoriestemplatescommonpersonal \
	Zip_accessoriestemplatescommonpresent \
))
endif

# vim: set noet sw=4 ts=4:
