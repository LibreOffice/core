

.IF "$(RCTARGET)"!=""
$(RCTARGET): $(RCFILES) 		\
             $(RCDEPN)
    @echo ------------------------------
    @echo Making: $@
#	RCLANGFLAGS fuer setup/loader
.IF "$(NO_REC_RES)"!=""
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
.ENDIF
    $(RC) $(INCLUDE) $(RCLANGFLAGS) $(RCFLAGS)
.IF "$(NO_REC_RES)"!=""
    @+echo > $(RES)$/$(@:f)
.ENDIF
.ENDIF
