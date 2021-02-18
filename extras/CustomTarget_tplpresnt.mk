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
	Candy \
	DNA \
	Focus \
	Forestbird \
	Grey_Elegant \
	Growing_Liberty \
	Inspiration \
	Lights \
	Metropolis \
	Midnightblue \
	Nature_Illustration \
	Pencil \
	Piano \
	Portfolio \
	Progress \
	Sunset \
	Vintage \
	Vivid \
	Yellow_Idea \

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
	Candy/content.xml \
	Candy/META-INF/manifest.xml \
	Candy/meta.xml \
	Candy/styles.xml \
	Candy/Thumbnails/thumbnail.png \
	Candy/Pictures/10000000000004A800000500DD5BB9FFED422172.png \
	Candy/Pictures/10000000000005000000050014DEA376B6E67205.png \
	Candy/Pictures/1000000000000780000004383B6CE268A9802A07.png \
	Candy/Pictures/10000000000007800000043870026B1FACED0D1B.png \
	Candy/Pictures/100002010000070E0000007A072587FC46AD7996.png \
	Candy/Pictures/100002010000078100000028E4140FADDA6D818B.png \
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
	Grey_Elegant/meta.xml \
	Grey_Elegant/styles.xml \
	Grey_Elegant/content.xml \
	Grey_Elegant/META-INF/manifest.xml \
	Grey_Elegant/Thumbnails/thumbnail.png \
	Grey_Elegant/Pictures/100002010000001900000019CA03B954885E470D.png \
	Grey_Elegant/Pictures/1000000000000007000000075D9D31963C8C0E25.jpg \
	Grey_Elegant/Pictures/1000000000000007000000076677E5BE1EDAF335.jpg \
	Grey_Elegant/Pictures/100001E0000002950000029531405702AAD800D0.svg \
	Grey_Elegant/Pictures/10000201000003300000026455AC9786240DB2F8.png \
	Grey_Elegant/Pictures/100002510000029500000295D9236A158400ED63.svg \
	Grey_Elegant/Pictures/10000109000002950000029599843C084F54196C.svg \
	Grey_Elegant/Pictures/100002800000029500000295458C6106AED5C839.svg \
	Grey_Elegant/Pictures/1000DC810000A57800007C20AABC768B7E7ACB30.svg \
	Grey_Elegant/Pictures/10000201000000870000001D01E2F59468D22926.png \
	Grey_Elegant/Pictures/100002010000001900000019B19ADF33FD67D860.png \
	Grey_Elegant/Pictures/1000020100000019000000194F0EE08BB7018624.png \
	Grey_Elegant/Pictures/10000201000000190000001967D610D5BD5B9486.png \
	Grey_Elegant/Pictures/10000201000000190000001945D72F17575B71B4.png \
	Grey_Elegant/Pictures/10000000000001920000019230489F7A89351262.jpg \
	Grey_Elegant/Pictures/100002CA0000029500000295A190AE1D987B676B.svg \
	Growing_Liberty/meta.xml \
	Growing_Liberty/styles.xml \
	Growing_Liberty/content.xml \
	Growing_Liberty/META-INF/manifest.xml \
	Growing_Liberty/Thumbnails/thumbnail.png \
	Growing_Liberty/Pictures/10000201000001F10000029333ED1BC48076B5C4.png \
	Growing_Liberty/Pictures/10000000000007800000043801760C0AF0E62E0B.jpg \
	Growing_Liberty/Pictures/10000000000007800000043827B5F54D1F4F498F.jpg \
	Growing_Liberty/Pictures/100002010000050000000020D23CB52AADBCAD4A.png \
	Growing_Liberty/Pictures/10000201000001F3000003548E51808DE171DA02.png \
	Growing_Liberty/Pictures/10000C8F0000034200004A6ABD021DA9BD39254C.svg \
	Growing_Liberty/Pictures/10000C1C0000034200004A6A2C92F38BC117DD3C.svg \
	Growing_Liberty/Pictures/1000020100000020000002D01396E0588E1EBFBA.png \
	Growing_Liberty/Pictures/10000201000005000000002010F5D1670B946E0C.png \
	Growing_Liberty/Pictures/10000CA90000844B000003426738F0E80CCFF9F3.svg \
	Growing_Liberty/Pictures/10000C340000844B000003422549B923A55839CC.svg \
	Growing_Liberty/Pictures/1000020100000020000002D04D74467F50F635D0.png \
	Growing_Liberty/Pictures/10000201000001F000000351DA39462E363D7ED3.png \
	Growing_Liberty/Pictures/100002010000016A0000029374C9F2B8F26392AD.png \
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
	Sunset/content.xml \
	Sunset/meta.xml \
	Sunset/styles.xml \
	Sunset/META-INF/manifest.xml \
	Sunset/Thumbnails/thumbnail.png \
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
	Yellow_Idea/content.xml \
	Yellow_Idea/meta.xml \
	Yellow_Idea/styles.xml \
	Yellow_Idea/META-INF/manifest.xml \
	Yellow_Idea/Pictures/1000020100000021000000087A16FD8FB901FE83.png \
	Yellow_Idea/Pictures/100002010000001A0000001A609D6B9B144132F8.png \
	Yellow_Idea/Pictures/10000201000000210000000835496FFA25AE5DB6.png \
	Yellow_Idea/Pictures/100002010000001A0000001A21D7EADBA56038F4.png \
	Yellow_Idea/Pictures/1000020100000008000000218CFF265B51917950.png \
	Yellow_Idea/Pictures/100002010000004100000019BACEF928CB2A84CB.png \
	Yellow_Idea/Pictures/100002010000004E000000192EFFFDEF12FDB2A8.png \
	Yellow_Idea/Pictures/100002010000004D0000008BE35F31AC2914377D.png \
	Yellow_Idea/Pictures/100002010000009E000000BD075AA48B79110C47.png \
	Yellow_Idea/Thumbnails/thumbnail.png \

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
