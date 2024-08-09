# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# this file lists the contents/parts that make up the individual templates that are created during
# the build.
#
# The Package_*.mk files list the files that should be built and delivered to the installation tree,
# and CustomTarget_templates.mk has the actual recipes to assemble and package the template files.
#
# The split used here follows the split from the Package_*mk files, but that split is arbitrary/
# artificial, they are ultimately combined into a single variable.
# A split only matters on the Package level, but even there only the wizard/desktop files are
# "special" (they are always created/packaged, while the rests is optional/behind a WITH_TEMPLATES
# conditional)

# Specify filenames relative to SRC_ROOT/extras/source/templates
# mimetype, META-INF/manifest.xml, content.xml, meta.xml and styles.xml are always assumed to be
# present and can be omitted

extra_files_COMMON := \
    draw/bpmn/Thumbnails/thumbnail.png \
    l10n/ja_ott_normal/manifest.rdf \
    l10n/ja_ott_normal/settings.xml \
    l10n/ja_ott_normal/Thumbnails/thumbnail.png \
    l10n/zh_CN_ott_normal/Thumbnails/thumbnail.png \
    officorr/Modern_business_letter_sans_serif/manifest.rdf \
    officorr/Modern_business_letter_sans_serif/Thumbnails/thumbnail.png \
    officorr/Modern_business_letter_serif/manifest.rdf \
    officorr/Modern_business_letter_serif/Thumbnails/thumbnail.png \
    offimisc/Businesscard-with-logo/manifest.rdf \
    offimisc/Businesscard-with-logo/Pictures/10000201000001F4000000A0108F3F06.png \
    offimisc/Businesscard-with-logo/settings.xml \
    offimisc/Businesscard-with-logo/Thumbnails/thumbnail.png \
    personal/CV/manifest.rdf \
    personal/CV/Thumbnails/thumbnail.png \
    personal/Resume1page/manifest.rdf \
    personal/Resume1page/Thumbnails/thumbnail.png \
    styles/Default/Thumbnails/thumbnail.png \
    styles/Modern/Thumbnails/thumbnail.png \
    styles/Simple/Thumbnails/thumbnail.png \


