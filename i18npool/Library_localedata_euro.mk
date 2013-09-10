# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,localedata_euro))

$(eval $(call gb_Library_use_libraries,localedata_euro,\
	localedata_en \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_euro,\
	CustomTarget/i18npool/localedata/localedata_an_ES \
	CustomTarget/i18npool/localedata/localedata_ast_ES \
	CustomTarget/i18npool/localedata/localedata_be_BY \
	CustomTarget/i18npool/localedata/localedata_bg_BG \
	CustomTarget/i18npool/localedata/localedata_br_FR \
	CustomTarget/i18npool/localedata/localedata_bs_BA \
	CustomTarget/i18npool/localedata/localedata_ca_ES \
	CustomTarget/i18npool/localedata/localedata_cs_CZ \
	CustomTarget/i18npool/localedata/localedata_cv_RU \
	CustomTarget/i18npool/localedata/localedata_cy_GB \
	CustomTarget/i18npool/localedata/localedata_da_DK \
	CustomTarget/i18npool/localedata/localedata_de_AT \
	CustomTarget/i18npool/localedata/localedata_de_CH \
	CustomTarget/i18npool/localedata/localedata_de_DE \
	CustomTarget/i18npool/localedata/localedata_de_LI \
	CustomTarget/i18npool/localedata/localedata_de_LU \
	CustomTarget/i18npool/localedata/localedata_dsb_DE \
	CustomTarget/i18npool/localedata/localedata_el_GR \
	CustomTarget/i18npool/localedata/localedata_et_EE \
	CustomTarget/i18npool/localedata/localedata_eu \
	CustomTarget/i18npool/localedata/localedata_fi_FI \
	CustomTarget/i18npool/localedata/localedata_fo_FO \
	CustomTarget/i18npool/localedata/localedata_fr_BE \
	CustomTarget/i18npool/localedata/localedata_fr_CA \
	CustomTarget/i18npool/localedata/localedata_fr_CH \
	CustomTarget/i18npool/localedata/localedata_fr_FR \
	CustomTarget/i18npool/localedata/localedata_fr_LU \
	CustomTarget/i18npool/localedata/localedata_fr_MC \
	CustomTarget/i18npool/localedata/localedata_fur_IT \
	CustomTarget/i18npool/localedata/localedata_fy_NL \
	CustomTarget/i18npool/localedata/localedata_ga_IE \
	CustomTarget/i18npool/localedata/localedata_gd_GB \
	CustomTarget/i18npool/localedata/localedata_gsc_FR \
	CustomTarget/i18npool/localedata/localedata_hr_HR \
	CustomTarget/i18npool/localedata/localedata_hsb_DE \
	CustomTarget/i18npool/localedata/localedata_is_IS \
	CustomTarget/i18npool/localedata/localedata_it_CH \
	CustomTarget/i18npool/localedata/localedata_it_IT \
	CustomTarget/i18npool/localedata/localedata_ka_GE \
	CustomTarget/i18npool/localedata/localedata_kl_GL \
	CustomTarget/i18npool/localedata/localedata_la_VA \
	CustomTarget/i18npool/localedata/localedata_lb_LU \
	CustomTarget/i18npool/localedata/localedata_lt_LT \
	CustomTarget/i18npool/localedata/localedata_ltg_LV \
	CustomTarget/i18npool/localedata/localedata_lv_LV \
	CustomTarget/i18npool/localedata/localedata_mk_MK \
	CustomTarget/i18npool/localedata/localedata_mt_MT \
	CustomTarget/i18npool/localedata/localedata_myv_RU \
	CustomTarget/i18npool/localedata/localedata_nb_NO \
	CustomTarget/i18npool/localedata/localedata_nl_BE \
	CustomTarget/i18npool/localedata/localedata_nl_NL \
	CustomTarget/i18npool/localedata/localedata_nn_NO \
	CustomTarget/i18npool/localedata/localedata_no_NO \
	CustomTarget/i18npool/localedata/localedata_oc_FR \
	CustomTarget/i18npool/localedata/localedata_pl_PL \
	CustomTarget/i18npool/localedata/localedata_pt_BR \
	CustomTarget/i18npool/localedata/localedata_pt_PT \
	CustomTarget/i18npool/localedata/localedata_ro_RO \
	CustomTarget/i18npool/localedata/localedata_rue_SK \
	CustomTarget/i18npool/localedata/localedata_ru_RU \
	CustomTarget/i18npool/localedata/localedata_sc_IT \
	CustomTarget/i18npool/localedata/localedata_sk_SK \
	CustomTarget/i18npool/localedata/localedata_sl_SI \
	CustomTarget/i18npool/localedata/localedata_sr_Latn_ME \
	CustomTarget/i18npool/localedata/localedata_sr_Latn_RS \
	CustomTarget/i18npool/localedata/localedata_sr_Latn_YU \
	CustomTarget/i18npool/localedata/localedata_sr_ME \
	CustomTarget/i18npool/localedata/localedata_sr_RS \
	CustomTarget/i18npool/localedata/localedata_sr_YU \
	CustomTarget/i18npool/localedata/localedata_sv_FI \
	CustomTarget/i18npool/localedata/localedata_sv_SE \
	CustomTarget/i18npool/localedata/localedata_tr_TR \
	CustomTarget/i18npool/localedata/localedata_uk_UA \
	CustomTarget/i18npool/localedata/localedata_wa_BE \
))

# vim: set noet sw=4 ts=4:
