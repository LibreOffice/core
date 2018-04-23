# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notomore))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notomore,$(FONT_NOTO_MORE_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notosansjp))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notosansjp,$(FONT_NOTO_SANS_JP_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notosanskr))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notosanskr,$(FONT_NOTO_SANS_KR_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notosanssc))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notosanssc,$(FONT_NOTO_SANS_SC_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notosanstc))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notosanstc,$(FONT_NOTO_SANS_TC_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notoserifjp))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notoserifjp,$(FONT_NOTO_SERIF_JP_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notoserifkr))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notoserifkr,$(FONT_NOTO_SERIF_KR_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notoserifsc))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notoserifsc,$(FONT_NOTO_SERIF_SC_TARBALL),0))

$(eval $(call gb_UnpackedTarball_UnpackedTarball,font_notoseriftc))

$(eval $(call gb_UnpackedTarball_set_tarball,font_notoseriftc,$(FONT_NOTO_SERIF_TC_TARBALL),0))

# vim: set noet sw=4 ts=4:
