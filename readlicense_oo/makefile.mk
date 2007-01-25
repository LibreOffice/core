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

fallbacklicenses=$(foreach,i,{$(subst,$(defaultlangiso), $(alllangiso))} $(foreach,j,$(SYSLICBASE) $(SYSLICDEST)$/$(j:b)_$i$(j:e)))

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: convert $(fallbacklicenses) just_for_nice_optics

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : convert
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)
    @echo done.

convert:
    @echo converting license files
    @-$(PERL) conv.pl -o $(MISC)
# no conversion for *.rtf
.IF "$(GUI)"=="WNT"
    @$(COPY) source$/license$/wnt$/*.rtf $(SYSLICDEST)
.ENDIF          # "$(GUI)"=="WNT"

