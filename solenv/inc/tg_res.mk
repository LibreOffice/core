

.IF "$(RCTARGET)"!=""
$(RCTARGET): $(RCFILES) 		\
             $(RCDEPN)
    @echo ------------------------------
    @echo Making: $@
#	RCLANGFLAGS fuer setup/loader
.IF "$(NO_REC_RES)"!=""
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
.ENDIF
    $(RC) $(INCLUDE) -I$(SOLARRESDIR) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.IF "$(NO_REC_RES)"!=""
    @+echo > $(RES)$/$(@:f)
.ENDIF
.ENDIF