extra_files_PRESENTATIONS := \
    presnt/Beehive/Thumbnails/thumbnail.png \
    presnt/Blue_Curve/Thumbnails/thumbnail.png \
    presnt/Blueprint_Plans/Pictures/100000000000033C0000026D847ACE982C6E20B6.png  \
    presnt/Blueprint_Plans/Pictures/10000000000004220000031A5C2E04AB7C51FB60.png  \
    presnt/Blueprint_Plans/Pictures/1000020100000500000002D03918081590A75FCF.png  \
    presnt/Blueprint_Plans/Pictures/1000020100000500000002D0F25E20EA381A5EBD.png  \
    presnt/Blueprint_Plans/Thumbnails/thumbnail.png \
    presnt/Candy/Pictures/10000000000004A800000500DD5BB9FFED422172.png  \
    presnt/Candy/Pictures/10000000000005000000050014DEA376B6E67205.png  \
    presnt/Candy/Pictures/1000000000000780000004383B6CE268A9802A07.png  \
    presnt/Candy/Pictures/10000000000007800000043870026B1FACED0D1B.png  \
    presnt/Candy/Thumbnails/thumbnail.png \
    presnt/DNA/Thumbnails/thumbnail.png \
    presnt/Focus/Thumbnails/thumbnail.png \
    presnt/Forestbird/Thumbnails/thumbnail.png \
    presnt/Freshes/Object_1/content.xml \
    presnt/Freshes/Object_2/content.xml \
    presnt/Freshes/Pictures/10000000000001980000067FB46981E5FBCD5A53.jpg  \
    presnt/Freshes/Pictures/10000000000002C5000003C02FC12A07C25F42DF.jpg  \
    presnt/Freshes/Pictures/10000000000002D50000067FACCA26111F046EB5.jpg  \
    presnt/Freshes/Pictures/100000000000030F0000036F55A62FC34FCAB38E.jpg  \
    presnt/Freshes/Pictures/100000000000049100000359E7B1AC100BA62FBB.jpg  \
    presnt/Freshes/Pictures/10000000000006D7000003A637B7BBC50EDF4D24.jpg  \
    presnt/Freshes/Pictures/1000000000000B8B0000067F7C4C059ECC323C6F.jpg  \
    presnt/Freshes/Thumbnails/thumbnail.png \
    presnt/Grey_Elegant/Pictures/1000000000000007000000075D9D31963C8C0E25.jpg  \
    presnt/Grey_Elegant/Pictures/1000000000000007000000076677E5BE1EDAF335.jpg  \
    presnt/Grey_Elegant/Pictures/10000000000001920000019230489F7A89351262.jpg  \
    presnt/Grey_Elegant/Pictures/10000109000002950000029599843C084F54196C.svg  \
    presnt/Grey_Elegant/Pictures/100001E0000002950000029531405702AAD800D0.svg  \
    presnt/Grey_Elegant/Pictures/10000201000000190000001945D72F17575B71B4.png  \
    presnt/Grey_Elegant/Pictures/1000020100000019000000194F0EE08BB7018624.png  \
    presnt/Grey_Elegant/Pictures/10000201000000190000001967D610D5BD5B9486.png  \
    presnt/Grey_Elegant/Pictures/100002010000001900000019B19ADF33FD67D860.png  \
    presnt/Grey_Elegant/Pictures/100002010000001900000019CA03B954885E470D.png  \
    presnt/Grey_Elegant/Pictures/10000201000003300000026455AC9786240DB2F8.png  \
    presnt/Grey_Elegant/Pictures/100002510000029500000295D9236A158400ED63.svg  \
    presnt/Grey_Elegant/Pictures/100002800000029500000295458C6106AED5C839.svg  \
    presnt/Grey_Elegant/Pictures/100002CA0000029500000295A190AE1D987B676B.svg  \
    presnt/Grey_Elegant/Pictures/1000DC810000A57800007C20AABC768B7E7ACB30.svg  \
    presnt/Grey_Elegant/Thumbnails/thumbnail.png \
    presnt/Growing_Liberty/Pictures/10000000000007800000043801760C0AF0E62E0B.jpg  \
    presnt/Growing_Liberty/Pictures/10000000000007800000043827B5F54D1F4F498F.jpg  \
    presnt/Growing_Liberty/Pictures/100002010000016A0000029374C9F2B8F26392AD.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F000000351DA39462E363D7ED3.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F10000029333ED1BC48076B5C4.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F3000003548E51808DE171DA02.png  \
    presnt/Growing_Liberty/Pictures/10000C1C0000034200004A6A2C92F38BC117DD3C.svg  \
    presnt/Growing_Liberty/Pictures/10000C340000844B000003422549B923A55839CC.svg  \
    presnt/Growing_Liberty/Pictures/10000C8F0000034200004A6ABD021DA9BD39254C.svg  \
    presnt/Growing_Liberty/Pictures/10000CA90000844B000003426738F0E80CCFF9F3.svg  \
    presnt/Growing_Liberty/Thumbnails/thumbnail.png \
    presnt/Inspiration/Thumbnails/thumbnail.png \
    presnt/Lights/Thumbnails/thumbnail.png \
    presnt/Metropolis/Pictures/100021B200008D2E00004F60B312A649B9EEC188.svg  \
    presnt/Metropolis/Thumbnails/thumbnail.png \
    presnt/Midnightblue/Thumbnails/thumbnail.png \
    presnt/Nature_Illustration/Thumbnails/thumbnail.png \
    presnt/Pencil/Pictures/1000020100000088000000870A26C51C993940C3.png  \
    presnt/Pencil/Pictures/100002010000037D0000001C2F14EFE8274AD9FC.png  \
    presnt/Pencil/Pictures/1000020100000402000000A2F7F509AE74DE6C69.png  \
    presnt/Pencil/Thumbnails/thumbnail.png \
    presnt/Piano/Pictures/100000000000035400000120266623530F1969D0.png  \
    presnt/Piano/Pictures/100002010000033A0000006EDD62437E1CC42053.png  \
    presnt/Piano/Thumbnails/thumbnail.png \
    presnt/Portfolio/Thumbnails/thumbnail.png \
    presnt/Progress/Thumbnails/thumbnail.png \
    presnt/Sunset/Thumbnails/thumbnail.png \
    presnt/Vintage/Pictures/10000201000004B000000337490AE22881331C9B.png  \
    presnt/Vintage/Pictures/10000201000004B00000033776CA70A4BFE2196B.png  \
    presnt/Vintage/Pictures/10000201000004B0000003379D1127938561C230.png  \
    presnt/Vintage/Pictures/10000201000004B000000337D99A2303560BD59C.png  \
    presnt/Vintage/Thumbnails/thumbnail.png \
    presnt/Vivid/Thumbnails/thumbnail.png \
    presnt/Yellow_Idea/Thumbnails/thumbnail.png \


