
.IF "$(YACCTARGET)"!=""
$(YACCTARGET): $(YACCFILES)
    @echo ------------------------------
    @echo Making: $@
#.IF "$(GUI)"!="UNX"
    $(BISON) $(YACCFLAGS) $(YACCOUT)$/$(YACCTARGET) $(YACCFILES)
#.ELSE
#	$(BISON) $(YACCFLAGS) $(YACCFILES)
#	mv y.tab.c $(INCCOM)/rscyacc.yxx
#	mv y.tab.h $(INCCOM)/yytab.h
#.ENDIF
.ENDIF

