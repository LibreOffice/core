# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/presnt))

# # for OTP templates: mimetype, content.xml, styles.xml, META-INF/manifest.xml and
# Thumbnails/thumbnail.png files are automatically added for each template
# list of meta.xml files (one per template) + other files (content.xml, manifest.rdf, settings.xml, pictures...)
extras_PRESENTATIONS_XMLFILES := \
	Beehive/meta.xml  \
	Blue_Curve/meta.xml  \
	Blueprint_Plans/meta.xml  \
	Candy/meta.xml  \
	DNA/meta.xml  \
	Focus/meta.xml  \
	Forestbird/meta.xml  \
	Grey_Elegant/meta.xml  \
	Growing_Liberty/meta.xml  \
	Freshes/meta.xml  \
	Inspiration/meta.xml  \
	Lights/meta.xml  \
	Metropolis/meta.xml  \
	Midnightblue/meta.xml  \
	Nature_Illustration/meta.xml  \
	Pencil/meta.xml  \
	Piano/meta.xml  \
	Portfolio/meta.xml  \
	Progress/meta.xml  \
	Sunset/meta.xml  \
	Vintage/meta.xml  \
	Vivid/meta.xml  \
	Yellow_Idea/meta.xml  \
	Freshes/Object_1/content.xml  \
	Freshes/Object_2/content.xml  \
	Blueprint_Plans/Pictures/10000000000000580000001F99D2A38C014B254B.png  \
	Blueprint_Plans/Pictures/100000000000033C0000026D847ACE982C6E20B6.png  \
	Blueprint_Plans/Pictures/10000000000004220000031A5C2E04AB7C51FB60.png  \
	Blueprint_Plans/Pictures/1000020100000500000002D03918081590A75FCF.png  \
	Blueprint_Plans/Pictures/1000020100000500000002D0F25E20EA381A5EBD.png  \
	Candy/Pictures/10000000000004A800000500DD5BB9FFED422172.png  \
	Candy/Pictures/10000000000005000000050014DEA376B6E67205.png  \
	Candy/Pictures/1000000000000780000004383B6CE268A9802A07.png  \
	Candy/Pictures/10000000000007800000043870026B1FACED0D1B.png  \
	Freshes/Pictures/10000000000001980000067FB46981E5FBCD5A53.jpg  \
	Freshes/Pictures/10000000000002C5000003C02FC12A07C25F42DF.jpg  \
	Freshes/Pictures/10000000000002D50000067FACCA26111F046EB5.jpg  \
	Freshes/Pictures/100000000000030F0000036F55A62FC34FCAB38E.jpg  \
	Freshes/Pictures/100000000000049100000359E7B1AC100BA62FBB.jpg  \
	Freshes/Pictures/10000000000006D7000003A637B7BBC50EDF4D24.jpg  \
	Freshes/Pictures/1000000000000B8B0000067F7C4C059ECC323C6F.jpg  \
	Grey_Elegant/Pictures/1000000000000007000000075D9D31963C8C0E25.jpg  \
	Grey_Elegant/Pictures/1000000000000007000000076677E5BE1EDAF335.jpg  \
	Grey_Elegant/Pictures/10000000000001920000019230489F7A89351262.jpg  \
	Grey_Elegant/Pictures/10000109000002950000029599843C084F54196C.svg  \
	Grey_Elegant/Pictures/100001E0000002950000029531405702AAD800D0.svg  \
	Grey_Elegant/Pictures/10000201000000190000001945D72F17575B71B4.png  \
	Grey_Elegant/Pictures/1000020100000019000000194F0EE08BB7018624.png  \
	Grey_Elegant/Pictures/10000201000000190000001967D610D5BD5B9486.png  \
	Grey_Elegant/Pictures/100002010000001900000019B19ADF33FD67D860.png  \
	Grey_Elegant/Pictures/100002010000001900000019CA03B954885E470D.png  \
	Grey_Elegant/Pictures/10000201000003300000026455AC9786240DB2F8.png  \
	Grey_Elegant/Pictures/100002510000029500000295D9236A158400ED63.svg  \
	Grey_Elegant/Pictures/100002800000029500000295458C6106AED5C839.svg  \
	Grey_Elegant/Pictures/100002CA0000029500000295A190AE1D987B676B.svg  \
	Grey_Elegant/Pictures/1000DC810000A57800007C20AABC768B7E7ACB30.svg  \
	Growing_Liberty/Pictures/10000000000007800000043801760C0AF0E62E0B.jpg  \
	Growing_Liberty/Pictures/10000000000007800000043827B5F54D1F4F498F.jpg  \
	Growing_Liberty/Pictures/100002010000016A0000029374C9F2B8F26392AD.png  \
	Growing_Liberty/Pictures/10000201000001F000000351DA39462E363D7ED3.png  \
	Growing_Liberty/Pictures/10000201000001F10000029333ED1BC48076B5C4.png  \
	Growing_Liberty/Pictures/10000201000001F3000003548E51808DE171DA02.png  \
	Growing_Liberty/Pictures/10000C1C0000034200004A6A2C92F38BC117DD3C.svg  \
	Growing_Liberty/Pictures/10000C340000844B000003422549B923A55839CC.svg  \
	Growing_Liberty/Pictures/10000C8F0000034200004A6ABD021DA9BD39254C.svg  \
	Growing_Liberty/Pictures/10000CA90000844B000003426738F0E80CCFF9F3.svg  \
	Metropolis/Pictures/10000201000003AF000002126A94207C4902B60B.png  \
	Metropolis/Pictures/100021B200008D2E00004F60B312A649B9EEC188.svg  \
	Nature_Illustration/Pictures/10000000000004210000031931EA92AB7FB2FD6C.jpg  \
	Nature_Illustration/Pictures/100000000000042100000319892187DC121460D8.jpg  \
	Pencil/Pictures/1000020100000088000000870A26C51C993940C3.png  \
	Pencil/Pictures/100002010000037D0000001C2F14EFE8274AD9FC.png  \
	Pencil/Pictures/1000020100000402000000A2F7F509AE74DE6C69.png  \
	Piano/Pictures/100000000000035400000120266623530F1969D0.png  \
	Piano/Pictures/100002010000033A0000006EDD62437E1CC42053.png  \
	Vintage/Pictures/10000000000000580000001F99D2A38C014B254B.png  \
	Vintage/Pictures/10000201000004B000000337490AE22881331C9B.png  \
	Vintage/Pictures/10000201000004B00000033776CA70A4BFE2196B.png  \
	Vintage/Pictures/10000201000004B0000003379D1127938561C230.png  \
	Vintage/Pictures/10000201000004B000000337D99A2303560BD59C.png  \
	Yellow_Idea/Pictures/1000020100000008000000218CFF265B51917950.png  \
	Yellow_Idea/Pictures/100002010000001A0000001A21D7EADBA56038F4.png  \
	Yellow_Idea/Pictures/100002010000001A0000001A609D6B9B144132F8.png  \
	Yellow_Idea/Pictures/10000201000000210000000835496FFA25AE5DB6.png  \
	Yellow_Idea/Pictures/1000020100000021000000087A16FD8FB901FE83.png  \
	Yellow_Idea/Pictures/100002010000004100000019BACEF928CB2A84CB.png  \
	Yellow_Idea/Pictures/100002010000004D0000008BE35F31AC2914377D.png  \
	Yellow_Idea/Pictures/100002010000004E000000192EFFFDEF12FDB2A8.png  \
	Yellow_Idea/Pictures/100002010000009E000000BD075AA48B79110C47.png  \

