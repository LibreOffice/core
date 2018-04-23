# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_notomore,font_notomore))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_notomore,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	NotoColorEmoji.ttf \
	NotoEmoji-Regular.ttf \
	NotoNastaliqUrdu-Regular.ttf \
	NotoSansAvestan-Regular.ttf \
	NotoSansBalinese-Regular.ttf \
	NotoSansBamum-Regular.ttf \
	NotoSansBatak-Regular.ttf \
	NotoSansBengali-Bold.ttf \
	NotoSansBengali-Regular.ttf \
	NotoSansBrahmi-Regular.ttf \
	NotoSansBuginese-Regular.ttf \
	NotoSansBuhid-Regular.ttf \
	NotoSansCanadianAboriginal-Regular.ttf \
	NotoSansCarian-Regular.ttf \
	NotoSansCham-Bold.ttf \
	NotoSansCham-Regular.ttf \
	NotoSansCherokee-Regular.ttf \
	NotoSansCoptic-Regular.ttf \
	NotoSansCuneiform-Regular.ttf \
	NotoSansCypriot-Regular.ttf \
	NotoSansDeseret-Regular.ttf \
	NotoSansDevanagari-Bold.ttf \
	NotoSansDevanagari-Regular.ttf \
	NotoSansEgyptianHieroglyphs-Regular.ttf \
	NotoSansEthiopic-Bold.ttf \
	NotoSansEthiopic-Regular.ttf \
	NotoSansGlagolitic-Regular.ttf \
	NotoSansGothic-Regular.ttf \
	NotoSansGujarati-Bold.ttf \
	NotoSansGujarati-Regular.ttf \
	NotoSansGurmukhi-Bold.ttf \
	NotoSansGurmukhi-Regular.ttf \
	NotoSansHanunoo-Regular.ttf \
	NotoSansImperialAramaic-Regular.ttf \
	NotoSansInscriptionalPahlavi-Regular.ttf \
	NotoSansInscriptionalParthian-Regular.ttf \
	NotoSansJavanese-Regular.ttf \
	NotoSansKaithi-Regular.ttf \
	NotoSansKannada-Bold.ttf \
	NotoSansKannada-Regular.ttf \
	NotoSansKayahLi-Regular.ttf \
	NotoSansKharoshthi-Regular.ttf \
	NotoSansKhmer-Bold.ttf \
	NotoSansKhmer-Regular.ttf \
	NotoSansLepcha-Regular.ttf \
	NotoSansLimbu-Regular.ttf \
	NotoSansLinearB-Regular.ttf \
	NotoSansLycian-Regular.ttf \
	NotoSansLydian-Regular.ttf \
	NotoSansMalayalam-Bold.ttf \
	NotoSansMalayalam-Regular.ttf \
	NotoSansMandaic-Regular.ttf \
	NotoSansMeeteiMayek-Regular.ttf \
	NotoSansMongolian-Regular.ttf \
	NotoSansMyanmar-Bold.ttf \
	NotoSansMyanmar-Regular.ttf \
	NotoSansNewTaiLue-Regular.ttf \
	NotoSansNKo-Regular.ttf \
	NotoSansOgham-Regular.ttf \
	NotoSansOlChiki-Regular.ttf \
	NotoSansOldItalic-Regular.ttf \
	NotoSansOldPersian-Regular.ttf \
	NotoSansOldSouthArabian-Regular.ttf \
	NotoSansOldTurkic-Regular.ttf \
	NotoSansOriya-Bold.ttf \
	NotoSansOriya-Regular.ttf \
	NotoSansOsmanya-Regular.ttf \
	NotoSansPhagsPa-Regular.ttf \
	NotoSansPhoenician-Regular.ttf \
	NotoSansRejang-Regular.ttf \
	NotoSansRunic-Regular.ttf \
	NotoSansSamaritan-Regular.ttf \
	NotoSansSaurashtra-Regular.ttf \
	NotoSansShavian-Regular.ttf \
	NotoSansSinhala-Bold.ttf \
	NotoSansSinhala-Regular.ttf \
	NotoSansSundanese-Regular.ttf \
	NotoSansSylotiNagri-Regular.ttf \
	NotoSansSymbols-Regular.ttf \
	NotoSansSyriacEastern-Regular.ttf \
	NotoSansSyriacEstrangela-Regular.ttf \
	NotoSansSyriacWestern-Regular.ttf \
	NotoSansTagalog-Regular.ttf \
	NotoSansTagbanwa-Regular.ttf \
	NotoSansTaiLe-Regular.ttf \
	NotoSansTaiTham-Regular.ttf \
	NotoSansTaiViet-Regular.ttf \
	NotoSansTamil-Bold.ttf \
	NotoSansTamil-Regular.ttf \
	NotoSansTelugu-Bold.ttf \
	NotoSansTelugu-Regular.ttf \
	NotoSansThaana-Bold.ttf \
	NotoSansThaana-Regular.ttf \
	NotoSansThai-Bold.ttf \
	NotoSansThai-Regular.ttf \
	NotoSansTibetan-Bold.ttf \
	NotoSansTibetan-Regular.ttf \
	NotoSansTifinagh-Regular.ttf \
	NotoSansUgaritic-Regular.ttf \
	NotoSansVai-Regular.ttf \
	NotoSansYi-Regular.ttf \
	NotoSerifBengali-Bold.ttf \
	NotoSerifBengali-Regular.ttf \
	NotoSerifDevanagari-Bold.ttf \
	NotoSerifDevanagari-Regular.ttf \
	NotoSerifGujarati-Bold.ttf \
	NotoSerifGujarati-Regular.ttf \
	NotoSerifKannada-Bold.ttf \
	NotoSerifKannada-Regular.ttf \
	NotoSerifKhmer-Bold.ttf \
	NotoSerifKhmer-Regular.ttf \
	NotoSerifMalayalam-Bold.ttf \
	NotoSerifMalayalam-Regular.ttf \
	NotoSerifTamil-Bold.ttf \
	NotoSerifTamil-Regular.ttf \
	NotoSerifTelugu-Bold.ttf \
	NotoSerifTelugu-Regular.ttf \
	NotoSerifThai-Bold.ttf \
	NotoSerifThai-Regular.ttf \
))

# vim: set noet sw=4 ts=4:
