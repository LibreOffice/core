# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/autotext))

extras_AUTOCORR_LANGS := \
	af-ZA:af-ZA \
	bg:bg-BG \
	ca:ca \
	cs:cs-CZ \
	da:da-DK \
	de:de-DE \
	en-AU:en-AU \
	en-GB:en-GB \
	en-US:en-US \
	en-ZA:en-ZA \
	es:es-ES \
	fa:fa-IR \
	fi:fi-FI \
	fr:fr-FR \
	ga-IE:ga-IE \
	hr:hr-HR \
	hu:hu-HU \
	is:is-IS \
	it:it-IT \
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
	zh-CN:zh-CN \
	zh-TW:zh-TW \

extras_AUTOCORR_XMLFILES := \
	af-ZA/acor/DocumentList.xml \
	af-ZA/acor/META-INF/manifest.xml \
	af-ZA/acor/SentenceExceptList.xml \
	af-ZA/acor/WordExceptList.xml \
	bg/acor/DocumentList.xml \
	bg/acor/META-INF/manifest.xml \
	bg/acor/SentenceExceptList.xml \
	bg/acor/WordExceptList.xml \
	ca/acor/BlockList.xml \
	ca/acor/DocumentList.xml \
	ca/acor/META-INF/manifest.xml \
	ca/acor/SentenceExceptList.xml \
	ca/acor/WordExceptList.xml \
	cs/acor/BlockList.xml \
	cs/acor/DocumentList.xml \
	cs/acor/META-INF/manifest.xml \
	da/acor/BlockList.xml \
	da/acor/DocumentList.xml \
	da/acor/META-INF/manifest.xml \
	da/acor/SentenceExceptList.xml \
	da/acor/WordExceptList.xml \
	de/acor/BlockList.xml \
	de/acor/DocumentList.xml \
	de/acor/META-INF/manifest.xml \
	de/acor/SentenceExceptList.xml \
	de/acor/WordExceptList.xml \
	en-AU/acor/BlockList.xml \
	en-AU/acor/DocumentList.xml \
	en-AU/acor/META-INF/manifest.xml \
	en-AU/acor/SentenceExceptList.xml \
	en-AU/acor/WordExceptList.xml \
	en-GB/acor/BlockList.xml \
	en-GB/acor/DocumentList.xml \
	en-GB/acor/META-INF/manifest.xml \
	en-GB/acor/SentenceExceptList.xml \
	en-GB/acor/WordExceptList.xml \
	en-US/acor/BlockList.xml \
	en-US/acor/DocumentList.xml \
	en-US/acor/META-INF/manifest.xml \
	en-US/acor/SentenceExceptList.xml \
	en-US/acor/WordExceptList.xml \
	en-ZA/acor/DocumentList.xml \
	en-ZA/acor/META-INF/manifest.xml \
	en-ZA/acor/SentenceExceptList.xml \
	en-ZA/acor/WordExceptList.xml \
	es/acor/BlockList.xml \
	es/acor/DocumentList.xml \
	es/acor/META-INF/manifest.xml \
	es/acor/SentenceExceptList.xml \
	es/acor/WordExceptList.xml \
	fa/acor/DocumentList.xml \
	fa/acor/META-INF/manifest.xml \
	fi/acor/BlockList.xml \
	fi/acor/DocumentList.xml \
	fi/acor/META-INF/manifest.xml \
	fi/acor/SentenceExceptList.xml \
	fi/acor/WordExceptList.xml \
	fr/acor/DocumentList.xml \
	fr/acor/META-INF/manifest.xml \
	fr/acor/SentenceExceptList.xml \
	fr/acor/WordExceptList.xml \
	ga-IE/acor/DocumentList.xml \
	ga-IE/acor/META-INF/manifest.xml \
	ga-IE/acor/SentenceExceptList.xml \
	ga-IE/acor/WordExceptList.xml \
	hr/acor/DocumentList.xml \
	hr/acor/META-INF/manifest.xml \
	hr/acor/SentenceExceptList.xml \
	hr/acor/WordExceptList.xml \
	hu/acor/BlockList.xml \
	hu/acor/DocumentList.xml \
	hu/acor/META-INF/manifest.xml \
	hu/acor/SentenceExceptList.xml \
	hu/acor/WordExceptList.xml \
	is/acor/META-INF/manifest.xml \
	is/acor/SentenceExceptList.xml \
	it/acor/BlockList.xml \
	it/acor/DocumentList.xml \
	it/acor/META-INF/manifest.xml \
	it/acor/SentenceExceptList.xml \
	it/acor/WordExceptList.xml \
	ja/acor/DocumentList.xml \
	ja/acor/META-INF/manifest.xml \
	ja/acor/SentenceExceptList.xml \
	ko/acor/DocumentList.xml \
	ko/acor/META-INF/manifest.xml \
	ko/acor/SentenceExceptList.xml \
	lb-LU/acor/DocumentList.xml \
	lb-LU/acor/META-INF/manifest.xml \
	lt/acor/BlockList.xml \
	lt/acor/DocumentList.xml \
	lt/acor/META-INF/manifest.xml \
	lt/acor/SentenceExceptList.xml \
	lt/acor/WordExceptList.xml \
	mn/acor/DocumentList.xml \
	mn/acor/META-INF/manifest.xml \
	mn/acor/SentenceExceptList.xml \
	mn/acor/WordExceptList.xml \
	nl-BE/acor/BlockList.xml \
	nl-BE/acor/DocumentList.xml \
	nl-BE/acor/META-INF/manifest.xml \
	nl-BE/acor/SentenceExceptList.xml \
	nl-BE/acor/WordExceptList.xml \
	nl/acor/BlockList.xml \
	nl/acor/DocumentList.xml \
	nl/acor/META-INF/manifest.xml \
	nl/acor/SentenceExceptList.xml \
	nl/acor/WordExceptList.xml \
	pl/acor/DocumentList.xml \
	pl/acor/META-INF/manifest.xml \
	pl/acor/SentenceExceptList.xml \
	pl/acor/WordExceptList.xml \
	pt-BR/acor/BlockList.xml \
	pt-BR/acor/DocumentList.xml \
	pt-BR/acor/META-INF/manifest.xml \
	pt-BR/acor/SentenceExceptList.xml \
	pt-BR/acor/WordExceptList.xml \
	pt/acor/BlockList.xml \
	pt/acor/DocumentList.xml \
	pt/acor/META-INF/manifest.xml \
	pt/acor/SentenceExceptList.xml \
	pt/acor/WordExceptList.xml \
	ro/acor/BlockList.xml \
	ro/acor/DocumentList.xml \
	ro/acor/META-INF/manifest.xml \
	ro/acor/SentenceExceptList.xml \
	ro/acor/WordExceptList.xml \
	ru/acor/BlockList.xml \
	ru/acor/DocumentList.xml \
	ru/acor/META-INF/manifest.xml \
	ru/acor/SentenceExceptList.xml \
	ru/acor/WordExceptList.xml \
	sr-Latn-ME/acor/DocumentList.xml \
	sr-Latn-ME/acor/META-INF/manifest.xml \
	sr-Latn-ME/acor/SentenceExceptList.xml \
	sr-Latn-ME/acor/WordExceptList.xml \
	sr-Latn-RS/acor/DocumentList.xml \
	sr-Latn-RS/acor/META-INF/manifest.xml \
	sr-Latn-RS/acor/SentenceExceptList.xml \
	sr-Latn-RS/acor/WordExceptList.xml \
	sr-Latn-CS/acor/DocumentList.xml \
	sr-Latn-CS/acor/META-INF/manifest.xml \
	sr-Latn-CS/acor/SentenceExceptList.xml \
	sr-Latn-CS/acor/WordExceptList.xml \
	sk/acor/DocumentList.xml \
	sk/acor/META-INF/manifest.xml \
	sk/acor/SentenceExceptList.xml \
	sk/acor/WordExceptList.xml \
	sl/acor/DocumentList.xml \
	sl/acor/META-INF/manifest.xml \
	sl/acor/SentenceExceptList.xml \
	sl/acor/WordExceptList.xml \
	sr-ME/acor/DocumentList.xml \
	sr-ME/acor/META-INF/manifest.xml \
	sr-ME/acor/SentenceExceptList.xml \
	sr-ME/acor/WordExceptList.xml \
	sr-RS/acor/DocumentList.xml \
	sr-RS/acor/META-INF/manifest.xml \
	sr-RS/acor/SentenceExceptList.xml \
	sr-RS/acor/WordExceptList.xml \
	sr-CS/acor/DocumentList.xml \
	sr-CS/acor/META-INF/manifest.xml \
	sr-CS/acor/SentenceExceptList.xml \
	sr-CS/acor/WordExceptList.xml \
	sv/acor/BlockList.xml \
	sv/acor/DocumentList.xml \
	sv/acor/META-INF/manifest.xml \
	sv/acor/SentenceExceptList.xml \
	sv/acor/WordExceptList.xml \
	tr/acor/DocumentList.xml \
	tr/acor/META-INF/manifest.xml \
	tr/acor/SentenceExceptList.xml \
	tr/acor/WordExceptList.xml \
	vi/acor/DocumentList.xml \
	vi/acor/META-INF/manifest.xml \
	vi/acor/WordExceptList.xml \
	zh-CN/acor/DocumentList.xml \
	zh-CN/acor/META-INF/manifest.xml \
	zh-CN/acor/SentenceExceptList.xml \
	zh-TW/acor/DocumentList.xml \
	zh-TW/acor/META-INF/manifest.xml \
	zh-TW/acor/SentenceExceptList.xml \


