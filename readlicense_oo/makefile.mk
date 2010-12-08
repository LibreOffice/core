PRJ=.
PRJNAME=readlicense_oo
TARGET=source

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
SYSLICBASE=license.txt LICENSE.odt
SYSLICDEST=$(MISC)$/license$/wnt
.ELSE          # "$(GUI)"=="WNT"
SYSLICBASE=LICENSE LICENSE.odt
SYSLICDEST=$(MISC)$/license$/unx
.ENDIF          # "$(GUI)"=="WNT"

SOURCELICENCES=$(foreach,i,$(SYSLICBASE) $(SYSLICDEST)$/$(i:b)$(i:e))

fallbacklicenses=$(foreach,i,{$(subst,$(defaultlangiso), $(alllangiso))} $(foreach,j,$(SYSLICBASE) $(SYSLICDEST)$/$(j:b)_$i$(j:e)))

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: $(SOURCELICENCES) $(fallbacklicenses) just_for_nice_optics \
        $(MISC)$/LICENSE.odt $(MISC)$/CREDITS.odt \
        $(MISC)$/THIRDPARTYLICENSEREADME.html

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : $(SOURCELICENCES)
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)

# for windows, convert linends to DOS
$(SYSLICDEST)$/license.% : txt$/license.%
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(PERL) -p -e 's/\r?\n$$/\r\n/' < $< > $@

# for others just copy
$(SYSLICDEST)$/LICENSE : txt$/license.txt
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@
$(SYSLICDEST)$/LICENSE.odt : odt$/LICENSE.odt
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@

# just copy into misc
$(MISC)$/LICENSE.odt: odt/LICENSE.odt
    $(COPY) odt/LICENSE.odt $@
$(MISC)$/CREDITS.odt: odt/CREDITS.odt
    $(COPY) odt/CREDITS.odt $@
$(MISC)$/THIRDPARTYLICENSEREADME.html: html/THIRDPARTYLICENSEREADME.html
    $(COPY) html/THIRDPARTYLICENSEREADME.html $@
