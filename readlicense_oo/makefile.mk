PRJ=.
PRJNAME=readlicense_oo
TARGET=source

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: $(MISC)/$/license.txt $(MISC)/$/LICENSE \
	$(MISC)/$/NOTICE \
        $(MISC)$/LICENSE.odt $(MISC)$/CREDITS.odt \
        $(MISC)$/THIRDPARTYLICENSEREADME.html \
        $(MISC)/EULA_en-US.rtf

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : $(SOURCELICENCES)
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)

# for windows, convert linends to DOS
$(MISC)$/license.txt : txt$/license.txt
    $(PERL) -p -e 's/\r?\n$$/\r\n/' < $< > $@
# for others just copy
$(MISC)$/LICENSE : txt$/license.txt
    $(COPY) $< $@

$(MISC)$/SYSLICDEST)$/LICENSE.odt : odt$/LICENSE.odt
    $(COPY) $< $@

# just copy into misc
$(MISC)$/LICENSE.odt: odt/LICENSE.odt
    $(COPY) odt/LICENSE.odt $@
$(MISC)$/CREDITS.odt: odt/CREDITS.odt
    $(COPY) odt/CREDITS.odt $@
$(MISC)$/THIRDPARTYLICENSEREADME.html: html/THIRDPARTYLICENSEREADME.html
    $(COPY) html/THIRDPARTYLICENSEREADME.html $@
$(MISC)$/NOTICE: txt/NOTICE
    $(COPY) txt/NOTICE $@
$(MISC)/EULA_en-US.rtf: eula/EULA_en-US.rtf
    $(COPY) eula/EULA_*.rtf $(MISC)
