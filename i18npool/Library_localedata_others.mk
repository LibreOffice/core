#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,localedata_others))

$(eval $(call gb_Library_set_include,localedata_others,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,localedata_others, \
#	offapi \
#	udkapi \
#))

#$(eval $(call gb_Library_add_defs,localedata_others,\
#	-DI18NPOOL_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_set_versionmap,localedata_others,$(SRCDIR)/i18npool/source/localedata/data/localedata_others.map))

$(eval $(call gb_Library_add_linked_libs,localedata_others,\
	localedata_en \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_others,\
	CustomTarget/i18npool/source/localedata/data/localedata_af_NA \
	CustomTarget/i18npool/source/localedata/data/localedata_af_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_ak_GH \
	CustomTarget/i18npool/source/localedata/data/localedata_am_ET \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_DZ \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_EG \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_LB \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_OM \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_SA \
	CustomTarget/i18npool/source/localedata/data/localedata_ar_TN \
	CustomTarget/i18npool/source/localedata/data/localedata_az_AZ \
	CustomTarget/i18npool/source/localedata/data/localedata_bn_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_bn_BD \
	CustomTarget/i18npool/source/localedata/data/localedata_dz_BT \
	CustomTarget/i18npool/source/localedata/data/localedata_ee_GH \
	CustomTarget/i18npool/source/localedata/data/localedata_en_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_eo \
	CustomTarget/i18npool/source/localedata/data/localedata_fa_IR \
	CustomTarget/i18npool/source/localedata/data/localedata_gu_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_gug_PY \
	CustomTarget/i18npool/source/localedata/data/localedata_haw_US \
	CustomTarget/i18npool/source/localedata/data/localedata_ha_GH \
	CustomTarget/i18npool/source/localedata/data/localedata_he_IL \
	CustomTarget/i18npool/source/localedata/data/localedata_hi_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_hil_PH \
	CustomTarget/i18npool/source/localedata/data/localedata_ht_HT \
	CustomTarget/i18npool/source/localedata/data/localedata_hu_HU \
	CustomTarget/i18npool/source/localedata/data/localedata_hy_AM \
	CustomTarget/i18npool/source/localedata/data/localedata_ia \
	CustomTarget/i18npool/source/localedata/data/localedata_id_ID \
	CustomTarget/i18npool/source/localedata/data/localedata_ja_JP \
	CustomTarget/i18npool/source/localedata/data/localedata_jbo \
	CustomTarget/i18npool/source/localedata/data/localedata_kab_DZ \
	CustomTarget/i18npool/source/localedata/data/localedata_kk_KZ \
	CustomTarget/i18npool/source/localedata/data/localedata_km_KH \
	CustomTarget/i18npool/source/localedata/data/localedata_kn_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_ko_KR \
	CustomTarget/i18npool/source/localedata/data/localedata_ku_TR \
	CustomTarget/i18npool/source/localedata/data/localedata_ky_KG \
	CustomTarget/i18npool/source/localedata/data/localedata_lg_UG \
	CustomTarget/i18npool/source/localedata/data/localedata_lif_NP \
	CustomTarget/i18npool/source/localedata/data/localedata_ln_CD \
	CustomTarget/i18npool/source/localedata/data/localedata_lo_LA \
	CustomTarget/i18npool/source/localedata/data/localedata_mai_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_ml_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_mn_MN \
	CustomTarget/i18npool/source/localedata/data/localedata_mr_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_ms_MY \
	CustomTarget/i18npool/source/localedata/data/localedata_my_MM \
	CustomTarget/i18npool/source/localedata/data/localedata_ne_NP \
	CustomTarget/i18npool/source/localedata/data/localedata_nr_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_nso_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_om_ET \
	CustomTarget/i18npool/source/localedata/data/localedata_or_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_pa_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_plt_MG \
	CustomTarget/i18npool/source/localedata/data/localedata_rw_RW \
	CustomTarget/i18npool/source/localedata/data/localedata_sg_CF \
	CustomTarget/i18npool/source/localedata/data/localedata_shs_CA \
	CustomTarget/i18npool/source/localedata/data/localedata_so_SO \
	CustomTarget/i18npool/source/localedata/data/localedata_ss_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_st_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_sw_TZ \
	CustomTarget/i18npool/source/localedata/data/localedata_ta_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_te_IN \
	CustomTarget/i18npool/source/localedata/data/localedata_tg_TJ \
	CustomTarget/i18npool/source/localedata/data/localedata_th_TH \
	CustomTarget/i18npool/source/localedata/data/localedata_ti_ER \
	CustomTarget/i18npool/source/localedata/data/localedata_tk_TM \
	CustomTarget/i18npool/source/localedata/data/localedata_tn_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_tpi_PG \
	CustomTarget/i18npool/source/localedata/data/localedata_ts_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_ug_CN \
	CustomTarget/i18npool/source/localedata/data/localedata_ur_PK \
	CustomTarget/i18npool/source/localedata/data/localedata_uz_UZ \
	CustomTarget/i18npool/source/localedata/data/localedata_ve_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_vi_VN \
	CustomTarget/i18npool/source/localedata/data/localedata_xh_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_yi_US \
	CustomTarget/i18npool/source/localedata/data/localedata_zh_CN \
	CustomTarget/i18npool/source/localedata/data/localedata_zh_HK \
	CustomTarget/i18npool/source/localedata/data/localedata_zh_MO \
	CustomTarget/i18npool/source/localedata/data/localedata_zh_SG \
	CustomTarget/i18npool/source/localedata/data/localedata_zh_TW \
	CustomTarget/i18npool/source/localedata/data/localedata_zu_ZA \
))


# vim: set noet sw=4 ts=4:

