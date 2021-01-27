# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/presnt))

extras_TEMPLATES_PRESENTATIONS := \
	Beehive \
	Blue_Curve \
	Blueprint_Plans \
	DNA \
	Focus \
	Forestbird \
	Inspiration \
	Lights \
	Metropolis \
	Midnightblue \
	Nature_Illustration \
	Pencil \
	Piano \
	Portfolio \
	Progress \
	Vintage \
	Vivid \


extras_PRESENTATIONS_XMLFILES := \
	Beehive/content.xml \
	Beehive/META-INF/manifest.xml \
	Beehive/meta.xml \
	Beehive/styles.xml \
	Beehive/Thumbnails/thumbnail.png \
	Blue_Curve/content.xml \
	Blue_Curve/META-INF/manifest.xml \
	Blue_Curve/meta.xml \
	Blue_Curve/styles.xml \
	Blue_Curve/Thumbnails/thumbnail.png \
	Blueprint_Plans/content.xml \
	Blueprint_Plans/META-INF/manifest.xml \
	Blueprint_Plans/meta.xml \
	Blueprint_Plans/Pictures/10000000000000580000001F99D2A38C014B254B.png \
	Blueprint_Plans/Pictures/100000000000033C0000026D847ACE982C6E20B6.png \
	Blueprint_Plans/Pictures/10000000000004220000031A5C2E04AB7C51FB60.png \
	Blueprint_Plans/Pictures/1000020100000500000002D03918081590A75FCF.png \
	Blueprint_Plans/Pictures/1000020100000500000002D0F25E20EA381A5EBD.png \
	Blueprint_Plans/styles.xml \
	Blueprint_Plans/Thumbnails/thumbnail.png \
	DNA/content.xml \
	DNA/META-INF/manifest.xml \
	DNA/meta.xml \
	DNA/styles.xml \
	DNA/Thumbnails/thumbnail.png \
	Focus/content.xml \
	Focus/META-INF/manifest.xml \
	Focus/meta.xml \
	Focus/styles.xml \
	Focus/Thumbnails/thumbnail.png \
	Forestbird/content.xml \
	Forestbird/META-INF/manifest.xml \
	Forestbird/meta.xml \
	Forestbird/styles.xml \
	Forestbird/Thumbnails/thumbnail.png \
	Inspiration/content.xml \
	Inspiration/META-INF/manifest.xml \
	Inspiration/meta.xml \
	Inspiration/styles.xml \
	Inspiration/Thumbnails/thumbnail.png \
	Lights/content.xml \
	Lights/META-INF/manifest.xml \
	Lights/meta.xml \
	Lights/styles.xml \
	Lights/Thumbnails/thumbnail.png \
	Metropolis/content.xml \
	Metropolis/META-INF/manifest.xml \
	Metropolis/meta.xml \
	Metropolis/Pictures/10000201000003AF000002126A94207C4902B60B.png \
	Metropolis/Pictures/100021B200008D2E00004F60B312A649B9EEC188.svg \
	Metropolis/styles.xml \
	Metropolis/Thumbnails/thumbnail.png \
	Midnightblue/content.xml \
	Midnightblue/META-INF/manifest.xml \
	Midnightblue/meta.xml \
	Midnightblue/styles.xml \
	Midnightblue/Thumbnails/thumbnail.png \
	Nature_Illustration/content.xml \
	Nature_Illustration/META-INF/manifest.xml \
	Nature_Illustration/meta.xml \
	Nature_Illustration/Pictures/10000000000004210000031931EA92AB7FB2FD6C.jpg \
	Nature_Illustration/Pictures/100000000000042100000319892187DC121460D8.jpg \
	Nature_Illustration/styles.xml \
	Nature_Illustration/Thumbnails/thumbnail.png \
	Pencil/content.xml \
	Pencil/META-INF/manifest.xml \
	Pencil/meta.xml \
	Pencil/Pictures/1000020100000088000000870A26C51C993940C3.png \
	Pencil/Pictures/100002010000037D0000001C2F14EFE8274AD9FC.png \
	Pencil/Pictures/1000020100000402000000A2F7F509AE74DE6C69.png \
	Pencil/styles.xml \
	Pencil/Thumbnails/thumbnail.png \
	Piano/content.xml \
	Piano/META-INF/manifest.xml \
	Piano/meta.xml \
	Piano/Pictures/100000000000035400000120266623530F1969D0.png \
	Piano/Pictures/100002010000033A0000006EDD62437E1CC42053.png \
	Piano/styles.xml \
	Piano/Thumbnails/thumbnail.png \
	Portfolio/content.xml \
	Portfolio/META-INF/manifest.xml \
	Portfolio/meta.xml \
	Portfolio/styles.xml \
	Portfolio/Thumbnails/thumbnail.png \
	Progress/content.xml \
	Progress/META-INF/manifest.xml \
	Progress/meta.xml \
	Progress/styles.xml \
	Progress/Thumbnails/thumbnail.png \
	Vintage/content.xml \
	Vintage/META-INF/manifest.xml \
	Vintage/meta.xml \
	Vintage/Pictures/10000000000000580000001F99D2A38C014B254B.png \
	Vintage/Pictures/10000201000004B000000337490AE22881331C9B.png \
	Vintage/Pictures/10000201000004B00000033776CA70A4BFE2196B.png \
	Vintage/Pictures/10000201000004B0000003379D1127938561C230.png \
	Vintage/Pictures/10000201000004B000000337D99A2303560BD59C.png \
	Vintage/styles.xml \
	Vintage/Thumbnails/thumbnail.png \
	Vivid/content.xml \
	Vivid/META-INF/manifest.xml \
	Vivid/meta.xml \
	Vivid/styles.xml \
	Vivid/Thumbnails/thumbnail.png \


