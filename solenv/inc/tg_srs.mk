#*************************************************************************
#
#   $RCSfile: tg_srs.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2004-07-14 07:33:03 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


.IF "$(MULTI_SRC_FLAG)"==""

SRC1 ?= TNR!:=1
$(SRC1TARGET) .NULL : SRC1

SRC2 ?= TNR!:=2
$(SRC2TARGET) .NULL : SRC2

SRC3 ?= TNR!:=3
$(SRC3TARGET) .NULL : SRC3

SRC4 ?= TNR!:=4
$(SRC4TARGET) .NULL : SRC4

SRC5 ?= TNR!:=5
$(SRC5TARGET) .NULL : SRC5

SRC6 ?= TNR!:=6
$(SRC6TARGET) .NULL : SRC6

SRC7 ?= TNR!:=7
$(SRC7TARGET) .NULL : SRC7

SRC8 ?= TNR!:=8
$(SRC8TARGET) .NULL : SRC8

SRC9 ?= TNR!:=9
$(SRC9TARGET) .NULL : SRC9

# -----------------

DEPSRS1 ?= TNR!:=1
$(DEPSRS1FILE) .NULL : DEPSRS1

DEPSRS2 ?= TNR!:=2
$(DEPSRS2FILE) .NULL : DEPSRS2

DEPSRS3 ?= TNR!:=3
$(DEPSRS3FILE) .NULL : DEPSRS3

DEPSRS4 ?= TNR!:=4
$(DEPSRS4FILE) .NULL : DEPSRS4

DEPSRS5 ?= TNR!:=5
$(DEPSRS5FILE) .NULL : DEPSRS5

DEPSRS6 ?= TNR!:=6
$(DEPSRS6FILE) .NULL : DEPSRS6

DEPSRS7 ?= TNR!:=7
$(DEPSRS7FILE) .NULL : DEPSRS7

DEPSRS8 ?= TNR!:=8
$(DEPSRS8FILE) .NULL : DEPSRS8

DEPSRS9 ?= TNR!:=9
$(DEPSRS9FILE) .NULL : DEPSRS9

.ENDIF

.IF "$(MULTI_SRC_FLAG)" == ""
SRC1 SRC2 SRC3 SRC4 SRC5 SRC6 SRC7 SRC8 SRC9:
    @+dmake $(SRS)$/$(SRS$(TNR)NAME).srs MULTI_SRC_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)

DEPSRS1 DEPSRS2 DEPSRS3 DEPSRS4 DEPSRS5 DEPSRS6 DEPSRS7 DEPSRS8 DEPSRS9:
    @+dmake $(DEPSRS$(TNR)FILE) MULTI_SRC_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE

#######################################################
# unroll begin

.IF "$(SRS$(TNR)NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID$(TNR)FILES=$(foreach,i,$(SRC$(TNR)FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS$(TNR)PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS$(TNR)NAME)_srs.hid
$(HIDSRS$(TNR)PARTICLE) : $(HID$(TNR)FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID$(TNR)FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS$(TNR)PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS$(TNR)NAME).dprr: $(SRC$(TNR)FILES) $(HIDSRS$(TNR)PARTICLE) $(HID$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS$(TNR)NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS$(TNR)NAME).srs} -fo=$@ $(SRC$(TNR)FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC$(TNR)FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC$(TNR)FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $(foreach,i,$(SRC$(TNR)FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC$(TNR)FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS$(TNR)NAME)"!=""

# unroll end
#######################################################

.ENDIF          # "$(MULTI_SRC_FLAG)"==""


