#*************************************************************************
#*
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#************************************************************************

PRJ=..$/..$/..

PRJNAME=i18npool
TARGET=localedata
.IF "$(GUI)" == "OS2"
TARGET=ld
.ENDIF
LIBTARGET=NO

# Disable debugging on MSC compilers, due linker bug
.IF "$(COM)"=="MSC"
debug!=
.ENDIF

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

my_components = sax.inbuild

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# macro to link with localedata_en library
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
LINK_LOCALEDATA_EN_LIB=-l$(SHL1TARGET)
LINK_LOCALEDATA_ES_LIB=-l$(SHL2TARGET)
.ELSE
LINK_LOCALEDATA_EN_LIB=$(LB)$/i$(SHL1TARGET).lib
LINK_LOCALEDATA_ES_LIB=$(LB)$/i$(SHL2TARGET).lib
.ENDIF
.ELIF "$(GUI)" == "OS2"
LINK_LOCALEDATA_EN_LIB=$(LB)$/i$(SHL1TARGET).lib
LINK_LOCALEDATA_ES_LIB=$(LB)$/i$(SHL2TARGET).lib
.ELSE
LINK_LOCALEDATA_EN_LIB=-l$(SHL1TARGET)
LINK_LOCALEDATA_ES_LIB=-l$(SHL2TARGET)
.ENDIF

# --- Files --------------------------------------------------------

# English Locales
SHL1TARGET=localedata_en
.IF "$(GUI)" == "OS2"
SHL1TARGET=ld_en
.ENDIF
SHL1IMPLIB=i$(SHL1TARGET)

SHL1VERSIONMAP=localedata_en.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1OBJS= \
    $(SLO)$/localedata_en_AU.obj \
    $(SLO)$/localedata_en_BZ.obj \
    $(SLO)$/localedata_en_CA.obj \
    $(SLO)$/localedata_en_GB.obj \
    $(SLO)$/localedata_en_GH.obj \
    $(SLO)$/localedata_en_IE.obj \
    $(SLO)$/localedata_en_JM.obj \
    $(SLO)$/localedata_en_NA.obj \
    $(SLO)$/localedata_en_NZ.obj \
    $(SLO)$/localedata_en_PH.obj \
    $(SLO)$/localedata_en_TT.obj \
    $(SLO)$/localedata_en_US.obj \
    $(SLO)$/localedata_en_ZA.obj \
    $(SLO)$/localedata_en_ZW.obj

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# Spanish Locales
SHL2TARGET=localedata_es
.IF "$(GUI)" == "OS2"
SHL2TARGET=ld_es
.ENDIF
SHL2IMPLIB=i$(SHL2TARGET)

SHL2VERSIONMAP=localedata_es.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)

SHL2STDLIBS=	$(LINK_LOCALEDATA_EN_LIB)
SHL2DEPN=$(SHL1TARGETN)
SHL2OBJS= \
    $(SLO)$/localedata_es_AR.obj \
    $(SLO)$/localedata_es_BO.obj \
    $(SLO)$/localedata_es_CL.obj \
    $(SLO)$/localedata_es_CO.obj \
    $(SLO)$/localedata_es_CR.obj \
    $(SLO)$/localedata_es_DO.obj \
    $(SLO)$/localedata_es_EC.obj \
    $(SLO)$/localedata_es_ES.obj \
    $(SLO)$/localedata_es_GT.obj \
    $(SLO)$/localedata_es_HN.obj \
    $(SLO)$/localedata_es_MX.obj \
    $(SLO)$/localedata_es_NI.obj \
    $(SLO)$/localedata_es_PA.obj \
    $(SLO)$/localedata_es_PE.obj \
    $(SLO)$/localedata_es_PR.obj \
    $(SLO)$/localedata_es_PY.obj \
    $(SLO)$/localedata_es_SV.obj \
    $(SLO)$/localedata_es_UY.obj \
    $(SLO)$/localedata_es_VE.obj \
    $(SLO)$/localedata_gl_ES.obj

