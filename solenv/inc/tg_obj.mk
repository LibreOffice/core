MKFILENAME:=TG_OBJ.MK

.IF "$(OBJTARGET)"!=""
$(OBJTARGET): $(OBJFILES) $(IDLOBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +echo. $(foreach,i,$(IDLOBJFILES:f) $(ROBJ)$/$(IDLPACKAGE)$/$(i)) > $@
    +echo. $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i)) >> $@
    @+$(TYPE) $@ | tr -d ï\n\rï > $@
    @+echo. >> $@
.ELSE			# "$(COM)"=="GCC"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(IDLOBJFILES:f) $(ROBJ)$/$(IDLPACKAGE)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
    +echo $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="MAC"
    +echo $(foreach,i,$(IDLOBJFILES:f) $(ROBJ)$/$(IDLPACKAGE)$/$(i)) | xargs -n1 > $@
    +echo $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(GUI)"=="WIN"
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(COM)"=="BLC"
    $(LIBMGR) $@ $(LIBFLAGS) +$(OBJFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ELSE			# "$(COM)"=="ICC"
    $(LIBMGR) r $@ $(SLOFILES)
.ENDIF			# "$(COM)"=="ICC"
.ENDIF			# "$(GUI)"=="OS2"

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
    +echo $(foreach,i,$(REAL_SVXLIGHTOBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_SVXLIGHTOBJFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(SVXLIGHTOBJTARGET)"!=""

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)OBJTARGET)"!=""
$($(SECOND_BUILD)OBJTARGET): $(REAL_$(SECOND_BUILD)OBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
#     $(LIBMGR) $(LIBFLAGS) $@ $(OBJFILES)
    +echo. $(foreach,i,$(REAL_$(SECOND_BUILD)OBJFILES:f) $(ROBJ)$/$(i)) >> $@
.ELSE
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_$(SECOND_BUILD)OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_$(SECOND_BUILD)OBJFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$($(SECOND_BUILD)OBJTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

