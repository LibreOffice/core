MKFILENAME:=TG_OBJ.MK

.IF "$(OBJTARGET)"!=""
$(OBJTARGET): $(OBJFILES) $(IDLOBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +echo. $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i)) >> $@
    @+$(TYPE) $@ | tr -d ï\n\rï > $@
    @+echo. >> $@
.ELSE			# "$(COM)"=="GCC"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(OBJTARGET)"!=""


.IF "$(SVXLIGHTOBJTARGET)"!=""
$(SVXLIGHTOBJTARGET): $(REAL_SVXLIGHTOBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
#     $(LIBMGR) $(LIBFLAGS) $@ $(OBJFILES)
    +echo. $(foreach,i,$(REAL_SVXLIGHTOBJFILES:f) $(ROBJ)$/$(i)) >> $@
.ELSE
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_SVXLIGHTOBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(SVXLIGHTOBJTARGET)"!=""

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)OBJTARGET)"!=""
$($(SECOND_BUILD)OBJTARGET): $(REAL_$(SECOND_BUILD)_OBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
#     $(LIBMGR) $(LIBFLAGS) $@ $(OBJFILES)
    +echo. $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)$/$(i)) > $@
.ELSE
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)OBJTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

