# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
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
    presnt/Blueprint_Plans/Pictures/Blueprint_Plans-first-bg-wide.svg  \
    presnt/Blueprint_Plans/Pictures/Blueprint_Plans-second-bg-wide.svg  \
    presnt/Blueprint_Plans/Pictures/Blueprint_Plans-first-bg-4-3.svg  \
    presnt/Blueprint_Plans/Pictures/Blueprint_Plans-second-bg-4-3.svg  \
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
    presnt/Grey_Elegant/Pictures/1000DC810000A57800007C20AABC768B7E7ACB30.svg  \
    presnt/Grey_Elegant/Thumbnails/thumbnail.png \
    presnt/Growing_Liberty/Pictures/10000000000007800000043801760C0AF0E62E0B.jpg  \
    presnt/Growing_Liberty/Pictures/10000000000007800000043827B5F54D1F4F498F.jpg  \
    presnt/Growing_Liberty/Pictures/100002010000016A0000029374C9F2B8F26392AD.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F000000351DA39462E363D7ED3.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F10000029333ED1BC48076B5C4.png  \
    presnt/Growing_Liberty/Pictures/10000201000001F3000003548E51808DE171DA02.png  \
    presnt/Growing_Liberty/Pictures/10000C1C0000034200004A6A2C92F38BC117DD3C.svg  \
    presnt/Growing_Liberty/Pictures/10000C8F0000034200004A6ABD021DA9BD39254C.svg  \
    presnt/Growing_Liberty/Thumbnails/thumbnail.png \
    presnt/Inspiration/Thumbnails/thumbnail.png \
    presnt/Lights/Thumbnails/thumbnail.png \
    presnt/Metropolis/Pictures/100021B200008D2E00004F60B312A649B9EEC188.svg  \
    presnt/Metropolis/Thumbnails/thumbnail.png \
    presnt/Midnightblue/Thumbnails/thumbnail.png \
    presnt/Nature_Illustration/Thumbnails/thumbnail.png \
    presnt/Pencil/Thumbnails/thumbnail.png \
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
    wizard/desktop/html/settings.xml \
    wizard/desktop/idxexample/settings.xml \
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