extras_AUTOCORR_MIMETYPEFILES := \
	af-ZA/acor/mimetype \
	bg/acor/mimetype \
	ca/acor/mimetype \
	cs/acor/mimetype \
	da/acor/mimetype \
	de/acor/mimetype \
	en-AU/acor/mimetype \
	en-GB/acor/mimetype \
	en-US/acor/mimetype \
	en-ZA/acor/mimetype \
	es/acor/mimetype \
	fa/acor/mimetype \
	fi/acor/mimetype \
	fr/acor/mimetype \
	ga-IE/acor/mimetype \
	hu/acor/mimetype \
	is/acor/mimetype \
	it/acor/mimetype \
	lb-LU/acor/mimetype \
	lt/acor/mimetype \
	mn/acor/mimetype \
	nl-BE/acor/mimetype \
	nl/acor/mimetype \
	pl/acor/mimetype \
	pt-BR/acor/mimetype \
	pt/acor/mimetype \
	ro/acor/mimetype \
	ru/acor/mimetype \
	sk/acor/mimetype \
	sl/acor/mimetype \
	sr-CS/acor/mimetype \
	sr-Latn-CS/acor/mimetype \
	sr-Latn-ME/acor/mimetype \
	sr-Latn-RS/acor/mimetype \
	sr-ME/acor/mimetype \
	sr-RS/acor/mimetype \
	sv/acor/mimetype \
	tr/acor/mimetype \
	vi/acor/mimetype \