extra_files_WIZARD := \
    wizard/agenda/10grey/manifest.rdf \
    wizard/agenda/10grey/settings.xml \
    wizard/agenda/1simple/manifest.rdf \
    wizard/agenda/1simple/settings.xml \
    wizard/agenda/2elegant/manifest.rdf \
    wizard/agenda/2elegant/Pictures/1000000000000280000000052876A81A.gif \
    wizard/agenda/2elegant/Pictures/100002000000008F0000000A83659D51.gif \
    wizard/agenda/2elegant/settings.xml \
    wizard/agenda/3modern/manifest.rdf \
    wizard/agenda/3modern/settings.xml \
    wizard/agenda/4classic/manifest.rdf \
    wizard/agenda/4classic/settings.xml \
    wizard/agenda/5blue/manifest.rdf \
    wizard/agenda/5blue/settings.xml \
    wizard/agenda/6orange/manifest.rdf \
    wizard/agenda/6orange/settings.xml \
    wizard/agenda/7red/manifest.rdf \
    wizard/agenda/7red/settings.xml \
    wizard/agenda/8green/manifest.rdf \
    wizard/agenda/8green/settings.xml \
    wizard/agenda/9colorful/manifest.rdf \
    wizard/agenda/9colorful/settings.xml \
    wizard/agenda/aw-10grey/manifest.rdf \
    wizard/agenda/aw-10grey/settings.xml \
    wizard/agenda/aw-1simple/manifest.rdf \
    wizard/agenda/aw-1simple/settings.xml \
    wizard/agenda/aw-2elegant/manifest.rdf \
    wizard/agenda/aw-2elegant/Pictures/1000000000000280000000052876A81A.gif \
    wizard/agenda/aw-2elegant/Pictures/100002000000008F0000000A83659D51.gif \
    wizard/agenda/aw-2elegant/settings.xml \
    wizard/agenda/aw-3modern/manifest.rdf \
    wizard/agenda/aw-3modern/settings.xml \
    wizard/agenda/aw-4classic/manifest.rdf \
    wizard/agenda/aw-4classic/settings.xml \
    wizard/agenda/aw-5blue/manifest.rdf \
    wizard/agenda/aw-5blue/settings.xml \
    wizard/agenda/aw-6orange/manifest.rdf \
    wizard/agenda/aw-6orange/settings.xml \
    wizard/agenda/aw-7red/manifest.rdf \
    wizard/agenda/aw-7red/settings.xml \
    wizard/agenda/aw-8green/manifest.rdf \
    wizard/agenda/aw-8green/settings.xml \
    wizard/agenda/aw-9colorful/manifest.rdf \
    wizard/agenda/aw-9colorful/settings.xml \
    wizard/desktop/html/settings.xml \
    wizard/desktop/idxexample/settings.xml \
    wizard/fax/bus-classic_f/manifest.rdf \
    wizard/fax/bus-classic_f/Pictures/200000520000271100001F73A977C564.wmf \
    wizard/fax/bus-classic_f/settings.xml \
    wizard/fax/bus-classic-pri_f/manifest.rdf \
    wizard/fax/bus-classic-pri_f/settings.xml \
    wizard/fax/bus-modern_f/manifest.rdf \
    wizard/fax/bus-modern_f/Pictures/2000004E0000271100001F73772F7F48.wmf \
    wizard/fax/bus-modern_f/settings.xml \
    wizard/fax/bus-modern-pri_f/manifest.rdf \
    wizard/fax/bus-modern-pri_f/settings.xml \
    wizard/fax/pri-bottle_f/Pictures/200002EB000022DA000024E1F891208C.wmf \
    wizard/fax/pri-bottle_f/settings.xml \
    wizard/fax/pri-fax_f/Pictures/2000032600002711000026A54A905481.wmf \
    wizard/fax/pri-fax_f/settings.xml \
    wizard/fax/pri-lines_f/settings.xml \
    wizard/fax/pri-marine_f/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
    wizard/fax/pri-marine_f/settings.xml \
    wizard/letter/bus-elegant_l/manifest.rdf \
    wizard/letter/bus-elegant_l/Pictures/10000000000001B8000000824B7EA208.png \
    wizard/letter/bus-elegant_l/settings.xml \
    wizard/letter/bus-modern_l/manifest.rdf \
    wizard/letter/bus-modern_l/Pictures/10000000000001B8000000824B7EA208.png \
    wizard/letter/bus-modern_l/settings.xml \
    wizard/letter/bus-office_l/manifest.rdf \
    wizard/letter/bus-office_l/Pictures/10000000000001B8000000824B7EA208.png \
    wizard/letter/bus-office_l/settings.xml \
    wizard/letter/off-elegant_l/manifest.rdf \
    wizard/letter/off-elegant_l/Pictures/2000000D00000434000006FD6841386B.svm \
    wizard/letter/off-elegant_l/settings.xml \
    wizard/letter/off-modern_l/manifest.rdf \
    wizard/letter/off-modern_l/Pictures/10000000000000110000001142D1547F.png \
    wizard/letter/off-modern_l/settings.xml \
    wizard/letter/off-office_l/manifest.rdf \
    wizard/letter/off-office_l/settings.xml \
    wizard/letter/pri-bottle_l/Pictures/200002EB000022DA000024E1F891208C.wmf \
    wizard/letter/pri-bottle_l/settings.xml \
    wizard/letter/pri-mail_l/Pictures/200000430000271100001205CAE80946.wmf \
    wizard/letter/pri-mail_l/settings.xml \
    wizard/letter/pri-marine_l/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
    wizard/letter/pri-marine_l/settings.xml \
    wizard/letter/pri-redline_l/settings.xml \
    wizard/report/cnt-01/settings.xml \
    wizard/report/cnt-011/settings.xml \
    wizard/report/cnt-012/settings.xml \
    wizard/report/cnt-02/settings.xml \
    wizard/report/cnt-021/settings.xml \
    wizard/report/cnt-022/settings.xml \
    wizard/report/cnt-03/settings.xml \
    wizard/report/cnt-031/settings.xml \
    wizard/report/cnt-032/settings.xml \
    wizard/report/cnt-04/settings.xml \
    wizard/report/cnt-041/settings.xml \
    wizard/report/cnt-042/settings.xml \
    wizard/report/cnt-05/settings.xml \
    wizard/report/cnt-051/settings.xml \
    wizard/report/cnt-052/settings.xml \
    wizard/report/cnt-06/settings.xml \
    wizard/report/cnt-061/settings.xml \
    wizard/report/cnt-062/settings.xml \
    wizard/report/cnt-default/settings.xml \
    wizard/report/stl-01/manifest.rdf \
    wizard/report/stl-01/Pictures/100002000000001400000014E87B6AC5.gif \
    wizard/report/stl-01/Pictures/2000018100001CB100002711BFED37A8.wmf \
    wizard/report/stl-01/settings.xml \
    wizard/report/stl-02/manifest.rdf \
    wizard/report/stl-02/Pictures/2000009E0000380100002657BA019D86.wmf \
    wizard/report/stl-02/settings.xml \
    wizard/report/stl-03/manifest.rdf \
    wizard/report/stl-03/Pictures/2000024900002711000021F4DCB2DF4A.wmf \
    wizard/report/stl-03/settings.xml \
    wizard/report/stl-04/manifest.rdf \
    wizard/report/stl-04/Pictures/10000000000000400000004077CDC8F9.png \
    wizard/report/stl-04/Pictures/10000000000001C6000000E44DCA6C94.jpg \
    wizard/report/stl-04/settings.xml \
    wizard/report/stl-05/manifest.rdf \
    wizard/report/stl-05/Pictures/2000036D000027110000145EB7E93049.wmf \
    wizard/report/stl-05/settings.xml \
    wizard/report/stl-06/manifest.rdf \
    wizard/report/stl-06/Pictures/100000000000005A000000268DE98CCE.gif \
    wizard/report/stl-06/Pictures/100000000000021B000001E8BE936AB6.gif \
    wizard/report/stl-06/settings.xml \
    wizard/report/stl-07/manifest.rdf \
    wizard/report/stl-07/settings.xml \
    wizard/report/stl-08/manifest.rdf \
    wizard/report/stl-08/Pictures/100000000000005E0000005E97FB9EF6.jpg \
    wizard/report/stl-08/settings.xml \
    wizard/report/stl-09/manifest.rdf \
    wizard/report/stl-09/Pictures/10000000000000050000004D32652675.jpg \
    wizard/report/stl-09/Pictures/100002000000001400000014E87B6AC5.gif \
    wizard/report/stl-09/Pictures/2000025100001D92000017719EB70CFB.wmf \
    wizard/report/stl-09/settings.xml \
    wizard/report/stl-default/manifest.rdf \
    wizard/report/stl-default/settings.xml \
    wizard/styles/black_white/settings.xml \
    wizard/styles/blackberry/settings.xml \
    wizard/styles/default/settings.xml \
    wizard/styles/diner/settings.xml \
    wizard/styles/fall/settings.xml \
    wizard/styles/glacier/settings.xml \
    wizard/styles/green_grapes/settings.xml \
    wizard/styles/jeans/settings.xml \
    wizard/styles/marine/settings.xml \
    wizard/styles/millennium/settings.xml \
    wizard/styles/nature/settings.xml \
    wizard/styles/neon/settings.xml \
    wizard/styles/night/settings.xml \
    wizard/styles/nostalgic/settings.xml \
    wizard/styles/pastell/settings.xml \
    wizard/styles/pool/settings.xml \
    wizard/styles/pumpkin/settings.xml \
    wizard/styles/xos/settings.xml \


extra_files_WIZDESKTOP := \
    wizard/desktop/html/settings.xml \
    wizard/desktop/idxexample/Pictures/2000008600001923000012C24E0D0895.wmf \
    wizard/desktop/idxexample/settings.xml \


extra_files_TEMPLATES := \
    $(extra_files_COMMON) \
    $(extra_files_PRESENTATIONS) \
    $(extra_files_WIZARD) \
    $(extra_files_WIZDESKTOP) \


extra_files_TEMPLATES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extra_files_TEMPLATES)))

# vim: set noet sw=4 ts=4:
