.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(SRCFILES)$(SRC1FILES)$(SRC2FILES)$(SRC3FILES)$(RCFILES)$(HXX1TARGET)$(HDBDEPNTARGET)$(IDLFILES)$(PARFILES)$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)"!=""
ALLDEP .PHONY: 
.IF "$(GUI)"=="UNX"
    @+if ( -e  "$(SRS)$/$(PWD:f).*.dpr" ) +-$(RM) "$(SRS)$/$(PWD:f).*.dpr" >& $(NULLDEV)
.ELSE
    @+-$(RM) $(SRS)$/$(PWD:f).*.dpr >& $(NULLDEV)
.ENDIF
    @+-$(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpj >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dp1 >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dp2 >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dp3 >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpc >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpz >& $(NULLDEV)
    +@echo ---
    +@echo      Old dependency files removed
    +@echo ---


ALLDPC: \
        $(CFILES) \
        $(CXXFILES) \
        $(RCFILES) \
        $(IDLFILES) \
        $(HDBDEPNTARGET)
.IF "$(nodep)"!=""
    @echo NOT-Making: Depend-Lists
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ELSE
    @echo Making : Dependencies 
.IF "$(CFILES)$(CXXFILES)"!=""
.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)"==""
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    @echo 		da stimmt was nicht!!
    @echo 		source files aber keine obj
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
#	@quit
.ENDIF
.ENDIF
.IF "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)$(PARFILES)"!=""
.IF "$(GUI)"=="UNX"
#	@+if ( -e  $(MISC)$/$(TARGET).dpr ) $(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+if ( -e  $(MISC)$/$(TARGET).dpw ) $(RM) $(MISC)$/$(TARGET).dpw >& $(NULLDEV)
    @+if ( -e  $(MISC)$/genjava.mk ) $(RM) $(MISC)$/genjava.mk >& $(NULLDEV)
.ELSE
#	@+-if exist $(MISC)$/$(TARGET).dpr $(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+-if exist $(MISC)$/$(TARGET).dpw $(RM) $(MISC)$/$(TARGET).dpw >& $(NULLDEV)
    @+-if exist $(MISC)$/genjava.mk $(RM) $(MISC)$/genjava.mk >& $(NULLDEV)
.ENDIF
    +$(MKDEP) @$(mktmp -O:$(MISC)$/$(TARGET).dpc $(MKDEPFLAGS) \
    $(HDBDEPNTARGET) $(OBJFILES) $(DEPOBJFILES) $(SLOFILES) $(RCFILES) \
    $(ALLPARFILES) )
#.IF "$(SVXLIGHTSLOFILES)"!=""
#	@+$(TYPE) $(mktmp $(foreach,i,$(SVXLIGHTSLOFILES) $(i:d:^"\n")sxl_$(i:f) : $i )) >> $(MISC)$/$(TARGET).dpc
#.ENDIF
.IF "$(SVXLIGHTOBJFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$(SVXLIGHTOBJFILES) $(i:d:^"\n")sxl_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$($(SECOND_BUILD)SLOFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)SLOFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $i )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$($(SECOND_BUILD)OBJFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)OBJFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$(GROUP)"=="WRITER"
.IF "$(debug)"==""
.IF "$(GUI)"=="UNX"
    @+-$(RM) $(MISC)$/$(TARGET).dpw
    @$(TYPE)  $(MISC)$/$(TARGET).dpc | $(SED) s\#$/slo$/\#$/dso$/\# | $(SED) s\#$/obj$/\#$/dbo$/\# > $(MISC)$/$(TARGET).dpw
.ELSE
    @+-$(RM) $(MISC)$/$(TARGET).dpw >& $(NULLDEV)
    @$(TYPE)  $(MISC)$/$(TARGET).dpc | $(SED) s#$/$/slo$/$/#$/$/dso$/$/# > $(TEMP)$/$(TARGET).dpt
    @$(TYPE) $(TEMP)$/$(TARGET).dpt | $(SED) s#$/$/obj$/$/#$/$/dbo$/$/# > $(MISC)$/$(TARGET).dpw
    @+-$(RM) $(TEMP)$/$(TARGET).dpt >& $(NULLDEV)
.ENDIF
.ELSE			# "$(debug)"==""
.IF "$(GUI)"=="UNX"
    @+$(COPY) $(MISC)$/$(TARGET).dpc $(MISC)$/$(TARGET).dpw
    @+-$(RM) $(MISC)$/$(TARGET).dpc
    @$(TYPE)  $(MISC)$/$(TARGET).dpw | $(SED) s\#$/dso$/\#$/slo$/\# | $(SED) s\#$/dbo$/\#$/obj$/\# > $(MISC)$/$(TARGET).dpc
.ELSE
    @+$(COPY) $(MISC)$/$(TARGET).dpc $(MISC)$/$(TARGET).dpw >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpc >& $(NULLDEV)
    @$(TYPE) $(MISC)$/$(TARGET).dpw | $(SED) s#$/$/dso$/$/#$/$/slo$/$/# > $(TEMP)$/$(TARGET).dpt
    @$(TYPE) $(TEMP)$/$(TARGET).dpt | $(SED) s#$/$/dbo$/$/#$/$/obj$/$/# > $(MISC)$/$(TARGET).dpc
    @+-$(RM) $(TEMP)$/$(TARGET).dpt >& $(NULLDEV)
.ENDIF
.ENDIF			# "$(debug)"==""
.ENDIF			# "$(GROUP)"=="WRITER"
.ELSE			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ENDIF			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
.ENDIF			# "$(nodep)"!=""
.ELSE		# irgendwas abhaengiges


ALLDPC:
    @echo ------------------------------
    @echo No Dependencies
.IF "$(GUI)"=="UNX"
    @echo "#" > $(MISC)$/$(TARGET).dpc
.IF "$(GROUP)"=="WRITER"
    @echo "#" > $(MISC)$/$(TARGET).dpw
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
    @echo # > $(MISC)$/$(TARGET).dpc
.IF "$(GROUP)"=="WRITER"
    @echo # > $(MISC)$/$(TARGET).dpw
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"

ALLDEP:
    @echo ------------------------------
    @echo No Dependencies

.ENDIF

