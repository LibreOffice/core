#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*	makefile
#*
#*  Erstellung          MT 25.6.98
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-12-14 16:26:09 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/source/indexentry/makefile.mk,v $
#*
#*  Copyright (c) 1998, Star Division Corp.
#*
#*************************************************************************

PRJ=..$/..


PRJNAME=i18n
TARGET=indexentry

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk


# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/indexentrysupplier.obj \
            $(SLO)$/indexentrysupplier_CJK.obj \
            $(SLO)$/indexentrysupplier_zh_CN_pinyin.obj \
            $(SLO)$/indexentrysupplier_zh_CN_stroke.obj \
            $(SLO)$/indexentrysupplier_zh_CN_radical.obj \
            $(SLO)$/indexentrysupplier_zh_TW_zhuyin.obj \
            $(SLO)$/indexentrysupplier_zh_TW_radical.obj \
            $(SLO)$/indexentrysupplier_zh_TW_stroke.obj \
            $(SLO)$/indexentrysupplier_ko_KR_dict.obj \
            $(SLO)$/indexentrysupplier_Euro.obj \
            $(SLO)$/indexentrysupplier_en_US_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_it_IT_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_sv_SE_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_en_AU_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_da_DK_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_fi_FI_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_is_IS_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_nl_BE_alphanumeric.obj \
            $(SLO)$/indexentrysupplier_no_NO_alphanumeric.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
