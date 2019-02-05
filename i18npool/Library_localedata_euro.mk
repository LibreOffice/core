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



$(eval $(call gb_Library_Library,localedata_euro))

$(eval $(call gb_Library_set_include,localedata_euro,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,localedata_euro, \
#	offapi \
#	udkapi \
#))

#$(eval $(call gb_Library_add_defs,localedata_euro,\
#	-DI18NPOOL_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_set_versionmap,localedata_euro,$(SRCDIR)/i18npool/source/localedata/data/localedata_euro.map))

$(eval $(call gb_Library_add_linked_libs,localedata_euro,\
	localedata_en \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_euro,\
	CustomTarget/i18npool/source/localedata/data/localedata_ast_ES \
	CustomTarget/i18npool/source/localedata/data/localedata_be_BY \
	CustomTarget/i18npool/source/localedata/data/localedata_bg_BG \
	CustomTarget/i18npool/source/localedata/data/localedata_br_FR \
	CustomTarget/i18npool/source/localedata/data/localedata_bs_BA \
	CustomTarget/i18npool/source/localedata/data/localedata_ca_ES \
	CustomTarget/i18npool/source/localedata/data/localedata_cs_CZ \
	CustomTarget/i18npool/source/localedata/data/localedata_cv_RU \
	CustomTarget/i18npool/source/localedata/data/localedata_cy_GB \
	CustomTarget/i18npool/source/localedata/data/localedata_da_DK \
	CustomTarget/i18npool/source/localedata/data/localedata_de_AT \
	CustomTarget/i18npool/source/localedata/data/localedata_de_CH \
	CustomTarget/i18npool/source/localedata/data/localedata_de_DE \
	CustomTarget/i18npool/source/localedata/data/localedata_de_LI \
	CustomTarget/i18npool/source/localedata/data/localedata_de_LU \
	CustomTarget/i18npool/source/localedata/data/localedata_dsb_DE \
	CustomTarget/i18npool/source/localedata/data/localedata_el_GR \
	CustomTarget/i18npool/source/localedata/data/localedata_et_EE \
	CustomTarget/i18npool/source/localedata/data/localedata_eu \
	CustomTarget/i18npool/source/localedata/data/localedata_fi_FI \
	CustomTarget/i18npool/source/localedata/data/localedata_fo_FO \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_BE \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_CA \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_CH \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_FR \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_LU \
	CustomTarget/i18npool/source/localedata/data/localedata_fr_MC \
	CustomTarget/i18npool/source/localedata/data/localedata_fur_IT \
	CustomTarget/i18npool/source/localedata/data/localedata_fy_NL \
	CustomTarget/i18npool/source/localedata/data/localedata_ga_IE \
	CustomTarget/i18npool/source/localedata/data/localedata_gd_GB \
	CustomTarget/i18npool/source/localedata/data/localedata_gsc_FR \
	CustomTarget/i18npool/source/localedata/data/localedata_hr_HR \
	CustomTarget/i18npool/source/localedata/data/localedata_hsb_DE \
	CustomTarget/i18npool/source/localedata/data/localedata_is_IS \
	CustomTarget/i18npool/source/localedata/data/localedata_it_CH \
	CustomTarget/i18npool/source/localedata/data/localedata_it_IT \
	CustomTarget/i18npool/source/localedata/data/localedata_ka_GE \
	CustomTarget/i18npool/source/localedata/data/localedata_kl_GL \
	CustomTarget/i18npool/source/localedata/data/localedata_la_VA \
	CustomTarget/i18npool/source/localedata/data/localedata_lb_LU \
	CustomTarget/i18npool/source/localedata/data/localedata_lt_LT \
	CustomTarget/i18npool/source/localedata/data/localedata_ltg_LV \
	CustomTarget/i18npool/source/localedata/data/localedata_lv_LV \
	CustomTarget/i18npool/source/localedata/data/localedata_mk_MK \
	CustomTarget/i18npool/source/localedata/data/localedata_mt_MT \
	CustomTarget/i18npool/source/localedata/data/localedata_nb_NO \
	CustomTarget/i18npool/source/localedata/data/localedata_nds_DE \
	CustomTarget/i18npool/source/localedata/data/localedata_nl_BE \
	CustomTarget/i18npool/source/localedata/data/localedata_nl_NL \
	CustomTarget/i18npool/source/localedata/data/localedata_nn_NO \
	CustomTarget/i18npool/source/localedata/data/localedata_no_NO \
	CustomTarget/i18npool/source/localedata/data/localedata_oc_FR \
	CustomTarget/i18npool/source/localedata/data/localedata_pl_PL \
	CustomTarget/i18npool/source/localedata/data/localedata_pt_BR \
	CustomTarget/i18npool/source/localedata/data/localedata_pt_PT \
	CustomTarget/i18npool/source/localedata/data/localedata_ro_RO \
	CustomTarget/i18npool/source/localedata/data/localedata_ru_RU \
	CustomTarget/i18npool/source/localedata/data/localedata_rue_SK \
	CustomTarget/i18npool/source/localedata/data/localedata_sc_IT \
	CustomTarget/i18npool/source/localedata/data/localedata_sh_ME \
	CustomTarget/i18npool/source/localedata/data/localedata_sh_RS \
	CustomTarget/i18npool/source/localedata/data/localedata_sh_YU \
	CustomTarget/i18npool/source/localedata/data/localedata_sk_SK \
	CustomTarget/i18npool/source/localedata/data/localedata_sl_SI \
	CustomTarget/i18npool/source/localedata/data/localedata_sr_ME \
	CustomTarget/i18npool/source/localedata/data/localedata_sr_RS \
	CustomTarget/i18npool/source/localedata/data/localedata_sr_YU \
	CustomTarget/i18npool/source/localedata/data/localedata_sv_FI \
	CustomTarget/i18npool/source/localedata/data/localedata_sv_SE \
	CustomTarget/i18npool/source/localedata/data/localedata_tr_TR \
	CustomTarget/i18npool/source/localedata/data/localedata_tt_RU \
	CustomTarget/i18npool/source/localedata/data/localedata_uk_UA \
	CustomTarget/i18npool/source/localedata/data/localedata_wa_BE \
))


# vim: set noet sw=4 ts=4:

