
.IF "$(JARTARGETN)"!=""

.IF "$(JARCOMPRESS)"==""
JARCOMPRESS_FLAG=0
.ENDIF

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(use_starjar)"!=""
XSTARJARPATH=$(strip $(STARJARPATH))
.ENDIF			# "$(use_starjar)"!=""
$(JARTARGETN) : $(JARMANIFEST)
.ENDIF			# "$(NEW_JAR_PACK)"==""

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(JARMANIFEST)"!=""

.IF "$(CUSTOMMANIFESTFILE)"!=""

CUSTOMMANIFESTFILEDEP:=..$/misc$/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

..$/misc$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(DMAKE_WORK_DIR)$/$(CUSTOMMANIFESTFILE)
    +-$(RM) $@
#>& $(NULLDEV)
    +$(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP) 
    +-$(MKDIR) .$/META-INF >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +echo Manifest-Version: 1.0 > $@
.IF "$(GUI)"=="UNX"
    +echo "Solar-Version: $(RSCREVISION)" >> $@
.ELSE			# "$(GUI)"=="UNX"
    +echo Solar-Version: $(RSCREVISION) >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(CUSTOMMANIFESTFILE)"!=""
    +$(TYPE) ..$/misc$/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"==""

#
# old jar dependencies...
#
.IF "$(NOJARDEP)$(NEW_JAR_PACK)"==""

..$/misc$/$(JARTARGETDEP) $(MISC)$/$(JARTARGETDEP) .SETDIR=$(CLASSDIR) : # $(JARTARGETN)
    @+echo -------------------------
    @+echo Making $@
    +-$(RM) ..$/misc$/$(JARTARGETDEP) >& $(NULLDEV)
    +-$(RM) ..$/misc$/$(JARTARGETDEP).tmp >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    +echo " $(JARTARGET) : \" > ..$/misc$/$(JARTARGETDEP)
    @+cat $(mktmp $(foreach,i,$(JARCLASSDIRS) $(shell find $i -type f -name "*.class" \! -name '*$$*' ))) >> ..$/misc$/$(JARTARGETDEP)
.ENDIF
.IF "$(GUI)"=="OS2"
#	@+echo $(foreach,i,$(JARCLASSDIRS) $(shell $(FIND) $i -type f -name "*.class" ! -name "*\$$*" -print >> ..$/misc$/$(JARTARGETDEP) )) >& $(NULLDEV)
    +echo  $(JARTARGETN) : \> ..$/misc$/$(JARTARGETDEP)
    +echo $(foreach,i,$(JARCLASSDIRS) $(shell $(FIND) $i -type f -name "*.class" ! -name "*$$*" -print >> ..$/misc$/$(JARTARGETDEP).tmp )) >& $(NULLDEV)
    +type ..$/misc$/$(JARTARGETDEP).tmp | sed "s#\.class#\.class \\#" >> ..$/misc$/$(JARTARGETDEP)
    +echo. >> ..$/misc$/$(JARTARGETDEP)
    +-$(RM) ..$/misc$/$(JARTARGETDEP).tmp >& $(NULLDEV)
.ENDIF
.IF "$(GUI)"=="WNT"
    +echo  $(JARTARGETN) : \> ..$/misc$/$(JARTARGETDEP)
    +echo $(foreach,i,$(JARCLASSDIRS) $(shell $(FIND) $i -type f -name "*.class" ! -name "*$$*" -print >> ..$/misc$/$(JARTARGETDEP).tmp )) >& $(NULLDEV)
    +type ..$/misc$/$(JARTARGETDEP).tmp | sed "s#\.class#\.class \\#" >> ..$/misc$/$(JARTARGETDEP)
    +echo. >> ..$/misc$/$(JARTARGETDEP)
    +-$(RM) ..$/misc$/$(JARTARGETDEP).tmp >& $(NULLDEV)
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
.IF "$(depend)"==""
.INCLUDE : $(MISC)$/$(JARTARGETN).dep
.ENDIF

# jardepfile=$(shell -cat -s $(MISC)$/$(JARTARGETN).dep )
.ENDIF
.ENDIF			# "$(NOJARDEP)$(NEW_JAR_PACK)"==""

#
# build jar 
#
.IF "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) .SETDIR=$(CLASSDIR) .PHONY :
#  $(JARMANIFEST)
.ELSE			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.DIRCACHE = no
$(JARTARGETN) .SETDIR=$(CLASSDIR) :
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell -cat -s $(MISC)$/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(UPDATER)"!=""
.IF "$(GUI)"=="WNT"
#	+tolodir .
.IF "$(JARIMGDIRS)"!=""
    +toloimg $(JARIMGDIRS)
.ENDIF
.ENDIF
.IF "$(OS)$(CPU)"=="SOLARISS"
    @+-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.ENDIF
.IF "$(NEW_JAR_PACK)"==""
    +-$(RM) $@
.IF "$(use_starjar)"!=""
    @+-$(COPY) ..$/..$/util$/$(JARFLT) $(JARFLT) >& $(NULLDEV)
.IF "$(JARMANIFEST)"!=""
    $(STARJAR) $@ $(JARMANIFEST) $(JARFLT) $(PATH_SEPERATOR) $(STARJARPATH)  
.ELSE
    $(STARJAR) $@ - $(JARFLT) $(PATH_SEPERATOR) $(STARJARPATH)  
.ENDIF
.ELSE			# "$(use_starjar)"!=""
.IF "$(JARMANIFEST)"!=""
    jar -cvmf$(JARCOMPRESS_FLAG) $(JARMANIFEST) $@ $(JARCLASSDIRS)
.ELSE
    jar -cvf$(JARCOMPRESS_FLAG) $@ $(JARCLASSDIRS)
.ENDIF
.ENDIF			# "$(use_starjar)"!=""
.ELSE			# "$(NEW_JAR_PACK)"==""
.IF "$(use_starjar)"!=""
    +-$(RM) $@
    @+-$(COPY) $(DMAKE_WORK_DIR)$/$(JARFLT) $(TARGET)_$(JARFLT) >& $(NULLDEV)
    $(STARJAR) $@ $(JARMANIFEST) $(TARGET)_$(JARFLT) + $(XSTARJARPATH:s/ /+/)
.ELSE			# "$(use_starjar)"!=""
    zip -u -r $@ $(JARMANIFEST) $(JARCLASSDIRS)
.ENDIF			# "$(use_starjar)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"==""
.ENDIF

