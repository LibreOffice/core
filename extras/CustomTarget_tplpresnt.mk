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
	Freshes \
	Gotong_Royong \
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
	Candy/Pictures/100000000000000400000001B46A8EBA49DAD60A.png \
	Candy/Pictures/10000000000007800000043870026B1FACED0D1B.png \
	Candy/Pictures/100002010000070E0000007A3641833282F827C5.png \
	Candy/Pictures/10000000000004A800000500DD5BB9FFED422172.png \
	Candy/Pictures/100002010000001B0000000AEB61C0813AEE6F25.png \
	Candy/Pictures/100002010000078100000028DC0E69D21306DE4C.png \
	Candy/Pictures/10000000000005000000050014DEA376B6E67205.png \
	Candy/Pictures/100002010000001F0000000BA83CF1B9CFF4042E.png \
	Candy/Pictures/1000000000000780000004383B6CE268A9802A07.png \
	Candy/Pictures/1000020100000185000001677E243EDE4274F096.png \
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
	Freshes/META-INF/manifest.xml \
	Freshes/Object_1/content.xml \
	Freshes/Object_1/meta.xml \
	Freshes/Object_1/styles.xml \
	Freshes/Object_2/content.xml \
	Freshes/Object_2/meta.xml \
	Freshes/Object_2/styles.xml \
	Freshes/Object_4/content.xml \
	Freshes/Object_4/meta.xml \
	Freshes/Object_4/styles.xml \
	Freshes/Pictures/10000000000001980000067FB46981E5FBCD5A53.jpg \
	Freshes/Pictures/10000000000002C5000003C00D994E0B9681AE23.jpg \
	Freshes/Pictures/10000000000002D50000067F097E75D5BAC429C7.jpg \
	Freshes/Pictures/100000000000030F0000036FF294B251FB06E041.jpg \
	Freshes/Pictures/100000000000049100000359A264C7C22C173FA8.jpg \
	Freshes/Pictures/10000000000006D7000003A6682E4AA1B60AAA8A.jpg \
	Freshes/Pictures/1000000000000B8B0000067FE7E8A5A3415E8510.jpg \
	Freshes/Pictures/TablePreview1.svm \
	Freshes/Thumbnails/thumbnail.png \
	Freshes/content.xml \
	Freshes/meta.xml \
	Freshes/styles.xml \
	Gotong_Royong/meta.xml \
	Gotong_Royong/styles.xml \
	Gotong_Royong/content.xml \
	Gotong_Royong/META-INF/manifest.xml \
	Gotong_Royong/Thumbnails/thumbnail.png \
	Gotong_Royong/Pictures/100000000000078000000438173A72363C15C3D1.png \
	Gotong_Royong/Pictures/10000201000001FC000001FC83C8AB04C1E0B2D4.png \
	Gotong_Royong/Pictures/1000020100000526000003FA51DC7E3B86A58E39.png \
	Gotong_Royong/Pictures/1000020100000017000000179FB23D9B750CDE98.png \
	Gotong_Royong/Pictures/100002010000027D000002B6172A32BEAD4B137A.png \
	Gotong_Royong/Pictures/100002010000052A00000015104143C17D86C817.png \
	Gotong_Royong/Pictures/1000020100000019000000146198876E109275E0.png \
	Gotong_Royong/Pictures/10000201000002A2000003BE0299FABC236EDA9B.png \
	Gotong_Royong/Pictures/10000201000007800000039A71CF2B4E9839F11E.png \
	Gotong_Royong/Pictures/100002010000001A0000001A84984FCB064315A9.png \
	Gotong_Royong/Pictures/10000201000002A8000001CEC4CCB69D36CF2042.png \
	Gotong_Royong/Pictures/1000020100000780000003D6D57F1A321384327F.png \
	Gotong_Royong/Pictures/1000020100000079000000342C2C8714A90D5A7B.png \
	Gotong_Royong/Pictures/10000201000003680000023E0A0096F1CDFDDF67.png \
	Gotong_Royong/Pictures/1000C2280000028D00000213392CB29515C72D3F.svg \
	Gotong_Royong/Pictures/10000201000000A3000004356DF09231B5E1281C.png \
	Gotong_Royong/Pictures/10000201000003D40000000CAC7CB2D6D03F13D7.png \
	Gotong_Royong/Pictures/1000C3FF0000026B0000026BBD66575EF6D00B3B.svg \
	Gotong_Royong/Pictures/10000201000000A300000435DF3CD8E2CA664B8A.png \
	Gotong_Royong/Pictures/100002010000043800000438DBE30C09AC57E6E6.png \
	Gotong_Royong/Pictures/1000C495000002AD000002AD27EA1D4F6CC3599D.svg \
	Gotong_Royong/Pictures/10000201000000B90000003B5ED14ADDB25E77DC.png \
	Gotong_Royong/Pictures/100002010000045F000004384333BFDCB7CAA6CA.png \
	Gotong_Royong/Pictures/10027C820000755400004D36580BD3F03398CAE9.svg \
	Gotong_Royong/Pictures/10000201000001E600000054A0B3FFE0A008B8BA.png \
	Gotong_Royong/Pictures/10000201000004BE00000546BA0853C8E9F2A0FE.png \
	Grey_Elegant/meta.xml \
	Grey_Elegant/styles.xml \
	Grey_Elegant/content.xml \
	Grey_Elegant/META-INF/manifest.xml \
	Grey_Elegant/Thumbnails/thumbnail.png \
	Grey_Elegant/Pictures/1000000000000007000000073659829D002593D5.jpg \
	Grey_Elegant/Pictures/1000020100000019000000195EC1E2054BFAD9D3.png \
	Grey_Elegant/Pictures/1000020100000330000002645B2B744CE825EF5E.png \
	Grey_Elegant/Pictures/100000000000000700000007B7FA142B482B262B.jpg \
	Grey_Elegant/Pictures/10000201000000190000001982F004DB24C87E80.png \
	Grey_Elegant/Pictures/100002010000033000000264E09BE80C150D3D4F.png \
	Grey_Elegant/Pictures/100000000000000800000008202A206BCF5A9BE8.jpg \
	Grey_Elegant/Pictures/1000020100000019000000199D20B70F500F79EE.png \
	Grey_Elegant/Pictures/10000201000003450000022E1193F14082413842.png \
	Grey_Elegant/Pictures/1000000000000173000001EFBFCF215D5F505109.jpg \
	Grey_Elegant/Pictures/100002010000001900000019B6159168AC20D9E3.png \
	Grey_Elegant/Pictures/10000234000002950000029586CC39B65277F3DC.svg \
	Grey_Elegant/Pictures/100000000000019200000192E10720968CAFF7FA.jpg \
	Grey_Elegant/Pictures/100002010000001900000019E40E9ECBAC8D15B6.png \
	Grey_Elegant/Pictures/100002510000029500000295D9236A158400ED63.svg \
	Grey_Elegant/Pictures/10000000000001A20000022EB5E60C159DAE8718.jpg \
	Grey_Elegant/Pictures/100002010000002300000023464E9D30BCA857BB.png \
	Grey_Elegant/Pictures/100002800000029500000295458C6106AED5C839.svg \
	Grey_Elegant/Pictures/10000000000002200000013223C0FE110C90F7F7.jpg \
	Grey_Elegant/Pictures/100002010000002D0000002D1FF4245F5DAD1778.png \
	Grey_Elegant/Pictures/100002C50000029500000295208B2213591A4669.svg \
	Grey_Elegant/Pictures/10000109000002950000029599843C084F54196C.svg \
	Grey_Elegant/Pictures/100002010000002D0000002D4F703F26B6D30746.png \
	Grey_Elegant/Pictures/100002CA0000029500000295A190AE1D987B676B.svg \
	Grey_Elegant/Pictures/100001E0000002950000029531405702AAD800D0.svg \
	Grey_Elegant/Pictures/10000201000000870000001D92547068FD3CDBBB.png \
	Grey_Elegant/Pictures/1000DC810000A57800007C20AABC768B7E7ACB30.svg \
	Grey_Elegant/Pictures/100002000000025E0000015421462289FAE31E5F.png \
	Grey_Elegant/Pictures/100002010000018C0000018C40A858899E505BEF.png \
	Grey_Elegant/Pictures/2000001F00003EA20000232519B31386346EB0A4.svm \
	Grey_Elegant/Pictures/100002000000028E000001702D2CED1046283A5C.png \
	Grey_Elegant/Pictures/10000201000001AB000001AB39F44AA92C68E11A.png \
	Grey_Elegant/Pictures/20000233000043790000260CC9D5D5B8B6DE0478.svm \
	Grey_Elegant/Pictures/1000020100000019000000194A81626808FB577D.png \
	Grey_Elegant/Pictures/100002010000022E0000022E48FFB7CEBB6BD478.png \
	Grey_Elegant/Pictures/1000020100000019000000195289F5875EE5F150.png \
	Grey_Elegant/Pictures/100002010000029B000002199DD9907B0CBEEC7D.png \
	Growing_Liberty/meta.xml \
	Growing_Liberty/styles.xml \
	Growing_Liberty/content.xml \
	Growing_Liberty/META-INF/manifest.xml \
	Growing_Liberty/Thumbnails/thumbnail.png \
	Growing_Liberty/Pictures/10000201000001F0000003510AD092C28B33E104.png \
	Growing_Liberty/Pictures/10000201000001F100000293096DCCB2F35675D8.png \
	Growing_Liberty/Pictures/1000020100000020000002D043C6D13F127D3EEF.png \
	Growing_Liberty/Pictures/10000201000001F300000354AB37D226CA33E09E.png \
	Growing_Liberty/Pictures/1000020100000020000002D095C279FC0C593EBB.png \
	Growing_Liberty/Pictures/100002010000050000000020B9CA542EADA8F79F.png \
	Growing_Liberty/Pictures/100002010000016A0000029368E4AA223A54D05B.png \
	Growing_Liberty/Pictures/100002010000050000000020E86F037E3A70AFFD.png \
	Growing_Liberty/Pictures/10000000000007800000043884D34C72887C351F.jpg \
	Growing_Liberty/Pictures/10000000000007800000043897EDBDD04730521D.jpg \
	Growing_Liberty/Pictures/10000C8F0000034200004A6ABD021DA9BD39254C.svg \
	Growing_Liberty/Pictures/10000CA90000844B000003426738F0E80CCFF9F3.svg \
	Growing_Liberty/Pictures/10000C340000844B000003422549B923A55839CC.svg \
	Growing_Liberty/Pictures/10000C1C0000034200004A6A2C92F38BC117DD3C.svg \
	Growing_Liberty/Pictures/TablePreview1.svm \
	Growing_Liberty/Pictures/TablePreview2.svm \
	Growing_Liberty/Pictures/TablePreview3.svm \
	Growing_Liberty/Pictures/TablePreview4.svm \
	Growing_Liberty/Pictures/TablePreview5.svm \
	Growing_Liberty/Pictures/TablePreview6.svm \
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
	Yellow_Idea/content.xml \
	Yellow_Idea/meta.xml \
	Yellow_Idea/styles.xml \
	Yellow_Idea/META-INF/manifest.xml \
	Yellow_Idea/Pictures/10000000000002C2000002C1B7F42F4DE8271D81.png \
	Yellow_Idea/Pictures/100002010000001A0000001A5B32E054C05D95E5.png \
	Yellow_Idea/Pictures/100002010000007600000076BC42AA3644D44BE0.png \
	Yellow_Idea/Pictures/100000000000036A0000036AB9C53D1211DBFD8E.png \
	Yellow_Idea/Pictures/100002010000001A0000001AEE357C78D2EBA933.png \
	Yellow_Idea/Pictures/100002010000007600000076E252A7FD78A025AE.png \
	Yellow_Idea/Pictures/10000000000003C1000003C0F445ECE87BA9C279.png \
	Yellow_Idea/Pictures/100002010000002100000008124151E04327A389.png \
	Yellow_Idea/Pictures/1000020100000085000000853C3C6C67BD02C35C.png \
	Yellow_Idea/Pictures/10000000000003E8000004B0EBCEC3CEC8E1EED9.png \
	Yellow_Idea/Pictures/10000201000000210000000840658AA69E501D6E.png \
	Yellow_Idea/Pictures/100002010000008500000085C73E6B5E846825CF.png \
	Yellow_Idea/Pictures/100000000000043800000438DA4DA9B8AB9CB322.png \
	Yellow_Idea/Pictures/100002010000004100000019CF90251C6A78A330.png \
	Yellow_Idea/Pictures/1000020100000086000000862BF6E9E294ED9195.png \
	Yellow_Idea/Pictures/1000000000000440000004409F63440037E6D5C4.png \
	Yellow_Idea/Pictures/100002010000004D0000008B5018FAF0D94EF52A.png \
	Yellow_Idea/Pictures/100002010000008600000086D6CEC6EE1EC95C0A.png \
	Yellow_Idea/Pictures/100000000000050000000500A4C097FEFECD8BF7.png \
	Yellow_Idea/Pictures/100002010000004E000000199D39C9513C0C0E5D.png \
	Yellow_Idea/Pictures/100002010000009E000000BD6919A1002B355C89.png \
	Yellow_Idea/Pictures/1000020100000008000000218CFF265B51917950.png \
	Yellow_Idea/Pictures/100002010000007600000076BB8A2D7764C0444C.png \
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
