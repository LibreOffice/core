# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_autocorr,$(call gb_CustomTarget_get_workdir,extras/source/autocorr)))

$(eval $(call gb_Package_add_files,extras_autocorr,$(LIBO_SHARE_FOLDER)/autocorr,\
	acor_af-ZA.dat \
	acor_bg-BG.dat \
	acor_cs-CZ.dat \
	acor_ca-ES.dat \
	acor_da-DK.dat \
	acor_de.dat \
	acor_en-AU.dat \
	acor_en-GB.dat \
	acor_en-US.dat \
	acor_en-ZA.dat \
	acor_es.dat \
	acor_fa-IR.dat \
	acor_fi-FI.dat \
	acor_fr.dat \
	acor_ga-IE.dat \
	acor_hr-HR.dat \
	acor_hu-HU.dat \
	acor_is-IS.dat \
	acor_it.dat \
	acor_ja-JP.dat \
	acor_ko-KR.dat \
	acor_lb-LU.dat \
	acor_lt-LT.dat \
	acor_mn-MN.dat \
	acor_nl-BE.dat \
	acor_nl-NL.dat \
	acor_pl-PL.dat \
	acor_pt-BR.dat \
	acor_pt-PT.dat \
	acor_ro-RO.dat \
	acor_ru-RU.dat \
	acor_sk-SK.dat \
	acor_sl-SI.dat \
	acor_sr-CS.dat \
	acor_sr-Latn-CS.dat \
	acor_sr-Latn-ME.dat \
	acor_sr-Latn-RS.dat \
	acor_sr-ME.dat \
	acor_sr-RS.dat \
	acor_sv-SE.dat \
	acor_tr-TR.dat \
	acor_vi-VN.dat \
	acor_zh-CN.dat \
	acor_zh-TW.dat \
))

# vim: set noet sw=4 ts=4:
