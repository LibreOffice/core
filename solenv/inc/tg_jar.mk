
.IF "$(JARTARGETN)"!=""

.IF "$(JARCOMPRESS)"==""
JARCOMPRESS_FLAG=0
.ENDIF

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(use_starjar)"!=""
XSTARJARPATH=$(strip $(STARJARPATH))
.ENDIF			# "$(use_starjar)"!=""
$(JARTARGETN) : $(JARMANIFEST)
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(JARMANIFEST)"!=""

.IF "$(CUSTOMMANIFESTFILE)"!=""

CUSTOMMANIFESTFILEDEP:=$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

$(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(subst,/,$/ $(DMAKE_WORK_DIR))$/$(CUSTOMMANIFESTFILE)
    +-$(RM) $@
#>& $(NULLDEV)
    +$(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

.IF "$(USE_EXTENDED_MANIFESTFILE)"!=""
EXTENDEDMANIFESTFILE=$(MISC)$/$(JARTARGET:b)
.IF "$(JARMANIFEST)"!=""
$(JARMANIFEST) : $(EXTENDEDMANIFESTFILE)
.ENDIF			# "$(JARMANIFEST)"!=""
$(MISC)$/$(JARTARGET:b) : $(SOLARINCDIR)$/$(UPD)minor.mk
    @+echo Specification-Title: $(SPECTITLE) > $@
    @+echo Specification-Version: $(VERSION) >> $@
    @+echo Specification-Vendor: $(VENDOR) >> $@
    @+echo Implementation-Title: $(IMPLTITLE) >> $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @+echo "Implementation-Version: $(RSCREVISION)" >> $@
.ELSE			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    @+echo Implementation-Version: $(RSCREVISION) >> $@
.ENDIF			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    @+echo Implementation-Vendor: $(VENDOR) >> $@
.ENDIF			# "$(USE_EXTENDED_MANIFESTFILE)"!=""

.IF "$(EXTENDEDMANIFESTFILE)"!=""

EXTENDEDMANIFESTFILEDEP:=$(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f)

$(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f) : $(EXTENDEDMANIFESTFILE)
    +-$(RM) $@
    +$(COPY) $(EXTENDEDMANIFESTFILE) $@
.ENDIF			# "$(EXTENDEDMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP) $(EXTENDEDMANIFESTFILEDEP)
    +-$(MKDIR) $(@:d) >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +echo Manifest-Version: 1.0 > $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    +echo "Solar-Version: $(RSCREVISION)" >> $@
.ELSE			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
    +echo Solar-Version: $(RSCREVISION) >> $@
.ENDIF			# "$(GUI)"=="UNX"" || "$(USE_SHELL)"!="4nt"
.IF "$(EXTENDEDMANIFESTFILE)"!=""
    +$(TYPE) $(MISC)$/$(TARGET)_$(EXTENDEDMANIFESTFILE:f) >> $@
.ENDIF			# "$(EXTENDEDMANIFESTFILE)"!=""
.IF "$(CUSTOMMANIFESTFILE)"!=""
    +$(TYPE) $(MISC)$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"!=""

#
# build jar 
#
.IF "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) .PHONY :
#  $(JARMANIFEST)
.ELSE			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.DIRCACHE = no
$(JARTARGETN) :
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell -cat -s $(MISC)$/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @+-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.IF "$(use_starjar)"!=""
    +-$(RM) $@
    @+-$(COPY) $(DMAKE_WORK_DIR)$/$(JARFLT) $(TARGET)_$(JARFLT) >& $(NULLDEV)
    cd $(CLASSDIR) && $(STARJAR) $@ $(JARMANIFEST) $(TARGET)_$(JARFLT) + $(XSTARJARPATH:s/ /+/)
.ELSE			# "$(use_starjar)"!=""
    +cd $(CLASSDIR) && zip -u -r $(@:f) $(subst,$(CLASSDIR)$/, $(JARMANIFEST)) $(subst,\,/ $(JARCLASSDIRS))
.ENDIF			# "$(use_starjar)"!=""
.ENDIF

