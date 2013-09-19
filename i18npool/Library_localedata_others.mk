# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,localedata_others))

$(eval $(call gb_Library_use_libraries,localedata_others,\
	localedata_en \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_others,\
	CustomTarget/i18npool/localedata/localedata_af_NA \
	CustomTarget/i18npool/localedata/localedata_af_ZA \
	CustomTarget/i18npool/localedata/localedata_ak_GH \
	CustomTarget/i18npool/localedata/localedata_am_ET \
	CustomTarget/i18npool/localedata/localedata_ar_DZ \
	CustomTarget/i18npool/localedata/localedata_ar_EG \
	CustomTarget/i18npool/localedata/localedata_ar_LB \
	CustomTarget/i18npool/localedata/localedata_ar_OM \
	CustomTarget/i18npool/localedata/localedata_ar_SA \
	CustomTarget/i18npool/localedata/localedata_ar_TN \
	CustomTarget/i18npool/localedata/localedata_axk_CG \
	CustomTarget/i18npool/localedata/localedata_az_AZ \
	CustomTarget/i18npool/localedata/localedata_beq_CG \
	CustomTarget/i18npool/localedata/localedata_bkw_CG \
	CustomTarget/i18npool/localedata/localedata_bn_IN \
	CustomTarget/i18npool/localedata/localedata_bn_BD \
	CustomTarget/i18npool/localedata/localedata_bo_CN \
	CustomTarget/i18npool/localedata/localedata_bo_IN \
	CustomTarget/i18npool/localedata/localedata_bvx_CG \
	CustomTarget/i18npool/localedata/localedata_dde_CG \
	CustomTarget/i18npool/localedata/localedata_dz_BT \
	CustomTarget/i18npool/localedata/localedata_ebo_CG \
	CustomTarget/i18npool/localedata/localedata_ee_GH \
	CustomTarget/i18npool/localedata/localedata_en_IN \
	CustomTarget/i18npool/localedata/localedata_eo \
	CustomTarget/i18npool/localedata/localedata_fa_IR \
	CustomTarget/i18npool/localedata/localedata_gu_IN \
	CustomTarget/i18npool/localedata/localedata_gug_PY \
	CustomTarget/i18npool/localedata/localedata_haw_US \
	CustomTarget/i18npool/localedata/localedata_ha_GH \
	CustomTarget/i18npool/localedata/localedata_he_IL \
	CustomTarget/i18npool/localedata/localedata_hi_IN \
	CustomTarget/i18npool/localedata/localedata_hil_PH \
	CustomTarget/i18npool/localedata/localedata_ht_HT \
	CustomTarget/i18npool/localedata/localedata_hu_HU \
	CustomTarget/i18npool/localedata/localedata_hy_AM \
	CustomTarget/i18npool/localedata/localedata_ia \
	CustomTarget/i18npool/localedata/localedata_id_ID \
	CustomTarget/i18npool/localedata/localedata_iyx_CG \
	CustomTarget/i18npool/localedata/localedata_ja_JP \
	CustomTarget/i18npool/localedata/localedata_jbo \
	CustomTarget/i18npool/localedata/localedata_kab_DZ \
	CustomTarget/i18npool/localedata/localedata_kk_KZ \
	CustomTarget/i18npool/localedata/localedata_kkw_CG \
	CustomTarget/i18npool/localedata/localedata_km_KH \
	CustomTarget/i18npool/localedata/localedata_kn_IN \
	CustomTarget/i18npool/localedata/localedata_kng_CG \
	CustomTarget/i18npool/localedata/localedata_ko_KR \
	CustomTarget/i18npool/localedata/localedata_ku_TR \
	CustomTarget/i18npool/localedata/localedata_ky_KG \
	CustomTarget/i18npool/localedata/localedata_ldi_CG \
	CustomTarget/i18npool/localedata/localedata_lg_UG \
	CustomTarget/i18npool/localedata/localedata_lif_NP \
	CustomTarget/i18npool/localedata/localedata_ln_CD \
	CustomTarget/i18npool/localedata/localedata_lo_LA \
	CustomTarget/i18npool/localedata/localedata_mai_IN \
	CustomTarget/i18npool/localedata/localedata_mdw_CG \
	CustomTarget/i18npool/localedata/localedata_mkw_CG \
	CustomTarget/i18npool/localedata/localedata_ml_IN \
	CustomTarget/i18npool/localedata/localedata_mn_Cyrl_MN \
	CustomTarget/i18npool/localedata/localedata_mr_IN \
	CustomTarget/i18npool/localedata/localedata_ms_MY \
	CustomTarget/i18npool/localedata/localedata_my_MM \
	CustomTarget/i18npool/localedata/localedata_ne_NP \
	CustomTarget/i18npool/localedata/localedata_ngz_CG \
	CustomTarget/i18npool/localedata/localedata_njx_CG \
	CustomTarget/i18npool/localedata/localedata_njy_CG \
	CustomTarget/i18npool/localedata/localedata_nr_ZA \
	CustomTarget/i18npool/localedata/localedata_nso_ZA \
	CustomTarget/i18npool/localedata/localedata_om_ET \
	CustomTarget/i18npool/localedata/localedata_or_IN \
	CustomTarget/i18npool/localedata/localedata_pa_IN \
	CustomTarget/i18npool/localedata/localedata_pap_CW \
	CustomTarget/i18npool/localedata/localedata_plt_MG \
	CustomTarget/i18npool/localedata/localedata_pjt_AU \
	CustomTarget/i18npool/localedata/localedata_pt_AO \
	CustomTarget/i18npool/localedata/localedata_puu_CG \
	CustomTarget/i18npool/localedata/localedata_rw_RW \
	CustomTarget/i18npool/localedata/localedata_sdj_CG \
	CustomTarget/i18npool/localedata/localedata_sg_CF \
	CustomTarget/i18npool/localedata/localedata_shs_CA \
	CustomTarget/i18npool/localedata/localedata_sid_ET \
	CustomTarget/i18npool/localedata/localedata_so_SO \
	CustomTarget/i18npool/localedata/localedata_ss_ZA \
	CustomTarget/i18npool/localedata/localedata_st_ZA \
	CustomTarget/i18npool/localedata/localedata_sw_TZ \
	CustomTarget/i18npool/localedata/localedata_ta_IN \
	CustomTarget/i18npool/localedata/localedata_te_IN \
	CustomTarget/i18npool/localedata/localedata_tek_CG \
	CustomTarget/i18npool/localedata/localedata_tg_TJ \
	CustomTarget/i18npool/localedata/localedata_th_TH \
	CustomTarget/i18npool/localedata/localedata_ti_ER \
	CustomTarget/i18npool/localedata/localedata_tk_TM \
	CustomTarget/i18npool/localedata/localedata_tn_ZA \
	CustomTarget/i18npool/localedata/localedata_tpi_PG \
	CustomTarget/i18npool/localedata/localedata_tyx_CG \
	CustomTarget/i18npool/localedata/localedata_ts_ZA \
	CustomTarget/i18npool/localedata/localedata_tsa_CG \
	CustomTarget/i18npool/localedata/localedata_ug_CN \
	CustomTarget/i18npool/localedata/localedata_ur_PK \
	CustomTarget/i18npool/localedata/localedata_uz_UZ \
	CustomTarget/i18npool/localedata/localedata_ve_ZA \
	CustomTarget/i18npool/localedata/localedata_vi_VN \
	CustomTarget/i18npool/localedata/localedata_vif_CG \
	CustomTarget/i18npool/localedata/localedata_xh_ZA \
	CustomTarget/i18npool/localedata/localedata_xku_CG \
	CustomTarget/i18npool/localedata/localedata_yi_US \
	CustomTarget/i18npool/localedata/localedata_yom_CG \
	CustomTarget/i18npool/localedata/localedata_zh_CN \
	CustomTarget/i18npool/localedata/localedata_zh_HK \
	CustomTarget/i18npool/localedata/localedata_zh_MO \
	CustomTarget/i18npool/localedata/localedata_zh_SG \
	CustomTarget/i18npool/localedata/localedata_zh_TW \
	CustomTarget/i18npool/localedata/localedata_zu_ZA \
))

# vim: set noet sw=4 ts=4:
