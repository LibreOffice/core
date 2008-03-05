#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_srs.mk,v $
#
#   $Revision: 1.25 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:33:58 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

#######################################################
# unroll begin

.IF "$(SRS$(TNR)NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID$(TNR)FILES=$(foreach,i,$(SRC$(TNR)FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS$(TNR)PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS$(TNR)NAME)_srs.hid
$(HIDSRS$(TNR)PARTICLE) : $(HID$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID$(TNR)FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS$(TNR)PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS$(TNR)NAME).dprr: $(SRC$(TNR)FILES) $(HIDSRS$(TNR)PARTICLE) $(HID$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS$(TNR)NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS$(TNR)NAME).srs} -fo=$@ $(SRC$(TNR)FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS$(TNR)NAME).srs: $(SRC$(TNR)FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS$(TNR)NAME)"!=""

# unroll end
#######################################################