LIB2TARGET=	$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=$(SHL2OBJS)

# European Locales
SHL3TARGET=localedata_euro
.IF "$(GUI)" == "OS2"
SHL3TARGET=ld_eur
.ENDIF
SHL3IMPLIB=i$(SHL3TARGET)

SHL3VERSIONMAP=localedata_euro.map
SHL3DEF=$(MISC)$/$(SHL3TARGET).def
DEF3NAME=$(SHL3TARGET)

SHL3STDLIBS=	$(LINK_LOCALEDATA_EN_LIB)
SHL3DEPN=$(SHL1TARGETN) $(SHL2TARGETN)
SHL3OBJS=	\
    $(SLO)$/localedata_ast_ES.obj	\
    $(SLO)$/localedata_be_BY.obj	\
    $(SLO)$/localedata_bg_BG.obj	\
    $(SLO)$/localedata_br_FR.obj	\
    $(SLO)$/localedata_bs_BA.obj	\
    $(SLO)$/localedata_ca_ES.obj	\
    $(SLO)$/localedata_cs_CZ.obj	\
    $(SLO)$/localedata_cv_RU.obj	\
    $(SLO)$/localedata_cy_GB.obj	\
    $(SLO)$/localedata_da_DK.obj	\
    $(SLO)$/localedata_de_AT.obj	\
    $(SLO)$/localedata_de_CH.obj	\
    $(SLO)$/localedata_de_DE.obj	\
    $(SLO)$/localedata_de_LI.obj	\
    $(SLO)$/localedata_de_LU.obj	\
    $(SLO)$/localedata_dsb_DE.obj	\
    $(SLO)$/localedata_el_GR.obj	\
    $(SLO)$/localedata_et_EE.obj	\
    $(SLO)$/localedata_eu.obj	\
    $(SLO)$/localedata_fi_FI.obj	\
    $(SLO)$/localedata_fo_FO.obj	\
    $(SLO)$/localedata_fr_BE.obj	\
    $(SLO)$/localedata_fr_CA.obj	\
    $(SLO)$/localedata_fr_CH.obj	\
    $(SLO)$/localedata_fr_FR.obj	\
    $(SLO)$/localedata_fr_LU.obj	\
    $(SLO)$/localedata_fr_MC.obj	\
    $(SLO)$/localedata_fur_IT.obj	\
    $(SLO)$/localedata_fy_NL.obj	\
    $(SLO)$/localedata_ga_IE.obj	\
    $(SLO)$/localedata_gsc_FR.obj	\
    $(SLO)$/localedata_hr_HR.obj	\
    $(SLO)$/localedata_hsb_DE.obj	\
    $(SLO)$/localedata_is_IS.obj	\
    $(SLO)$/localedata_it_CH.obj	\
    $(SLO)$/localedata_it_IT.obj	\
    $(SLO)$/localedata_ka_GE.obj	\
    $(SLO)$/localedata_kl_GL.obj	\
    $(SLO)$/localedata_la_VA.obj	\
    $(SLO)$/localedata_lb_LU.obj	\
    $(SLO)$/localedata_lt_LT.obj	\
    $(SLO)$/localedata_ltg_LV.obj	\
    $(SLO)$/localedata_lv_LV.obj	\
    $(SLO)$/localedata_mk_MK.obj	\
    $(SLO)$/localedata_mt_MT.obj	\
    $(SLO)$/localedata_nb_NO.obj	\
    $(SLO)$/localedata_nl_BE.obj	\
    $(SLO)$/localedata_nl_NL.obj	\
    $(SLO)$/localedata_nn_NO.obj	\
    $(SLO)$/localedata_no_NO.obj	\
    $(SLO)$/localedata_oc_FR.obj	\
    $(SLO)$/localedata_pl_PL.obj	\
    $(SLO)$/localedata_pt_BR.obj	\
    $(SLO)$/localedata_pt_PT.obj	\
    $(SLO)$/localedata_ro_RO.obj	\
    $(SLO)$/localedata_rue_SK.obj	\
    $(SLO)$/localedata_ru_RU.obj	\
    $(SLO)$/localedata_sc_IT.obj	\
    $(SLO)$/localedata_sh_ME.obj	\
    $(SLO)$/localedata_sh_RS.obj	\
    $(SLO)$/localedata_sh_YU.obj	\
    $(SLO)$/localedata_sk_SK.obj	\
    $(SLO)$/localedata_sl_SI.obj	\
    $(SLO)$/localedata_sr_ME.obj	\
    $(SLO)$/localedata_sr_RS.obj	\
    $(SLO)$/localedata_sr_YU.obj	\
    $(SLO)$/localedata_sv_FI.obj	\
    $(SLO)$/localedata_sv_SE.obj	\
    $(SLO)$/localedata_tr_TR.obj	\
    $(SLO)$/localedata_uk_UA.obj	\
    $(SLO)$/localedata_wa_BE.obj

