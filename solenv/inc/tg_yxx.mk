
.IF "$(YACCTARGET)"!=""
$(YACCTARGET): $(YACCFILES)
    @echo ------------------------------
    @echo Making: $@
    $(BISON) $(YACCFLAGS) $(YACCTARGET) $(YACCFILES)
# removing -f switch - avoid getting nothing when copying a file to itself
    @+-$(COPY:s/-f//) $@.h $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    @+-$(COPY:s/-f//) $(@:d)$/$(@:b).hxx $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
    +$(TYPE) $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)
.ENDIF