# param: style-base (e.g. Modern)
extras_PRESNT_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_PRESENTATIONS_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(gb_CustomTarget_workdir)/extras/source/templates/presnt/% : $(SRCDIR)/extras/source/templates/presnt/% \
        | $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*).dir
	$(call gb_Output_announce,templates/presnt/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/presnt/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/presnt/$*,CPY)

# test and copy xml files
$(gb_CustomTarget_workdir)/extras/source/templates/presnt/%.xml : $(SRCDIR)/extras/source/templates/presnt/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*.xml).dir
	$(call gb_Output_announce,templates/presnt/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/presnt/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/presnt/$*.xml,XSL)

# zip files to OTP
$(gb_CustomTarget_workdir)/extras/source/templates/presnt/%.otp : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*/,\
            mimetype $$(call extras_PRESNT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*/,\
            content.xml $$(call extras_PRESNT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*/,\
            styles.xml $$(call extras_PRESNT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*/,\
            META-INF/manifest.xml $$(call extras_PRESNT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/presnt/$$*/,\
            Thumbnails/thumbnail.png $$(call extras_PRESNT_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/presnt/$*.otp,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/presnt/$*.otp,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml styles.xml META-INF/manifest.xml Thumbnails/thumbnail.png && \
		$(WSL) zip -qrX --must-match $@ $(call extras_PRESNT_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/presnt/$*.otp,ZIP)

# vim: set noet sw=4 ts=4:
