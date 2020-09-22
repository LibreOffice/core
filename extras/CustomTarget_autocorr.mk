# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/autocorr))

extras_AUTOCORR_LANGS := \
	af-ZA:af-ZA \
	bg:bg-BG \
	ca:ca-ES \
	cs:cs-CZ \
	da:da-DK \
	de:de \
	dsb:dsb \
	el:el-GR \
	en-AU:en-AU \
	en-GB:en-GB \
	en-US:en-US \
	en-ZA:en-ZA \
	es:es \
	fa:fa-IR \
	fi:fi-FI \
	fr:fr \
	ga-IE:ga-IE \
	hr:hr-HR \
	hsb:hsb \
	hu:hu-HU \
	is:is-IS \
	it:it \
	ja:ja-JP \
	ko:ko-KR \
	lb-LU:lb-LU \
	lt:lt-LT \
	mn:mn-MN \
	nl:nl-NL \
	nl-BE:nl-BE \
	pl:pl-PL \
	pt:pt-PT \
	pt-BR:pt-BR \
	ro:ro-RO \
	ru:ru-RU \
	sk:sk-SK \
	sl:sl-SI \
	sr-CS:sr-CS \
	sr-Latn-CS:sr-Latn-CS \
	sr-Latn-ME:sr-Latn-ME \
	sr-Latn-RS:sr-Latn-RS \
	sr-ME:sr-ME \
	sr-RS:sr-RS \
	sv:sv-SE \
	tr:tr-TR \
	vi:vi-VN \
	vro:vro-EE \
	zh-CN:zh-CN \
	zh-TW:zh-TW \

extras_AUTOCORR_XMLFILES := \
	af-ZA/DocumentList.xml \
	af-ZA/META-INF/manifest.xml \
	af-ZA/SentenceExceptList.xml \
	af-ZA/WordExceptList.xml \
	bg/DocumentList.xml \
	bg/META-INF/manifest.xml \
	bg/SentenceExceptList.xml \
	bg/WordExceptList.xml \
	ca/DocumentList.xml \
	ca/META-INF/manifest.xml \
	ca/SentenceExceptList.xml \
	ca/WordExceptList.xml \
	cs/DocumentList.xml \
	cs/META-INF/manifest.xml \
	da/DocumentList.xml \
	da/META-INF/manifest.xml \
	da/SentenceExceptList.xml \
	da/WordExceptList.xml \
	de/DocumentList.xml \
	de/META-INF/manifest.xml \
	de/SentenceExceptList.xml \
	de/WordExceptList.xml \
	dsb/DocumentList.xml \
	dsb/META-INF/manifest.xml \
	el/DocumentList.xml \
	el/META-INF/manifest.xml \
	el/SentenceExceptList.xml \
	el/WordExceptList.xml \
	en-AU/DocumentList.xml \
	en-AU/META-INF/manifest.xml \
	en-AU/SentenceExceptList.xml \
	en-AU/WordExceptList.xml \
	en-GB/DocumentList.xml \
	en-GB/META-INF/manifest.xml \
	en-GB/SentenceExceptList.xml \
	en-GB/WordExceptList.xml \
	en-US/DocumentList.xml \
	en-US/META-INF/manifest.xml \
	en-US/SentenceExceptList.xml \
	en-US/WordExceptList.xml \
	en-ZA/DocumentList.xml \
	en-ZA/META-INF/manifest.xml \
	en-ZA/SentenceExceptList.xml \
	en-ZA/WordExceptList.xml \
	es/DocumentList.xml \
	es/META-INF/manifest.xml \
	es/SentenceExceptList.xml \
	es/WordExceptList.xml \
	fa/DocumentList.xml \
	fa/META-INF/manifest.xml \
	fi/DocumentList.xml \
	fi/META-INF/manifest.xml \
	fi/SentenceExceptList.xml \
	fi/WordExceptList.xml \
	fr/DocumentList.xml \
	fr/META-INF/manifest.xml \
	fr/SentenceExceptList.xml \
	fr/WordExceptList.xml \
	ga-IE/DocumentList.xml \
	ga-IE/META-INF/manifest.xml \
	ga-IE/SentenceExceptList.xml \
	ga-IE/WordExceptList.xml \
	hr/DocumentList.xml \
	hr/META-INF/manifest.xml \
	hr/SentenceExceptList.xml \
	hr/WordExceptList.xml \
	hsb/DocumentList.xml \
	hsb/META-INF/manifest.xml \
	hu/DocumentList.xml \
	hu/META-INF/manifest.xml \
	hu/SentenceExceptList.xml \
	hu/WordExceptList.xml \
	is/META-INF/manifest.xml \
	is/SentenceExceptList.xml \
	it/DocumentList.xml \
	it/META-INF/manifest.xml \
	it/SentenceExceptList.xml \
	it/WordExceptList.xml \
	ja/DocumentList.xml \
	ja/META-INF/manifest.xml \
	ja/SentenceExceptList.xml \
	ko/DocumentList.xml \
	ko/META-INF/manifest.xml \
	ko/SentenceExceptList.xml \
	lb-LU/DocumentList.xml \
	lb-LU/META-INF/manifest.xml \
	lt/DocumentList.xml \
	lt/META-INF/manifest.xml \
	lt/SentenceExceptList.xml \
	lt/WordExceptList.xml \
	mn/DocumentList.xml \
	mn/META-INF/manifest.xml \
	mn/SentenceExceptList.xml \
	mn/WordExceptList.xml \
	nl-BE/DocumentList.xml \
	nl-BE/META-INF/manifest.xml \
	nl-BE/SentenceExceptList.xml \
	nl-BE/WordExceptList.xml \
	nl/DocumentList.xml \
	nl/META-INF/manifest.xml \
	nl/SentenceExceptList.xml \
	nl/WordExceptList.xml \
	pl/DocumentList.xml \
	pl/META-INF/manifest.xml \
	pl/SentenceExceptList.xml \
	pl/WordExceptList.xml \
	pt-BR/DocumentList.xml \
	pt-BR/META-INF/manifest.xml \
	pt-BR/SentenceExceptList.xml \
	pt-BR/WordExceptList.xml \
	pt/DocumentList.xml \
	pt/META-INF/manifest.xml \
	pt/SentenceExceptList.xml \
	pt/WordExceptList.xml \
	ro/DocumentList.xml \
	ro/META-INF/manifest.xml \
	ro/SentenceExceptList.xml \
	ro/WordExceptList.xml \
	ru/DocumentList.xml \
	ru/META-INF/manifest.xml \
	ru/SentenceExceptList.xml \
	ru/WordExceptList.xml \
	sk/DocumentList.xml \
	sk/META-INF/manifest.xml \
	sk/SentenceExceptList.xml \
	sk/WordExceptList.xml \
	sl/DocumentList.xml \
	sl/META-INF/manifest.xml \
	sl/SentenceExceptList.xml \
	sl/WordExceptList.xml \
	sr-CS/DocumentList.xml \
	sr-CS/META-INF/manifest.xml \
	sr-CS/SentenceExceptList.xml \
	sr-CS/WordExceptList.xml \
	sr-Latn-CS/DocumentList.xml \
	sr-Latn-CS/META-INF/manifest.xml \
	sr-Latn-CS/SentenceExceptList.xml \
	sr-Latn-CS/WordExceptList.xml \
	sr-Latn-ME/DocumentList.xml \
	sr-Latn-ME/META-INF/manifest.xml \
	sr-Latn-ME/SentenceExceptList.xml \
	sr-Latn-ME/WordExceptList.xml \
	sr-Latn-RS/DocumentList.xml \
	sr-Latn-RS/META-INF/manifest.xml \
	sr-Latn-RS/SentenceExceptList.xml \
	sr-Latn-RS/WordExceptList.xml \
	sr-ME/DocumentList.xml \
	sr-ME/META-INF/manifest.xml \
	sr-ME/SentenceExceptList.xml \
	sr-ME/WordExceptList.xml \
	sr-RS/DocumentList.xml \
	sr-RS/META-INF/manifest.xml \
	sr-RS/SentenceExceptList.xml \
	sr-RS/WordExceptList.xml \
	sv/DocumentList.xml \
	sv/META-INF/manifest.xml \
	sv/SentenceExceptList.xml \
	sv/WordExceptList.xml \
	tr/DocumentList.xml \
	tr/META-INF/manifest.xml \
	tr/SentenceExceptList.xml \
	tr/WordExceptList.xml \
	vi/DocumentList.xml \
	vi/META-INF/manifest.xml \
	vi/WordExceptList.xml \
	vro/DocumentList.xml \
	vro/META-INF/manifest.xml \
	zh-CN/DocumentList.xml \
	zh-CN/META-INF/manifest.xml \
	zh-CN/SentenceExceptList.xml \
	zh-TW/DocumentList.xml \
	zh-TW/META-INF/manifest.xml \
	zh-TW/SentenceExceptList.xml \


