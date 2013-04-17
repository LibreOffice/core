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
	AllLangPackage_autotextshare \
	CustomTarget_autocorr \
	CustomTarget_glade \
	Package_autocorr \
	Package_autotextuser \
	Package_cfgsrvbitmapunx \
	Package_cfgsrvnolang \
	Package_cfgusr \
	Package_database \
	Package_databasebiblio \
	Package_fonts \
	Package_gallbullets \
	Package_gallhtmlexpo \
	Package_gallroot \
	Package_gallsound \
	Package_gallsounds \
	Package_gallsystem \
	Package_gallwwwback \
	Package_gallwwwgraf \
	Package_glade \
	Package_labels \
	Package_newfiles \
	Package_palettes \
	Package_tpllayoutimpr \
	Package_tplwizagenda \
	Package_tplwizbitmap \
	Package_tplwizdesktop \
	Package_tplwizfax \
	Package_tplwizletter \
	Package_tplwizreport \
	Package_tplwizstyles \
	Package_wordbook \
))

ifeq ($(WITH_EXTRA_EXTENSIONS),YES)
$(eval $(call gb_Module_add_targets,extras,\
	Package_extensions \
))
endif

ifeq ($(WITH_EXTRA_FONT),YES)
$(eval $(call gb_Module_add_targets,extras,\
	ExternalPackage_extra_fonts \
	UnpackedTarball_fonts_pack \
))
endif

ifeq ($(WITH_EXTRA_GALLERY),YES)
$(eval $(call gb_Module_add_targets,extras,\
	UnpackedTarball_gallery_pack \
	ExternalPackage_accessoriesgallery \
	ExternalPackage_accessoriesgallerydrawspeople \
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
	AllLangPackage_accessoriessamples \
	AllLangPackage_accessoriessamplesadvertisement \
	AllLangPackage_accessoriessamplesdocumentation \
	AllLangPackage_accessoriessamplesnonfree \
	AllLangPackage_accessoriessamplesnonfreeadvertisement \
	AllLangPackage_accessoriessamplesnonfreedocumentation \
	UnpackedTarball_samples_pack \
))
endif

ifeq ($(WITH_EXTRA_TEMPLATE),YES)
$(eval $(call gb_Module_add_targets,extras,\
	AllLangPackage_accessoriestemplates \
	AllLangPackage_accessoriestemplateseducate \
	AllLangPackage_accessoriestemplatesfinance \
	AllLangPackage_accessoriestemplatesforms \
	AllLangPackage_accessoriestemplateslabels \
	AllLangPackage_accessoriestemplateslayout \
	AllLangPackage_accessoriestemplatesmisc \
	AllLangPackage_accessoriestemplatesnonfree \
	AllLangPackage_accessoriestemplatesnonfreeeducate \
	AllLangPackage_accessoriestemplatesnonfreefinance \
	AllLangPackage_accessoriestemplatesnonfreeforms \
	AllLangPackage_accessoriestemplatesnonfreelabels \
	AllLangPackage_accessoriestemplatesnonfreelayout \
	AllLangPackage_accessoriestemplatesnonfreemisc \
	AllLangPackage_accessoriestemplatesnonfreeofficorr \
	AllLangPackage_accessoriestemplatesnonfreeoffimisc \
	AllLangPackage_accessoriestemplatesnonfreepersonal \
	AllLangPackage_accessoriestemplatesnonfreepresent \
	AllLangPackage_accessoriestemplatesofficorr \
	AllLangPackage_accessoriestemplatesoffimisc \
	AllLangPackage_accessoriestemplatespersonal \
	AllLangPackage_accessoriestemplatespresent \
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