LIB3TARGET=	$(SLB)$/$(SHL3TARGET).lib
LIB3OBJFILES=$(SHL3OBJS)

# Other Locales
SHL4TARGET=localedata_others
.IF "$(GUI)" == "OS2"
SHL4TARGET=ld_oth
.ENDIF
SHL4IMPLIB=i$(SHL4TARGET)

SHL4VERSIONMAP=localedata_others.map
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)

SHL4STDLIBS=	$(LINK_LOCALEDATA_EN_LIB)
SHL4DEPN=$(SHL1TARGETN)
SHL4OBJS= \
    $(SLO)$/localedata_af_NA.obj \
    $(SLO)$/localedata_af_ZA.obj \
    $(SLO)$/localedata_ak_GH.obj \
    $(SLO)$/localedata_am_ET.obj \
    $(SLO)$/localedata_ar_DZ.obj \
    $(SLO)$/localedata_ar_EG.obj \
    $(SLO)$/localedata_ar_LB.obj \
    $(SLO)$/localedata_ar_OM.obj \
    $(SLO)$/localedata_ar_SA.obj \
    $(SLO)$/localedata_ar_TN.obj \
    $(SLO)$/localedata_az_AZ.obj \
    $(SLO)$/localedata_bn_IN.obj \
    $(SLO)$/localedata_bn_BD.obj \
    $(SLO)$/localedata_dz_BT.obj \
    $(SLO)$/localedata_ee_GH.obj \
    $(SLO)$/localedata_en_IN.obj \
    $(SLO)$/localedata_eo.obj    \
    $(SLO)$/localedata_fa_IR.obj \
    $(SLO)$/localedata_gu_IN.obj \
    $(SLO)$/localedata_gug_PY.obj \
    $(SLO)$/localedata_haw_US.obj \
    $(SLO)$/localedata_ha_GH.obj \
    $(SLO)$/localedata_he_IL.obj \
    $(SLO)$/localedata_hi_IN.obj \
    $(SLO)$/localedata_hil_PH.obj \
    $(SLO)$/localedata_ht_HT.obj \
    $(SLO)$/localedata_hu_HU.obj \
    $(SLO)$/localedata_hy_AM.obj \
    $(SLO)$/localedata_ia.obj    \
    $(SLO)$/localedata_id_ID.obj \
    $(SLO)$/localedata_ja_JP.obj \
    $(SLO)$/localedata_jbo.obj \
    $(SLO)$/localedata_kk_KZ.obj \
    $(SLO)$/localedata_km_KH.obj \
    $(SLO)$/localedata_kn_IN.obj \
    $(SLO)$/localedata_ko_KR.obj \
    $(SLO)$/localedata_ku_TR.obj \
    $(SLO)$/localedata_ky_KG.obj \
    $(SLO)$/localedata_lg_UG.obj \
    $(SLO)$/localedata_lif_NP.obj \
    $(SLO)$/localedata_ln_CD.obj \
    $(SLO)$/localedata_lo_LA.obj \
    $(SLO)$/localedata_mai_IN.obj \
    $(SLO)$/localedata_ml_IN.obj \
    $(SLO)$/localedata_mn_MN.obj \
    $(SLO)$/localedata_mr_IN.obj \
    $(SLO)$/localedata_ms_MY.obj \
    $(SLO)$/localedata_my_MM.obj \
    $(SLO)$/localedata_ne_NP.obj \
    $(SLO)$/localedata_nr_ZA.obj \
    $(SLO)$/localedata_nso_ZA.obj \
    $(SLO)$/localedata_om_ET.obj \
    $(SLO)$/localedata_or_IN.obj \
    $(SLO)$/localedata_pa_IN.obj \
    $(SLO)$/localedata_plt_MG.obj \
    $(SLO)$/localedata_rw_RW.obj \
    $(SLO)$/localedata_sg_CF.obj \
    $(SLO)$/localedata_shs_CA.obj \
    $(SLO)$/localedata_so_SO.obj \
    $(SLO)$/localedata_ss_ZA.obj \
    $(SLO)$/localedata_st_ZA.obj \
    $(SLO)$/localedata_sw_TZ.obj \
    $(SLO)$/localedata_ta_IN.obj \
    $(SLO)$/localedata_te_IN.obj \
    $(SLO)$/localedata_tg_TJ.obj \
    $(SLO)$/localedata_th_TH.obj \
    $(SLO)$/localedata_ti_ER.obj \
    $(SLO)$/localedata_tk_TM.obj \
    $(SLO)$/localedata_tn_ZA.obj \
    $(SLO)$/localedata_tpi_PG.obj \
    $(SLO)$/localedata_ts_ZA.obj \
    $(SLO)$/localedata_ug_CN.obj \
    $(SLO)$/localedata_ur_PK.obj \
    $(SLO)$/localedata_uz_UZ.obj \
    $(SLO)$/localedata_ve_ZA.obj \
    $(SLO)$/localedata_vi_VN.obj \
    $(SLO)$/localedata_xh_ZA.obj \
    $(SLO)$/localedata_yi_US.obj \
    $(SLO)$/localedata_zh_CN.obj \
    $(SLO)$/localedata_zh_HK.obj \
    $(SLO)$/localedata_zh_MO.obj \
    $(SLO)$/localedata_zh_SG.obj \
    $(SLO)$/localedata_zh_TW.obj \
    $(SLO)$/localedata_zu_ZA.obj

LIB4TARGET=	$(SLB)$/$(SHL4TARGET).lib
LIB4OBJFILES=$(SHL4OBJS)

DEPOBJFILES= \
    $(SHL1OBJS) \
    $(SHL2OBJS) \
    $(SHL3OBJS) \
    $(SHL4OBJS)

# Interim files generated by the saxparser executable, for dependencies
MY_MISC_CXXFILES := $(foreach,i,$(DEPOBJFILES) $(MISC)/$(i:b).cxx)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MY_MISC_CXXFILES) : $(BIN)$/saxparser$(EXECPOST) $(MISC)/saxparser.rdb

$(MISC)$/localedata_%.cxx : %.xml
    $(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(BIN)$/saxparser $* $< $@ \
        $(my_file)$(PWD)/$(MISC)/saxparser.rdb $(SOLARBINDIR)$/types.rdb \
        -env:OOO_INBUILD_SHAREDLIB_DIR=$(my_file)$(SOLARSHAREDBIN)
    $(RM) $(BIN)$/$(@:b).rdb

$(MISC)/saxparser.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/saxparser.input \
        $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/saxparser.input

$(MISC)/saxparser.input :
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@
