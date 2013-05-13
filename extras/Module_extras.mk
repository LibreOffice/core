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
	Package_gallarrows \
	Package_gallbullets \
	Package_gallcomputers \
	Package_galldiagrams \
	Package_galleducation \
	Package_gallenvironment \
	Package_gallfinance \
	Package_gallhtmlexpo \
	Package_gallmytheme \
	Package_gallpeople \
	Package_gallroot \
	Package_gallsounds \
	Package_gallsymbols \
	Package_gallsystem \
	Package_galltransport \
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

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,extras,\
	Gallery_sound \
	Gallery_txtshapes \
))
endif

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
	ExternalPackage_accessoriesgalleryelementsbullets2 \
	ExternalPackage_accessoriesgallerynonfree \
	ExternalPackage_accessoriesgalleryphotosbuildings \
	ExternalPackage_accessoriesgalleryphotoscelebration \
	ExternalPackage_accessoriesgalleryphotoscities \
	ExternalPackage_accessoriesgalleryphotosfauna \
	ExternalPackage_accessoriesgalleryphotosflowers \
	ExternalPackage_accessoriesgalleryphotosfoodsanddrinks \
	ExternalPackage_accessoriesgalleryphotoshumans \
	ExternalPackage_accessoriesgalleryphotoslandscapes \
	ExternalPackage_accessoriesgalleryphotosobjects \
	ExternalPackage_accessoriesgalleryphotosplants \
	ExternalPackage_accessoriesgalleryphotosspace \
	ExternalPackage_accessoriesgalleryphotosstatues \
	ExternalPackage_accessoriesgalleryphotostravel \
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
	ExternalPackage_accessoriestemplatescommon \
	ExternalPackage_accessoriestemplatescommoneducate \
	ExternalPackage_accessoriestemplatescommonfinance \
	ExternalPackage_accessoriestemplatescommonforms \
	ExternalPackage_accessoriestemplatescommonlabels \
	ExternalPackage_accessoriestemplatescommonlayout \
	ExternalPackage_accessoriestemplatescommonmisc \
	ExternalPackage_accessoriestemplatescommonofficorr \
	ExternalPackage_accessoriestemplatescommonoffimisc \
	ExternalPackage_accessoriestemplatescommonpersonal \
	ExternalPackage_accessoriestemplatescommonpresent \
	UnpackedTarball_templates_pack \
))
endif

# vim: set noet sw=4 ts=4:
