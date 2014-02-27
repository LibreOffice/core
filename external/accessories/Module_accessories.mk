# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,accessories))

ifneq ($(WITH_EXTRA_FONT),)
$(eval $(call gb_Module_add_targets,accessories,\
	ExternalPackage_extra_fonts \
	UnpackedTarball_fonts_pack \
))
endif

ifneq ($(WITH_EXTRA_GALLERY),)
$(eval $(call gb_Module_add_targets,accessories,\
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

ifneq ($(WITH_EXTRA_SAMPLE),)
$(eval $(call gb_Module_add_l10n_targets,accessories,\
	AllLangPackage_accessoriessamples \
	AllLangPackage_accessoriessamplesadvertisement \
	AllLangPackage_accessoriessamplesdocumentation \
	AllLangPackage_accessoriessamplesnonfree \
	AllLangPackage_accessoriessamplesnonfreeadvertisement \
	AllLangPackage_accessoriessamplesnonfreedocumentation \
))

$(eval $(call gb_Module_add_targets,accessories,\
	UnpackedTarball_samples_pack \
))
endif

ifneq ($(WITH_EXTRA_TEMPLATE),)
$(eval $(call gb_Module_add_l10n_targets,accessories,\
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
))

$(eval $(call gb_Module_add_targets,accessories,\
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