extras_PRESENTATIONS_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),$(atexts)/mimetype)


$(call gb_CustomTarget_get_target,extras/source/templates/presnt) : \
	$(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/$(atexts).otp)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%/mimetype : $(SRCDIR)/extras/source/templates/presnt/%/mimetype
	$(call gb_Output_announce,templates/presnt/$*/mimetype,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*/mimetype,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*/mimetype,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.jpg : $(SRCDIR)/extras/source/templates/presnt/%.jpg
	$(call gb_Output_announce,templates/presnt/$*.jpg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.jpg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*.jpg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.svg : $(SRCDIR)/extras/source/templates/presnt/%.svg
	$(call gb_Output_announce,templates/presnt/$*.svg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.svg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*.svg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.png : $(SRCDIR)/extras/source/templates/presnt/%.png
	$(call gb_Output_announce,templates/presnt/$*.png,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.png,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*.png,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.svm : $(SRCDIR)/extras/source/templates/presnt/%.svm
	$(call gb_Output_announce,templates/presnt/$*.svm,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.svm,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*.svm,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.xml : $(SRCDIR)/extras/source/templates/presnt/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,templates/presnt/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/presnt/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.otp :
	$(call gb_Output_announce,templates/presnt/$*.otp,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/presnt/$*.otp,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_PRESENTATIONS_DIR) && \
		zip -q0X --filesync --must-match $@ $(EXTRAS_PRESENTATIONS_MIMEFILES_FILTER) && \
		zip -qrX --must-match $@ $(EXTRAS_PRESENTATIONS_XMLFILES_FILTER) \
	)
	$(call gb_Trace_EndRange,templates/presnt/$*.otp,ZIP)

define extras_Tplpresnt_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tplpresnt_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_PRESENTATIONS_MIMETYPEFILES) $(extras_PRESENTATIONS_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PRESENTATIONS_MIMEFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_PRESENTATIONS_MIMETYPEFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PRESENTATIONS_XMLFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_PRESENTATIONS_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PRESENTATIONS_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_PRESENTATIONS_MIMETYPEFILES) $(extras_PRESENTATIONS_XMLFILES),\
	$(call extras_Tplpresnt_make_file_deps,extras/source/templates/presnt,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),\
	$(call extras_Tplpresnt_make_zip_deps,extras/source/templates/presnt,$(atexts).otp,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