extras_AUTOCORR_MIMETYPEFILES := $(foreach lang,$(extras_AUTOCORR_LANGS),$(firstword $(subst :, ,$(lang)))/mimetype)


$(call gb_CustomTarget_get_target,extras/source/autocorr) : \
	$(foreach lang,$(extras_AUTOCORR_LANGS),$(call gb_CustomTarget_get_workdir,extras/source/autocorr)/acor_$(lastword $(subst :, ,$(lang))).dat)

$(call gb_CustomTarget_get_workdir,extras/source/autocorr)/%/mimetype : $(SRCDIR)/extras/source/autocorr/lang/%/mimetype
	$(call gb_Output_announce,autocorr/$*/mimetype,$(true),CPY,1)
	$(call gb_Trace_StartRange,autocorr/$*/mimetype,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,autocorr/$*/mimetype,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/autocorr)/%.xml : $(SRCDIR)/extras/source/autocorr/lang/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,autocorr/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,autocorr/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,autocorr/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/autocorr)/%.dat :
	$(call gb_Output_announce,autocorr/$*.dat,$(true),ZIP,2)
	$(call gb_Trace_StartRange,autocorr/$*.dat,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_AUTOCORR_DIR) && \
		zip -q0X --filesync --must-match $@ $(EXTRAS_AUTOCORR_MIMEFILES_FILTER) && \
		zip -qrX --must-match $@ $(EXTRAS_AUTOCORR_XMLFILES_FILTER) \
	)
	$(call gb_Trace_EndRange,autocorr/$*.dat,ZIP)

define extras_Autocorr_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/lang/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Autocorr_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_AUTOCORR_MIMETYPEFILES) $(extras_AUTOCORR_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_AUTOCORR_MIMEFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_AUTOCORR_MIMETYPEFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_AUTOCORR_XMLFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_AUTOCORR_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_AUTOCORR_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_AUTOCORR_MIMETYPEFILES) $(extras_AUTOCORR_XMLFILES),\
	$(call extras_Autocorr_make_file_deps,extras/source/autocorr,$(file)) \
))

$(eval $(foreach lang,$(extras_AUTOCORR_LANGS),\
	$(call extras_Autocorr_make_zip_deps,extras/source/autocorr,acor_$(lastword $(subst :, ,$(lang))).dat,$(firstword $(subst :, ,$(lang)))) \
))

# vim: set noet sw=4 ts=4:
