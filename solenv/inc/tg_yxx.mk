
.IF "$(YACCTARGET)"!=""
$(MISC)$/%.cxx : %.y
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
    +tr -d "\015" < $< > $(MISC)$/stripped_$<
.ELSE
    +cat $< > $(MISC)$/stripped_$<
.ENDIF
    $(BISON) $(YACCFLAGS) $(YACCTARGET) $(MISC)$/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @+-$(COPY:s/-f//) $@.h $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    @+-$(COPY:s/-f//) $(@:d)$/$(@:b).hxx $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    +$(TYPE) $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)

$(INCCOM)$/yy%.cxx : %.y
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
    +tr -d "\015" < $< > $(MISC)$/stripped_$<
.ELSE
    +cat $< > $(MISC)$/stripped_$<
.ENDIF
    $(BISON) $(YACCFLAGS) $(YACCTARGET) $(MISC)$/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @+-$(COPY:s/-f//) $@.h $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    @+-$(COPY:s/-f//) $(@:d)$/$(@:b).hxx $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    +$(TYPE) $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
.ENDIF