ifneq ($(sort $(foreach file,$(extras_AUTOCORR_XMLFILES),$(firstword $(subst /, ,$(file))))),$(sort $(foreach lang,$(extras_AUTOCORR_LANGS),$(firstword $(subst :, ,$(lang))))))
$(call gb_Output_error,defined autocorrection languages do not match existing directories)
endif

$(call gb_CustomTarget_get_target,extras/source/autotext) : \
	$(foreach lang,$(extras_AUTOCORR_LANGS),$(call gb_CustomTarget_get_workdir,extras/source/autotext)/acor_$(lastword $(subst :, ,$(lang))).dat)

$(call gb_CustomTarget_get_workdir,extras/source/autotext)/%/mimetype : $(SRCDIR)/extras/source/autotext/lang/%/mimetype
	$(call gb_Output_announce,$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/autotext)/%.xml : $(SRCDIR)/extras/source/autotext/lang/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/autotext)/%.dat :
	$(call gb_Output_announce,$*.dat,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_AUTOCORR_DIR) && \
		zip -qrX --filesync --must-match $@ $(EXTRAS_AUTOCORR_FILES) \
	)

define extras_Autocorr_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/lang/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Autocorr_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_AUTOCORR_MIMETYPEFILES) $(extras_AUTOCORR_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_AUTOCORR_FILES := $(foreach file,$(filter $(3)/%,$(extras_AUTOCORR_MIMETYPEFILES) $(extras_AUTOCORR_XMLFILES)),$(subst $(3)/acor/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_AUTOCORR_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)/acor

endef

$(eval $(foreach file,$(extras_AUTOCORR_MIMETYPEFILES) $(extras_AUTOCORR_XMLFILES),\
	$(call extras_Autocorr_make_file_deps,extras/source/autotext,$(file)) \
))

$(eval $(foreach lang,$(extras_AUTOCORR_LANGS),\
	$(call extras_Autocorr_make_zip_deps,extras/source/autotext,acor_$(lastword $(subst :, ,$(lang))).dat,$(firstword $(subst :, ,$(lang)))) \
))

# vim: set noet sw=4 ts=4:
