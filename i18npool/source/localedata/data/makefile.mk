#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.3 $
#*
#*  last change: $Author: bustamam $ $Date: 2002-03-15 20:09:33 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************

PRJ=..$/..$/..

PRJNAME=i18npool
TARGET=localedata

# Disable debugging on MSC compilers, due linker bug
.IF "$(COM)"=="MSC"
debug!=
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# English Locales
SHL1TARGET=localedata_en
SHL1IMPLIB= 	i$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)
SHL1OBJS= \
    $(SLO)$/localedata_en_AU.obj \
    $(SLO)$/localedata_en_BZ.obj \
    $(SLO)$/localedata_en_CA.obj \
    $(SLO)$/localedata_en_CB.obj \
    $(SLO)$/localedata_en_GB.obj \
    $(SLO)$/localedata_en_IE.obj \
    $(SLO)$/localedata_en_JM.obj \
    $(SLO)$/localedata_en_NZ.obj \
    $(SLO)$/localedata_en_PH.obj \
    $(SLO)$/localedata_en_TT.obj \
    $(SLO)$/localedata_en_US.obj \
    $(SLO)$/localedata_en_ZA.obj \
    $(SLO)$/localedata_en_ZW.obj

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# European Locales
SHL2TARGET=localedata_euro
SHL2IMPLIB= 	i$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEFLIB2NAME=	$(SHL2TARGET)
SHL2OBJS=	\
    $(SLO)$/localedata_de_DE.obj	\
    $(SLO)$/localedata_fr_FR.obj	\
    $(SLO)$/localedata_it_IT.obj	\
    $(SLO)$/localedata_sv_SE.obj	\
    $(SLO)$/localedata_ca_ES.obj	\
    $(SLO)$/localedata_cs_CZ.obj	\
    $(SLO)$/localedata_da_DK.obj	\
    $(SLO)$/localedata_de_AT.obj	\
    $(SLO)$/localedata_de_CH.obj	\
    $(SLO)$/localedata_de_LU.obj	\
    $(SLO)$/localedata_el_GR.obj	\
    $(SLO)$/localedata_fi_FI.obj	\
    $(SLO)$/localedata_fr_BE.obj	\
    $(SLO)$/localedata_fr_CA.obj	\
    $(SLO)$/localedata_fr_CH.obj	\
    $(SLO)$/localedata_fr_LU.obj	\
    $(SLO)$/localedata_fr_MC.obj	\
    $(SLO)$/localedata_is_IS.obj	\
    $(SLO)$/localedata_it_CH.obj	\
    $(SLO)$/localedata_nl_BE.obj	\
    $(SLO)$/localedata_nl_NL.obj	\
    $(SLO)$/localedata_no_NO.obj	\
    $(SLO)$/localedata_nn_NO.obj	\
    $(SLO)$/localedata_pl_PL.obj	\
    $(SLO)$/localedata_pt_BR.obj	\
    $(SLO)$/localedata_pt_PT.obj	\
    $(SLO)$/localedata_ru_RU.obj	\
    $(SLO)$/localedata_sv_FI.obj	\
    $(SLO)$/localedata_tr_TR.obj	\
    $(SLO)$/localedata_nb_NO.obj	\
    $(SLO)$/localedata_de_LI.obj

LIB2TARGET=	$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=$(SHL2OBJS)

# Missing
#	$(SLO)$/localedata_et_EE.obj

# Spanish Locales
SHL3TARGET=localedata_es
SHL3IMPLIB= 	i$(SHL3TARGET)
DEF3DEPN=	$(MISC)$/$(SHL3TARGET).flt
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def
DEF3NAME=	$(SHL3TARGET)
DEFLIB3NAME=	$(SHL3TARGET)
SHL3OBJS= \
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
    $(SLO)$/localedata_es_VE.obj

LIB3TARGET=	$(SLB)$/$(SHL3TARGET).lib
LIB3OBJFILES=$(SHL3OBJS)

# Other Locales
SHL4TARGET=localedata_others
SHL4IMPLIB= 	i$(SHL4TARGET)
DEF4DEPN=	$(MISC)$/$(SHL4TARGET).flt
SHL4DEF=	$(MISC)$/$(SHL4TARGET).def
DEF4NAME=	$(SHL4TARGET)
DEFLIB4NAME=	$(SHL4TARGET)
SHL4OBJS= \
    $(SLO)$/localedata_af_ZA.obj \
    $(SLO)$/localedata_ar_EG.obj \
    $(SLO)$/localedata_ar_SA.obj \
    $(SLO)$/localedata_he_IL.obj \
    $(SLO)$/localedata_hu_HU.obj \
    $(SLO)$/localedata_id_ID.obj \
    $(SLO)$/localedata_ja_JP.obj \
    $(SLO)$/localedata_ko_KR.obj \
    $(SLO)$/localedata_th_TH.obj \
    $(SLO)$/localedata_zh_CN.obj \
    $(SLO)$/localedata_zh_HK.obj \
    $(SLO)$/localedata_zh_MO.obj \
    $(SLO)$/localedata_zh_SG.obj \
    $(SLO)$/localedata_zh_TW.obj \
    $(SLO)$/localedata_hi_IN.obj

LIB4TARGET=	$(SLB)$/$(SHL4TARGET).lib
LIB4OBJFILES=$(SHL4OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(BIN)$/applicat.rdb : $(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)$/applicat.rdb
    +$(GNUCOPY) $(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)$/applicat.rdb $(BIN)$/applicat.rdb

$(MISC)$/localedata_%.cxx : %.xml
    +$(BIN)$/saxparser $* $< $@ $(BIN)$/applicat.rdb

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL4TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@
