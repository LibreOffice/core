# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,localedata_others))

$(eval $(call gb_Library_add_linked_libs,localedata_others,\
	localedata_en \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_others,\
	CustomTarget/i18npool/source/localedata/localedata_af_NA \
	CustomTarget/i18npool/source/localedata/localedata_af_ZA \
	CustomTarget/i18npool/source/localedata/localedata_ak_GH \
	CustomTarget/i18npool/source/localedata/localedata_am_ET \
	CustomTarget/i18npool/source/localedata/localedata_ar_DZ \
	CustomTarget/i18npool/source/localedata/localedata_ar_EG \
	CustomTarget/i18npool/source/localedata/localedata_ar_LB \
	CustomTarget/i18npool/source/localedata/localedata_ar_OM \
	CustomTarget/i18npool/source/localedata/localedata_ar_SA \
	CustomTarget/i18npool/source/localedata/localedata_ar_TN \
	CustomTarget/i18npool/source/localedata/localedata_az_AZ \
	CustomTarget/i18npool/source/localedata/localedata_bn_IN \
	CustomTarget/i18npool/source/localedata/localedata_bn_BD \
	CustomTarget/i18npool/source/localedata/localedata_dz_BT \
	CustomTarget/i18npool/source/localedata/localedata_ee_GH \
	CustomTarget/i18npool/source/localedata/localedata_en_IN \
	CustomTarget/i18npool/source/localedata/localedata_eo \
	CustomTarget/i18npool/source/localedata/localedata_fa_IR \
	CustomTarget/i18npool/source/localedata/localedata_gu_IN \
	CustomTarget/i18npool/source/localedata/localedata_gug_PY \
	CustomTarget/i18npool/source/localedata/localedata_haw_US \
	CustomTarget/i18npool/source/localedata/localedata_ha_GH \
	CustomTarget/i18npool/source/localedata/localedata_he_IL \
	CustomTarget/i18npool/source/localedata/localedata_hi_IN \
	CustomTarget/i18npool/source/localedata/localedata_hil_PH \
	CustomTarget/i18npool/source/localedata/localedata_ht_HT \
	CustomTarget/i18npool/source/localedata/localedata_hu_HU \
	CustomTarget/i18npool/source/localedata/localedata_hy_AM \
	CustomTarget/i18npool/source/localedata/localedata_ia \
	CustomTarget/i18npool/source/localedata/localedata_id_ID \
	CustomTarget/i18npool/source/localedata/localedata_ja_JP \
	CustomTarget/i18npool/source/localedata/localedata_jbo \
	CustomTarget/i18npool/source/localedata/localedata_kab_DZ \
	CustomTarget/i18npool/source/localedata/localedata_kk_KZ \
	CustomTarget/i18npool/source/localedata/localedata_km_KH \
	CustomTarget/i18npool/source/localedata/localedata_kn_IN \
	CustomTarget/i18npool/source/localedata/localedata_ko_KR \
	CustomTarget/i18npool/source/localedata/localedata_ku_TR \
	CustomTarget/i18npool/source/localedata/localedata_ky_KG \
	CustomTarget/i18npool/source/localedata/localedata_lg_UG \
	CustomTarget/i18npool/source/localedata/localedata_lif_NP \
	CustomTarget/i18npool/source/localedata/localedata_ln_CD \
	CustomTarget/i18npool/source/localedata/localedata_lo_LA \
	CustomTarget/i18npool/source/localedata/localedata_mai_IN \
	CustomTarget/i18npool/source/localedata/localedata_ml_IN \
	CustomTarget/i18npool/source/localedata/localedata_mn_MN \
	CustomTarget/i18npool/source/localedata/localedata_mr_IN \
	CustomTarget/i18npool/source/localedata/localedata_ms_MY \
	CustomTarget/i18npool/source/localedata/localedata_my_MM \
	CustomTarget/i18npool/source/localedata/localedata_ne_NP \
	CustomTarget/i18npool/source/localedata/localedata_nr_ZA \
	CustomTarget/i18npool/source/localedata/localedata_nso_ZA \
	CustomTarget/i18npool/source/localedata/localedata_om_ET \
	CustomTarget/i18npool/source/localedata/localedata_or_IN \
	CustomTarget/i18npool/source/localedata/localedata_pa_IN \
	CustomTarget/i18npool/source/localedata/localedata_plt_MG \
	CustomTarget/i18npool/source/localedata/localedata_pt_AO \
	CustomTarget/i18npool/source/localedata/localedata_rw_RW \
	CustomTarget/i18npool/source/localedata/localedata_sg_CF \
	CustomTarget/i18npool/source/localedata/localedata_shs_CA \
	CustomTarget/i18npool/source/localedata/localedata_so_SO \
	CustomTarget/i18npool/source/localedata/localedata_ss_ZA \
	CustomTarget/i18npool/source/localedata/localedata_st_ZA \
	CustomTarget/i18npool/source/localedata/localedata_sw_TZ \
	CustomTarget/i18npool/source/localedata/localedata_ta_IN \
	CustomTarget/i18npool/source/localedata/localedata_te_IN \
	CustomTarget/i18npool/source/localedata/localedata_tg_TJ \
	CustomTarget/i18npool/source/localedata/localedata_th_TH \
	CustomTarget/i18npool/source/localedata/localedata_ti_ER \
	CustomTarget/i18npool/source/localedata/localedata_tk_TM \
	CustomTarget/i18npool/source/localedata/localedata_tn_ZA \
	CustomTarget/i18npool/source/localedata/localedata_tpi_PG \
	CustomTarget/i18npool/source/localedata/localedata_ts_ZA \
	CustomTarget/i18npool/source/localedata/localedata_ug_CN \
	CustomTarget/i18npool/source/localedata/localedata_ur_PK \
	CustomTarget/i18npool/source/localedata/localedata_uz_UZ \
	CustomTarget/i18npool/source/localedata/localedata_ve_ZA \
	CustomTarget/i18npool/source/localedata/localedata_vi_VN \
	CustomTarget/i18npool/source/localedata/localedata_xh_ZA \
	CustomTarget/i18npool/source/localedata/localedata_yi_US \
	CustomTarget/i18npool/source/localedata/localedata_zh_CN \
	CustomTarget/i18npool/source/localedata/localedata_zh_HK \
	CustomTarget/i18npool/source/localedata/localedata_zh_MO \
	CustomTarget/i18npool/source/localedata/localedata_zh_SG \
	CustomTarget/i18npool/source/localedata/localedata_zh_TW \
	CustomTarget/i18npool/source/localedata/localedata_zu_ZA \
))

# vim: set noet sw=4 ts=4:
