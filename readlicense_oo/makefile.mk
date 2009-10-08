PRJ=.
PRJNAME=readlicense_oo
TARGET=source

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
SYSLICBASE=license.txt license.html license.rtf
SYSLICDEST=$(MISC)$/license$/wnt
.ELSE          # "$(GUI)"=="WNT"
SYSLICBASE=LICENSE LICENSE.html
SYSLICDEST=$(MISC)$/license$/unx
.ENDIF          # "$(GUI)"=="WNT"

SOURCELICENCES=$(foreach,i,$(SYSLICBASE) $(SYSLICDEST)$/$(i:b)_en-US$(i:e))

fallbacklicenses=$(foreach,i,{$(subst,$(defaultlangiso), $(alllangiso))} $(foreach,j,$(SYSLICBASE) $(SYSLICDEST)$/$(j:b)_$i$(j:e)))

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: $(SOURCELICENCES) $(fallbacklicenses) just_for_nice_optics

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : $(SOURCELICENCES)
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)

# for windows, convert linends to DOS
$(SYSLICDEST)$/license_en-US.% : source$/license$/license_en-US.%
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(PERL) -p -e 's/\r?\n$$/\r\n/' < $< > $@

# for others just copy
$(SYSLICDEST)$/LICENSE_en-US : source$/license$/license_en-US.txt
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@

$(SYSLICDEST)$/LICENSE_en-US.html : source$/license$/license_en-US.html
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@
