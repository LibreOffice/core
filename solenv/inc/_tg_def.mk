# unroll begin

.IF "$(DEF1TARGETN)"!=""

.IF "$(DEFLIB1NAME)"!=""
.IF "$(UPDATER)"!=""
DEF1DEPN+=$(SLB)$/$(DEFLIB1NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL1TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB1NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL1TARGET).flt $(SLB)\$(DEFLIB1NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL1TARGET).dll 							>>$@
.IF "$(DEFLIB1NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL1TARGET).flt $(SHL1LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF1EXPORT1)"!=""
    sed -e '/$(DEF1EXPORT1)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    sed -e '/$(DEF1EXPORT2)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    sed -e '/$(DEF1EXPORT3)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    sed -e '/$(DEF1EXPORT4)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    sed -e '/$(DEF1EXPORT5)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    sed -e '/$(DEF1EXPORT6)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    sed -e '/$(DEF1EXPORT7)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    sed -e '/$(DEF1EXPORT8)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    sed -e '/$(DEF1EXPORT9)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    sed -e '/$(DEF1EXPORT10)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    sed -e '/$(DEF1EXPORT11)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    sed -e '/$(DEF1EXPORT12)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    sed -e '/$(DEF1EXPORT13)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    sed -e '/$(DEF1EXPORT14)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    sed -e '/$(DEF1EXPORT15)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    sed -e '/$(DEF1EXPORT16)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    sed -e '/$(DEF1EXPORT17)/d' $@ 	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    sed -e '/$(DEF1EXPORT18)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    sed -e '/$(DEF1EXPORT19)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    sed -e '/$(DEF1EXPORT20)/d' $@  	> $(MISC)\$(SHL1TARGET).tmp
    copy $(MISC)\$(SHL1TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL1TARGET).tmp del $(MISC)\$(SHL1TARGET).tmp
.IF "$(DEF1EXPORT1)"!=""
    @echo $(DEF1EXPORT1)										>>$@
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    @echo $(DEF1EXPORT2)										>>$@
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    @echo $(DEF1EXPORT3)										>>$@
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    @echo $(DEF1EXPORT4)										>>$@
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    @echo $(DEF1EXPORT5)										>>$@
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    @echo $(DEF1EXPORT6)										>>$@
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    @echo $(DEF1EXPORT7)										>>$@
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    @echo $(DEF1EXPORT8)										>>$@
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    @echo $(DEF1EXPORT9)										>>$@
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    @echo $(DEF1EXPORT10)										>>$@
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    @echo $(DEF1EXPORT11)										>>$@
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    @echo $(DEF1EXPORT12)										>>$@
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    @echo $(DEF1EXPORT13)										>>$@
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    @echo $(DEF1EXPORT14)										>>$@
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    @echo $(DEF1EXPORT15)										>>$@
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    @echo $(DEF1EXPORT16)										>>$@
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    @echo $(DEF1EXPORT17)										>>$@
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    @echo $(DEF1EXPORT18)										>>$@
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    @echo $(DEF1EXPORT19)										>>$@
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    @echo $(DEF1EXPORT20)										>>$@
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
    $(TYPE) $(DEF1EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL1TARGET).map $(TOUCH) shl1.don
    if exist shl1.don echo  FUNCTIONS											>>$@
    if exist shl1.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL1TARGET).map
    if exist shl1.don (id2name $(SHL1TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl1.don del shl1.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP1HEAP)"!=""
    @echo NAME			$(APP1TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP1DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP1HEAP)								>>$@
    @echo STACKSIZE		$(APP1STACK)							>>$@
.ELSE			# "$(APP1HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL1TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF1EXPORT1)"!=""
    @echo $(DEF1EXPORT1)										>>$@
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    @echo $(DEF1EXPORT2)										>>$@
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    @echo $(DEF1EXPORT3)										>>$@
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    @echo $(DEF1EXPORT4)										>>$@
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    @echo $(DEF1EXPORT5)										>>$@
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    @echo $(DEF1EXPORT6)										>>$@
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    @echo $(DEF1EXPORT7)										>>$@
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    @echo $(DEF1EXPORT8)										>>$@
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    @echo $(DEF1EXPORT9)										>>$@
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    @echo $(DEF1EXPORT10)										>>$@
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    @echo $(DEF1EXPORT11)										>>$@
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    @echo $(DEF1EXPORT12)										>>$@
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    @echo $(DEF1EXPORT13)										>>$@
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    @echo $(DEF1EXPORT14)										>>$@
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    @echo $(DEF1EXPORT15)										>>$@
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    @echo $(DEF1EXPORT16)										>>$@
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    @echo $(DEF1EXPORT17)										>>$@
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    @echo $(DEF1EXPORT18)										>>$@
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    @echo $(DEF1EXPORT19)										>>$@
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    @echo $(DEF1EXPORT20)										>>$@
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
    $(TYPE) $(DEF1EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB1NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB1NAME)"!=""
.IF "$(DEF1CEXP)"!=""
    @$(LDUMP) -A $(DEF1CEXP) -E1 -F$(MISC)\$(SHL1TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF1CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL1TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF1CEXP)"!=""
.ENDIF			# "$(DEFLIB1NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB1NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB1NAME).lib  		>>tmp.cpf
.IF "$(DEF1CEXP)"!=""
    $(LDUMP) -A $(DEF1CEXP) -E1 -F$(MISC)\$(SHL1TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF1CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL1TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF1CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL1TARGET).flt $(SLB)\$(DEFLIB1NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB1NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL1TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB1NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL1TARGET).flt $(SHL1LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP1HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP1HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF1TARGETN) : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF1TARGETN) .PHONY : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL1TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB1NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL1TARGET).exp $(SLB)\$(DEFLIB1NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF1CEXP)"!=""
    @$(LDUMP2) -A $(DEF1CEXP) -E 20 -F $(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL1TARGET).exp
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF1EXPORT1)"!=""
    @echo $(DEF1EXPORT1)										>>$@
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    @echo $(DEF1EXPORT2)										>>$@
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    @echo $(DEF1EXPORT3)										>>$@
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    @echo $(DEF1EXPORT4)										>>$@
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    @echo $(DEF1EXPORT5)										>>$@
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    @echo $(DEF1EXPORT6)										>>$@
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    @echo $(DEF1EXPORT7)										>>$@
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    @echo $(DEF1EXPORT8)										>>$@
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    @echo $(DEF1EXPORT9)										>>$@
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    @echo $(DEF1EXPORT10)										>>$@
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    @echo $(DEF1EXPORT11)										>>$@
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    @echo $(DEF1EXPORT12)										>>$@
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    @echo $(DEF1EXPORT13)										>>$@
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    @echo $(DEF1EXPORT14)										>>$@
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    @echo $(DEF1EXPORT15)										>>$@
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    @echo $(DEF1EXPORT16)										>>$@
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    @echo $(DEF1EXPORT17)										>>$@
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    @echo $(DEF1EXPORT18)										>>$@
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    @echo $(DEF1EXPORT19)										>>$@
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    @echo $(DEF1EXPORT20)										>>$@
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
    $(TYPE) $(DEF1EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP1HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF1TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF2TARGETN)"!=""

.IF "$(DEFLIB2NAME)"!=""
.IF "$(UPDATER)"!=""
DEF2DEPN+=$(SLB)$/$(DEFLIB2NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL2TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB2NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL2TARGET).flt $(SLB)\$(DEFLIB2NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL2TARGET).dll 							>>$@
.IF "$(DEFLIB2NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL2TARGET).flt $(SHL2LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF2EXPORT1)"!=""
    sed -e '/$(DEF2EXPORT1)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    sed -e '/$(DEF2EXPORT2)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    sed -e '/$(DEF2EXPORT3)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    sed -e '/$(DEF2EXPORT4)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    sed -e '/$(DEF2EXPORT5)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    sed -e '/$(DEF2EXPORT6)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    sed -e '/$(DEF2EXPORT7)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    sed -e '/$(DEF2EXPORT8)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    sed -e '/$(DEF2EXPORT9)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    sed -e '/$(DEF2EXPORT10)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    sed -e '/$(DEF2EXPORT11)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    sed -e '/$(DEF2EXPORT12)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    sed -e '/$(DEF2EXPORT13)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    sed -e '/$(DEF2EXPORT14)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    sed -e '/$(DEF2EXPORT15)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    sed -e '/$(DEF2EXPORT16)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    sed -e '/$(DEF2EXPORT17)/d' $@ 	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    sed -e '/$(DEF2EXPORT18)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    sed -e '/$(DEF2EXPORT19)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    sed -e '/$(DEF2EXPORT20)/d' $@  	> $(MISC)\$(SHL2TARGET).tmp
    copy $(MISC)\$(SHL2TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL2TARGET).tmp del $(MISC)\$(SHL2TARGET).tmp
.IF "$(DEF2EXPORT1)"!=""
    @echo $(DEF2EXPORT1)										>>$@
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    @echo $(DEF2EXPORT2)										>>$@
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    @echo $(DEF2EXPORT3)										>>$@
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    @echo $(DEF2EXPORT4)										>>$@
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    @echo $(DEF2EXPORT5)										>>$@
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    @echo $(DEF2EXPORT6)										>>$@
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    @echo $(DEF2EXPORT7)										>>$@
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    @echo $(DEF2EXPORT8)										>>$@
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    @echo $(DEF2EXPORT9)										>>$@
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    @echo $(DEF2EXPORT10)										>>$@
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    @echo $(DEF2EXPORT11)										>>$@
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    @echo $(DEF2EXPORT12)										>>$@
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    @echo $(DEF2EXPORT13)										>>$@
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    @echo $(DEF2EXPORT14)										>>$@
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    @echo $(DEF2EXPORT15)										>>$@
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    @echo $(DEF2EXPORT16)										>>$@
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    @echo $(DEF2EXPORT17)										>>$@
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    @echo $(DEF2EXPORT18)										>>$@
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    @echo $(DEF2EXPORT19)										>>$@
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    @echo $(DEF2EXPORT20)										>>$@
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
    $(TYPE) $(DEF2EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL2TARGET).map $(TOUCH) shl2.don
    if exist shl2.don echo  FUNCTIONS											>>$@
    if exist shl2.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL2TARGET).map
    if exist shl2.don (id2name $(SHL2TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl2.don del shl2.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP2HEAP)"!=""
    @echo NAME			$(APP2TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP2DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP2HEAP)								>>$@
    @echo STACKSIZE		$(APP2STACK)							>>$@
.ELSE			# "$(APP2HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL2TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF2EXPORT1)"!=""
    @echo $(DEF2EXPORT1)										>>$@
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    @echo $(DEF2EXPORT2)										>>$@
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    @echo $(DEF2EXPORT3)										>>$@
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    @echo $(DEF2EXPORT4)										>>$@
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    @echo $(DEF2EXPORT5)										>>$@
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    @echo $(DEF2EXPORT6)										>>$@
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    @echo $(DEF2EXPORT7)										>>$@
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    @echo $(DEF2EXPORT8)										>>$@
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    @echo $(DEF2EXPORT9)										>>$@
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    @echo $(DEF2EXPORT10)										>>$@
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    @echo $(DEF2EXPORT11)										>>$@
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    @echo $(DEF2EXPORT12)										>>$@
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    @echo $(DEF2EXPORT13)										>>$@
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    @echo $(DEF2EXPORT14)										>>$@
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    @echo $(DEF2EXPORT15)										>>$@
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    @echo $(DEF2EXPORT16)										>>$@
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    @echo $(DEF2EXPORT17)										>>$@
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    @echo $(DEF2EXPORT18)										>>$@
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    @echo $(DEF2EXPORT19)										>>$@
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    @echo $(DEF2EXPORT20)										>>$@
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
    $(TYPE) $(DEF2EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB2NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB2NAME)"!=""
.IF "$(DEF2CEXP)"!=""
    @$(LDUMP) -A $(DEF2CEXP) -E1 -F$(MISC)\$(SHL2TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF2CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL2TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF2CEXP)"!=""
.ENDIF			# "$(DEFLIB2NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB2NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB2NAME).lib  		>>tmp.cpf
.IF "$(DEF2CEXP)"!=""
    $(LDUMP) -A $(DEF2CEXP) -E1 -F$(MISC)\$(SHL2TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF2CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL2TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF2CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL2TARGET).flt $(SLB)\$(DEFLIB2NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB2NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL2TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB2NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL2TARGET).flt $(SHL2LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP2HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP2HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF2TARGETN) : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF2TARGETN) .PHONY : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL2TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB2NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL2TARGET).exp $(SLB)\$(DEFLIB2NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF2CEXP)"!=""
    @$(LDUMP2) -A $(DEF2CEXP) -E 20 -F $(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL2TARGET).exp
.ENDIF				# "$(DEFLIB2NAME)"!=""
.IF "$(DEF2EXPORT1)"!=""
    @echo $(DEF2EXPORT1)										>>$@
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    @echo $(DEF2EXPORT2)										>>$@
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    @echo $(DEF2EXPORT3)										>>$@
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    @echo $(DEF2EXPORT4)										>>$@
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    @echo $(DEF2EXPORT5)										>>$@
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    @echo $(DEF2EXPORT6)										>>$@
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    @echo $(DEF2EXPORT7)										>>$@
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    @echo $(DEF2EXPORT8)										>>$@
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    @echo $(DEF2EXPORT9)										>>$@
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    @echo $(DEF2EXPORT10)										>>$@
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    @echo $(DEF2EXPORT11)										>>$@
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    @echo $(DEF2EXPORT12)										>>$@
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    @echo $(DEF2EXPORT13)										>>$@
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    @echo $(DEF2EXPORT14)										>>$@
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    @echo $(DEF2EXPORT15)										>>$@
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    @echo $(DEF2EXPORT16)										>>$@
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    @echo $(DEF2EXPORT17)										>>$@
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    @echo $(DEF2EXPORT18)										>>$@
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    @echo $(DEF2EXPORT19)										>>$@
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    @echo $(DEF2EXPORT20)										>>$@
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
    $(TYPE) $(DEF2EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP2HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF2TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF3TARGETN)"!=""

.IF "$(DEFLIB3NAME)"!=""
.IF "$(UPDATER)"!=""
DEF3DEPN+=$(SLB)$/$(DEFLIB3NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL3TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB3NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL3TARGET).flt $(SLB)\$(DEFLIB3NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL3TARGET).dll 							>>$@
.IF "$(DEFLIB3NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL3TARGET).flt $(SHL3LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF3EXPORT1)"!=""
    sed -e '/$(DEF3EXPORT1)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    sed -e '/$(DEF3EXPORT2)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    sed -e '/$(DEF3EXPORT3)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    sed -e '/$(DEF3EXPORT4)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    sed -e '/$(DEF3EXPORT5)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    sed -e '/$(DEF3EXPORT6)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    sed -e '/$(DEF3EXPORT7)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    sed -e '/$(DEF3EXPORT8)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    sed -e '/$(DEF3EXPORT9)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    sed -e '/$(DEF3EXPORT10)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    sed -e '/$(DEF3EXPORT11)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    sed -e '/$(DEF3EXPORT12)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    sed -e '/$(DEF3EXPORT13)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    sed -e '/$(DEF3EXPORT14)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    sed -e '/$(DEF3EXPORT15)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    sed -e '/$(DEF3EXPORT16)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    sed -e '/$(DEF3EXPORT17)/d' $@ 	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    sed -e '/$(DEF3EXPORT18)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    sed -e '/$(DEF3EXPORT19)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    sed -e '/$(DEF3EXPORT20)/d' $@  	> $(MISC)\$(SHL3TARGET).tmp
    copy $(MISC)\$(SHL3TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL3TARGET).tmp del $(MISC)\$(SHL3TARGET).tmp
.IF "$(DEF3EXPORT1)"!=""
    @echo $(DEF3EXPORT1)										>>$@
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    @echo $(DEF3EXPORT2)										>>$@
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    @echo $(DEF3EXPORT3)										>>$@
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    @echo $(DEF3EXPORT4)										>>$@
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    @echo $(DEF3EXPORT5)										>>$@
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    @echo $(DEF3EXPORT6)										>>$@
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    @echo $(DEF3EXPORT7)										>>$@
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    @echo $(DEF3EXPORT8)										>>$@
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    @echo $(DEF3EXPORT9)										>>$@
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    @echo $(DEF3EXPORT10)										>>$@
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    @echo $(DEF3EXPORT11)										>>$@
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    @echo $(DEF3EXPORT12)										>>$@
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    @echo $(DEF3EXPORT13)										>>$@
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    @echo $(DEF3EXPORT14)										>>$@
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    @echo $(DEF3EXPORT15)										>>$@
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    @echo $(DEF3EXPORT16)										>>$@
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    @echo $(DEF3EXPORT17)										>>$@
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    @echo $(DEF3EXPORT18)										>>$@
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    @echo $(DEF3EXPORT19)										>>$@
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    @echo $(DEF3EXPORT20)										>>$@
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
    $(TYPE) $(DEF3EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL3TARGET).map $(TOUCH) shl3.don
    if exist shl3.don echo  FUNCTIONS											>>$@
    if exist shl3.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL3TARGET).map
    if exist shl3.don (id2name $(SHL3TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl3.don del shl3.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP3HEAP)"!=""
    @echo NAME			$(APP3TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP3DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP3HEAP)								>>$@
    @echo STACKSIZE		$(APP3STACK)							>>$@
.ELSE			# "$(APP3HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL3TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF3EXPORT1)"!=""
    @echo $(DEF3EXPORT1)										>>$@
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    @echo $(DEF3EXPORT2)										>>$@
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    @echo $(DEF3EXPORT3)										>>$@
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    @echo $(DEF3EXPORT4)										>>$@
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    @echo $(DEF3EXPORT5)										>>$@
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    @echo $(DEF3EXPORT6)										>>$@
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    @echo $(DEF3EXPORT7)										>>$@
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    @echo $(DEF3EXPORT8)										>>$@
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    @echo $(DEF3EXPORT9)										>>$@
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    @echo $(DEF3EXPORT10)										>>$@
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    @echo $(DEF3EXPORT11)										>>$@
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    @echo $(DEF3EXPORT12)										>>$@
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    @echo $(DEF3EXPORT13)										>>$@
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    @echo $(DEF3EXPORT14)										>>$@
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    @echo $(DEF3EXPORT15)										>>$@
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    @echo $(DEF3EXPORT16)										>>$@
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    @echo $(DEF3EXPORT17)										>>$@
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    @echo $(DEF3EXPORT18)										>>$@
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    @echo $(DEF3EXPORT19)										>>$@
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    @echo $(DEF3EXPORT20)										>>$@
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
    $(TYPE) $(DEF3EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB3NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB3NAME)"!=""
.IF "$(DEF3CEXP)"!=""
    @$(LDUMP) -A $(DEF3CEXP) -E1 -F$(MISC)\$(SHL3TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF3CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL3TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF3CEXP)"!=""
.ENDIF			# "$(DEFLIB3NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB3NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB3NAME).lib  		>>tmp.cpf
.IF "$(DEF3CEXP)"!=""
    $(LDUMP) -A $(DEF3CEXP) -E1 -F$(MISC)\$(SHL3TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF3CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL3TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF3CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL3TARGET).flt $(SLB)\$(DEFLIB3NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB3NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL3TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB3NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL3TARGET).flt $(SHL3LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP3HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP3HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF3TARGETN) : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF3TARGETN) .PHONY : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL3TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB3NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL3TARGET).exp $(SLB)\$(DEFLIB3NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF3CEXP)"!=""
    @$(LDUMP2) -A $(DEF3CEXP) -E 20 -F $(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL3TARGET).exp
.ENDIF				# "$(DEFLIB3NAME)"!=""
.IF "$(DEF3EXPORT1)"!=""
    @echo $(DEF3EXPORT1)										>>$@
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    @echo $(DEF3EXPORT2)										>>$@
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    @echo $(DEF3EXPORT3)										>>$@
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    @echo $(DEF3EXPORT4)										>>$@
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    @echo $(DEF3EXPORT5)										>>$@
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    @echo $(DEF3EXPORT6)										>>$@
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    @echo $(DEF3EXPORT7)										>>$@
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    @echo $(DEF3EXPORT8)										>>$@
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    @echo $(DEF3EXPORT9)										>>$@
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    @echo $(DEF3EXPORT10)										>>$@
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    @echo $(DEF3EXPORT11)										>>$@
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    @echo $(DEF3EXPORT12)										>>$@
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    @echo $(DEF3EXPORT13)										>>$@
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    @echo $(DEF3EXPORT14)										>>$@
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    @echo $(DEF3EXPORT15)										>>$@
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    @echo $(DEF3EXPORT16)										>>$@
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    @echo $(DEF3EXPORT17)										>>$@
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    @echo $(DEF3EXPORT18)										>>$@
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    @echo $(DEF3EXPORT19)										>>$@
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    @echo $(DEF3EXPORT20)										>>$@
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
    $(TYPE) $(DEF3EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP3HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF3TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF4TARGETN)"!=""

.IF "$(DEFLIB4NAME)"!=""
.IF "$(UPDATER)"!=""
DEF4DEPN+=$(SLB)$/$(DEFLIB4NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL4TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB4NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL4TARGET).flt $(SLB)\$(DEFLIB4NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL4TARGET).dll 							>>$@
.IF "$(DEFLIB4NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL4TARGET).flt $(SHL4LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF4EXPORT1)"!=""
    sed -e '/$(DEF4EXPORT1)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    sed -e '/$(DEF4EXPORT2)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    sed -e '/$(DEF4EXPORT3)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    sed -e '/$(DEF4EXPORT4)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    sed -e '/$(DEF4EXPORT5)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    sed -e '/$(DEF4EXPORT6)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    sed -e '/$(DEF4EXPORT7)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    sed -e '/$(DEF4EXPORT8)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    sed -e '/$(DEF4EXPORT9)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    sed -e '/$(DEF4EXPORT10)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    sed -e '/$(DEF4EXPORT11)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    sed -e '/$(DEF4EXPORT12)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    sed -e '/$(DEF4EXPORT13)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    sed -e '/$(DEF4EXPORT14)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    sed -e '/$(DEF4EXPORT15)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    sed -e '/$(DEF4EXPORT16)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    sed -e '/$(DEF4EXPORT17)/d' $@ 	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    sed -e '/$(DEF4EXPORT18)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    sed -e '/$(DEF4EXPORT19)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    sed -e '/$(DEF4EXPORT20)/d' $@  	> $(MISC)\$(SHL4TARGET).tmp
    copy $(MISC)\$(SHL4TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL4TARGET).tmp del $(MISC)\$(SHL4TARGET).tmp
.IF "$(DEF4EXPORT1)"!=""
    @echo $(DEF4EXPORT1)										>>$@
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    @echo $(DEF4EXPORT2)										>>$@
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    @echo $(DEF4EXPORT3)										>>$@
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    @echo $(DEF4EXPORT4)										>>$@
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    @echo $(DEF4EXPORT5)										>>$@
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    @echo $(DEF4EXPORT6)										>>$@
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    @echo $(DEF4EXPORT7)										>>$@
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    @echo $(DEF4EXPORT8)										>>$@
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    @echo $(DEF4EXPORT9)										>>$@
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    @echo $(DEF4EXPORT10)										>>$@
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    @echo $(DEF4EXPORT11)										>>$@
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    @echo $(DEF4EXPORT12)										>>$@
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    @echo $(DEF4EXPORT13)										>>$@
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    @echo $(DEF4EXPORT14)										>>$@
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    @echo $(DEF4EXPORT15)										>>$@
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    @echo $(DEF4EXPORT16)										>>$@
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    @echo $(DEF4EXPORT17)										>>$@
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    @echo $(DEF4EXPORT18)										>>$@
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    @echo $(DEF4EXPORT19)										>>$@
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    @echo $(DEF4EXPORT20)										>>$@
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
    $(TYPE) $(DEF4EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL4TARGET).map $(TOUCH) shl4.don
    if exist shl4.don echo  FUNCTIONS											>>$@
    if exist shl4.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL4TARGET).map
    if exist shl4.don (id2name $(SHL4TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl4.don del shl4.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP4HEAP)"!=""
    @echo NAME			$(APP4TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP4DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP4HEAP)								>>$@
    @echo STACKSIZE		$(APP4STACK)							>>$@
.ELSE			# "$(APP4HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL4TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF4EXPORT1)"!=""
    @echo $(DEF4EXPORT1)										>>$@
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    @echo $(DEF4EXPORT2)										>>$@
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    @echo $(DEF4EXPORT3)										>>$@
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    @echo $(DEF4EXPORT4)										>>$@
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    @echo $(DEF4EXPORT5)										>>$@
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    @echo $(DEF4EXPORT6)										>>$@
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    @echo $(DEF4EXPORT7)										>>$@
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    @echo $(DEF4EXPORT8)										>>$@
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    @echo $(DEF4EXPORT9)										>>$@
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    @echo $(DEF4EXPORT10)										>>$@
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    @echo $(DEF4EXPORT11)										>>$@
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    @echo $(DEF4EXPORT12)										>>$@
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    @echo $(DEF4EXPORT13)										>>$@
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    @echo $(DEF4EXPORT14)										>>$@
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    @echo $(DEF4EXPORT15)										>>$@
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    @echo $(DEF4EXPORT16)										>>$@
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    @echo $(DEF4EXPORT17)										>>$@
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    @echo $(DEF4EXPORT18)										>>$@
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    @echo $(DEF4EXPORT19)										>>$@
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    @echo $(DEF4EXPORT20)										>>$@
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
    $(TYPE) $(DEF4EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB4NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB4NAME)"!=""
.IF "$(DEF4CEXP)"!=""
    @$(LDUMP) -A $(DEF4CEXP) -E1 -F$(MISC)\$(SHL4TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF4CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL4TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF4CEXP)"!=""
.ENDIF			# "$(DEFLIB4NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB4NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB4NAME).lib  		>>tmp.cpf
.IF "$(DEF4CEXP)"!=""
    $(LDUMP) -A $(DEF4CEXP) -E1 -F$(MISC)\$(SHL4TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF4CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL4TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF4CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL4TARGET).flt $(SLB)\$(DEFLIB4NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB4NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL4TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB4NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL4TARGET).flt $(SHL4LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP4HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP4HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF4TARGETN) : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF4TARGETN) .PHONY : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL4TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB4NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL4TARGET).exp $(SLB)\$(DEFLIB4NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF4CEXP)"!=""
    @$(LDUMP2) -A $(DEF4CEXP) -E 20 -F $(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL4TARGET).exp
.ENDIF				# "$(DEFLIB4NAME)"!=""
.IF "$(DEF4EXPORT1)"!=""
    @echo $(DEF4EXPORT1)										>>$@
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    @echo $(DEF4EXPORT2)										>>$@
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    @echo $(DEF4EXPORT3)										>>$@
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    @echo $(DEF4EXPORT4)										>>$@
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    @echo $(DEF4EXPORT5)										>>$@
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    @echo $(DEF4EXPORT6)										>>$@
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    @echo $(DEF4EXPORT7)										>>$@
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    @echo $(DEF4EXPORT8)										>>$@
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    @echo $(DEF4EXPORT9)										>>$@
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    @echo $(DEF4EXPORT10)										>>$@
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    @echo $(DEF4EXPORT11)										>>$@
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    @echo $(DEF4EXPORT12)										>>$@
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    @echo $(DEF4EXPORT13)										>>$@
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    @echo $(DEF4EXPORT14)										>>$@
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    @echo $(DEF4EXPORT15)										>>$@
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    @echo $(DEF4EXPORT16)										>>$@
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    @echo $(DEF4EXPORT17)										>>$@
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    @echo $(DEF4EXPORT18)										>>$@
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    @echo $(DEF4EXPORT19)										>>$@
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    @echo $(DEF4EXPORT20)										>>$@
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
    $(TYPE) $(DEF4EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP4HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF4TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF5TARGETN)"!=""

.IF "$(DEFLIB5NAME)"!=""
.IF "$(UPDATER)"!=""
DEF5DEPN+=$(SLB)$/$(DEFLIB5NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL5TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB5NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL5TARGET).flt $(SLB)\$(DEFLIB5NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL5TARGET).dll 							>>$@
.IF "$(DEFLIB5NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL5TARGET).flt $(SHL5LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF5EXPORT1)"!=""
    sed -e '/$(DEF5EXPORT1)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    sed -e '/$(DEF5EXPORT2)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    sed -e '/$(DEF5EXPORT3)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    sed -e '/$(DEF5EXPORT4)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    sed -e '/$(DEF5EXPORT5)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    sed -e '/$(DEF5EXPORT6)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    sed -e '/$(DEF5EXPORT7)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    sed -e '/$(DEF5EXPORT8)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    sed -e '/$(DEF5EXPORT9)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    sed -e '/$(DEF5EXPORT10)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    sed -e '/$(DEF5EXPORT11)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    sed -e '/$(DEF5EXPORT12)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    sed -e '/$(DEF5EXPORT13)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    sed -e '/$(DEF5EXPORT14)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    sed -e '/$(DEF5EXPORT15)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    sed -e '/$(DEF5EXPORT16)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    sed -e '/$(DEF5EXPORT17)/d' $@ 	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    sed -e '/$(DEF5EXPORT18)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    sed -e '/$(DEF5EXPORT19)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    sed -e '/$(DEF5EXPORT20)/d' $@  	> $(MISC)\$(SHL5TARGET).tmp
    copy $(MISC)\$(SHL5TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL5TARGET).tmp del $(MISC)\$(SHL5TARGET).tmp
.IF "$(DEF5EXPORT1)"!=""
    @echo $(DEF5EXPORT1)										>>$@
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    @echo $(DEF5EXPORT2)										>>$@
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    @echo $(DEF5EXPORT3)										>>$@
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    @echo $(DEF5EXPORT4)										>>$@
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    @echo $(DEF5EXPORT5)										>>$@
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    @echo $(DEF5EXPORT6)										>>$@
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    @echo $(DEF5EXPORT7)										>>$@
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    @echo $(DEF5EXPORT8)										>>$@
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    @echo $(DEF5EXPORT9)										>>$@
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    @echo $(DEF5EXPORT10)										>>$@
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    @echo $(DEF5EXPORT11)										>>$@
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    @echo $(DEF5EXPORT12)										>>$@
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    @echo $(DEF5EXPORT13)										>>$@
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    @echo $(DEF5EXPORT14)										>>$@
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    @echo $(DEF5EXPORT15)										>>$@
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    @echo $(DEF5EXPORT16)										>>$@
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    @echo $(DEF5EXPORT17)										>>$@
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    @echo $(DEF5EXPORT18)										>>$@
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    @echo $(DEF5EXPORT19)										>>$@
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    @echo $(DEF5EXPORT20)										>>$@
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
    $(TYPE) $(DEF5EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL5TARGET).map $(TOUCH) shl5.don
    if exist shl5.don echo  FUNCTIONS											>>$@
    if exist shl5.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL5TARGET).map
    if exist shl5.don (id2name $(SHL5TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl5.don del shl5.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP5HEAP)"!=""
    @echo NAME			$(APP5TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP5DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP5HEAP)								>>$@
    @echo STACKSIZE		$(APP5STACK)							>>$@
.ELSE			# "$(APP5HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL5TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF5EXPORT1)"!=""
    @echo $(DEF5EXPORT1)										>>$@
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    @echo $(DEF5EXPORT2)										>>$@
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    @echo $(DEF5EXPORT3)										>>$@
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    @echo $(DEF5EXPORT4)										>>$@
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    @echo $(DEF5EXPORT5)										>>$@
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    @echo $(DEF5EXPORT6)										>>$@
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    @echo $(DEF5EXPORT7)										>>$@
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    @echo $(DEF5EXPORT8)										>>$@
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    @echo $(DEF5EXPORT9)										>>$@
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    @echo $(DEF5EXPORT10)										>>$@
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    @echo $(DEF5EXPORT11)										>>$@
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    @echo $(DEF5EXPORT12)										>>$@
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    @echo $(DEF5EXPORT13)										>>$@
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    @echo $(DEF5EXPORT14)										>>$@
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    @echo $(DEF5EXPORT15)										>>$@
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    @echo $(DEF5EXPORT16)										>>$@
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    @echo $(DEF5EXPORT17)										>>$@
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    @echo $(DEF5EXPORT18)										>>$@
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    @echo $(DEF5EXPORT19)										>>$@
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    @echo $(DEF5EXPORT20)										>>$@
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
    $(TYPE) $(DEF5EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB5NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB5NAME)"!=""
.IF "$(DEF5CEXP)"!=""
    @$(LDUMP) -A $(DEF5CEXP) -E1 -F$(MISC)\$(SHL5TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF5CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL5TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF5CEXP)"!=""
.ENDIF			# "$(DEFLIB5NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB5NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB5NAME).lib  		>>tmp.cpf
.IF "$(DEF5CEXP)"!=""
    $(LDUMP) -A $(DEF5CEXP) -E1 -F$(MISC)\$(SHL5TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF5CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL5TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF5CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL5TARGET).flt $(SLB)\$(DEFLIB5NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB5NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL5TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB5NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL5TARGET).flt $(SHL5LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP5HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP5HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF5TARGETN) : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF5TARGETN) .PHONY : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL5TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB5NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL5TARGET).exp $(SLB)\$(DEFLIB5NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF5CEXP)"!=""
    @$(LDUMP2) -A $(DEF5CEXP) -E 20 -F $(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL5TARGET).exp
.ENDIF				# "$(DEFLIB5NAME)"!=""
.IF "$(DEF5EXPORT1)"!=""
    @echo $(DEF5EXPORT1)										>>$@
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    @echo $(DEF5EXPORT2)										>>$@
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    @echo $(DEF5EXPORT3)										>>$@
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    @echo $(DEF5EXPORT4)										>>$@
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    @echo $(DEF5EXPORT5)										>>$@
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    @echo $(DEF5EXPORT6)										>>$@
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    @echo $(DEF5EXPORT7)										>>$@
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    @echo $(DEF5EXPORT8)										>>$@
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    @echo $(DEF5EXPORT9)										>>$@
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    @echo $(DEF5EXPORT10)										>>$@
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    @echo $(DEF5EXPORT11)										>>$@
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    @echo $(DEF5EXPORT12)										>>$@
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    @echo $(DEF5EXPORT13)										>>$@
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    @echo $(DEF5EXPORT14)										>>$@
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    @echo $(DEF5EXPORT15)										>>$@
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    @echo $(DEF5EXPORT16)										>>$@
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    @echo $(DEF5EXPORT17)										>>$@
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    @echo $(DEF5EXPORT18)										>>$@
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    @echo $(DEF5EXPORT19)										>>$@
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    @echo $(DEF5EXPORT20)										>>$@
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
    $(TYPE) $(DEF5EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP5HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF5TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF6TARGETN)"!=""

.IF "$(DEFLIB6NAME)"!=""
.IF "$(UPDATER)"!=""
DEF6DEPN+=$(SLB)$/$(DEFLIB6NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL6TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB6NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL6TARGET).flt $(SLB)\$(DEFLIB6NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL6TARGET).dll 							>>$@
.IF "$(DEFLIB6NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL6TARGET).flt $(SHL6LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF6EXPORT1)"!=""
    sed -e '/$(DEF6EXPORT1)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    sed -e '/$(DEF6EXPORT2)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    sed -e '/$(DEF6EXPORT3)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    sed -e '/$(DEF6EXPORT4)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    sed -e '/$(DEF6EXPORT5)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    sed -e '/$(DEF6EXPORT6)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    sed -e '/$(DEF6EXPORT7)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    sed -e '/$(DEF6EXPORT8)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    sed -e '/$(DEF6EXPORT9)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    sed -e '/$(DEF6EXPORT10)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    sed -e '/$(DEF6EXPORT11)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    sed -e '/$(DEF6EXPORT12)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    sed -e '/$(DEF6EXPORT13)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    sed -e '/$(DEF6EXPORT14)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    sed -e '/$(DEF6EXPORT15)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    sed -e '/$(DEF6EXPORT16)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    sed -e '/$(DEF6EXPORT17)/d' $@ 	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    sed -e '/$(DEF6EXPORT18)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    sed -e '/$(DEF6EXPORT19)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    sed -e '/$(DEF6EXPORT20)/d' $@  	> $(MISC)\$(SHL6TARGET).tmp
    copy $(MISC)\$(SHL6TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL6TARGET).tmp del $(MISC)\$(SHL6TARGET).tmp
.IF "$(DEF6EXPORT1)"!=""
    @echo $(DEF6EXPORT1)										>>$@
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    @echo $(DEF6EXPORT2)										>>$@
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    @echo $(DEF6EXPORT3)										>>$@
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    @echo $(DEF6EXPORT4)										>>$@
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    @echo $(DEF6EXPORT5)										>>$@
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    @echo $(DEF6EXPORT6)										>>$@
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    @echo $(DEF6EXPORT7)										>>$@
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    @echo $(DEF6EXPORT8)										>>$@
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    @echo $(DEF6EXPORT9)										>>$@
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    @echo $(DEF6EXPORT10)										>>$@
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    @echo $(DEF6EXPORT11)										>>$@
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    @echo $(DEF6EXPORT12)										>>$@
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    @echo $(DEF6EXPORT13)										>>$@
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    @echo $(DEF6EXPORT14)										>>$@
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    @echo $(DEF6EXPORT15)										>>$@
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    @echo $(DEF6EXPORT16)										>>$@
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    @echo $(DEF6EXPORT17)										>>$@
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    @echo $(DEF6EXPORT18)										>>$@
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    @echo $(DEF6EXPORT19)										>>$@
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    @echo $(DEF6EXPORT20)										>>$@
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
    $(TYPE) $(DEF6EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL6TARGET).map $(TOUCH) shl6.don
    if exist shl6.don echo  FUNCTIONS											>>$@
    if exist shl6.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL6TARGET).map
    if exist shl6.don (id2name $(SHL6TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl6.don del shl6.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP6HEAP)"!=""
    @echo NAME			$(APP6TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP6DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP6HEAP)								>>$@
    @echo STACKSIZE		$(APP6STACK)							>>$@
.ELSE			# "$(APP6HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL6TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF6EXPORT1)"!=""
    @echo $(DEF6EXPORT1)										>>$@
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    @echo $(DEF6EXPORT2)										>>$@
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    @echo $(DEF6EXPORT3)										>>$@
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    @echo $(DEF6EXPORT4)										>>$@
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    @echo $(DEF6EXPORT5)										>>$@
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    @echo $(DEF6EXPORT6)										>>$@
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    @echo $(DEF6EXPORT7)										>>$@
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    @echo $(DEF6EXPORT8)										>>$@
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    @echo $(DEF6EXPORT9)										>>$@
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    @echo $(DEF6EXPORT10)										>>$@
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    @echo $(DEF6EXPORT11)										>>$@
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    @echo $(DEF6EXPORT12)										>>$@
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    @echo $(DEF6EXPORT13)										>>$@
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    @echo $(DEF6EXPORT14)										>>$@
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    @echo $(DEF6EXPORT15)										>>$@
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    @echo $(DEF6EXPORT16)										>>$@
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    @echo $(DEF6EXPORT17)										>>$@
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    @echo $(DEF6EXPORT18)										>>$@
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    @echo $(DEF6EXPORT19)										>>$@
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    @echo $(DEF6EXPORT20)										>>$@
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
    $(TYPE) $(DEF6EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB6NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB6NAME)"!=""
.IF "$(DEF6CEXP)"!=""
    @$(LDUMP) -A $(DEF6CEXP) -E1 -F$(MISC)\$(SHL6TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF6CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL6TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF6CEXP)"!=""
.ENDIF			# "$(DEFLIB6NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB6NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB6NAME).lib  		>>tmp.cpf
.IF "$(DEF6CEXP)"!=""
    $(LDUMP) -A $(DEF6CEXP) -E1 -F$(MISC)\$(SHL6TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF6CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL6TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF6CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL6TARGET).flt $(SLB)\$(DEFLIB6NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB6NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL6TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB6NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL6TARGET).flt $(SHL6LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP6HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP6HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF6TARGETN) : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF6TARGETN) .PHONY : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL6TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB6NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL6TARGET).exp $(SLB)\$(DEFLIB6NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF6CEXP)"!=""
    @$(LDUMP2) -A $(DEF6CEXP) -E 20 -F $(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL6TARGET).exp
.ENDIF				# "$(DEFLIB6NAME)"!=""
.IF "$(DEF6EXPORT1)"!=""
    @echo $(DEF6EXPORT1)										>>$@
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    @echo $(DEF6EXPORT2)										>>$@
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    @echo $(DEF6EXPORT3)										>>$@
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    @echo $(DEF6EXPORT4)										>>$@
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    @echo $(DEF6EXPORT5)										>>$@
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    @echo $(DEF6EXPORT6)										>>$@
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    @echo $(DEF6EXPORT7)										>>$@
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    @echo $(DEF6EXPORT8)										>>$@
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    @echo $(DEF6EXPORT9)										>>$@
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    @echo $(DEF6EXPORT10)										>>$@
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    @echo $(DEF6EXPORT11)										>>$@
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    @echo $(DEF6EXPORT12)										>>$@
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    @echo $(DEF6EXPORT13)										>>$@
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    @echo $(DEF6EXPORT14)										>>$@
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    @echo $(DEF6EXPORT15)										>>$@
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    @echo $(DEF6EXPORT16)										>>$@
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    @echo $(DEF6EXPORT17)										>>$@
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    @echo $(DEF6EXPORT18)										>>$@
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    @echo $(DEF6EXPORT19)										>>$@
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    @echo $(DEF6EXPORT20)										>>$@
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
    $(TYPE) $(DEF6EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP6HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF6TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF7TARGETN)"!=""

.IF "$(DEFLIB7NAME)"!=""
.IF "$(UPDATER)"!=""
DEF7DEPN+=$(SLB)$/$(DEFLIB7NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL7TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB7NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL7TARGET).flt $(SLB)\$(DEFLIB7NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL7TARGET).dll 							>>$@
.IF "$(DEFLIB7NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL7TARGET).flt $(SHL7LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF7EXPORT1)"!=""
    sed -e '/$(DEF7EXPORT1)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    sed -e '/$(DEF7EXPORT2)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    sed -e '/$(DEF7EXPORT3)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    sed -e '/$(DEF7EXPORT4)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    sed -e '/$(DEF7EXPORT5)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    sed -e '/$(DEF7EXPORT6)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    sed -e '/$(DEF7EXPORT7)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    sed -e '/$(DEF7EXPORT8)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    sed -e '/$(DEF7EXPORT9)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    sed -e '/$(DEF7EXPORT10)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    sed -e '/$(DEF7EXPORT11)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    sed -e '/$(DEF7EXPORT12)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    sed -e '/$(DEF7EXPORT13)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    sed -e '/$(DEF7EXPORT14)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    sed -e '/$(DEF7EXPORT15)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    sed -e '/$(DEF7EXPORT16)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    sed -e '/$(DEF7EXPORT17)/d' $@ 	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    sed -e '/$(DEF7EXPORT18)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    sed -e '/$(DEF7EXPORT19)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    sed -e '/$(DEF7EXPORT20)/d' $@  	> $(MISC)\$(SHL7TARGET).tmp
    copy $(MISC)\$(SHL7TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL7TARGET).tmp del $(MISC)\$(SHL7TARGET).tmp
.IF "$(DEF7EXPORT1)"!=""
    @echo $(DEF7EXPORT1)										>>$@
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    @echo $(DEF7EXPORT2)										>>$@
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    @echo $(DEF7EXPORT3)										>>$@
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    @echo $(DEF7EXPORT4)										>>$@
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    @echo $(DEF7EXPORT5)										>>$@
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    @echo $(DEF7EXPORT6)										>>$@
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    @echo $(DEF7EXPORT7)										>>$@
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    @echo $(DEF7EXPORT8)										>>$@
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    @echo $(DEF7EXPORT9)										>>$@
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    @echo $(DEF7EXPORT10)										>>$@
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    @echo $(DEF7EXPORT11)										>>$@
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    @echo $(DEF7EXPORT12)										>>$@
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    @echo $(DEF7EXPORT13)										>>$@
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    @echo $(DEF7EXPORT14)										>>$@
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    @echo $(DEF7EXPORT15)										>>$@
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    @echo $(DEF7EXPORT16)										>>$@
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    @echo $(DEF7EXPORT17)										>>$@
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    @echo $(DEF7EXPORT18)										>>$@
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    @echo $(DEF7EXPORT19)										>>$@
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    @echo $(DEF7EXPORT20)										>>$@
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
    $(TYPE) $(DEF7EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL7TARGET).map $(TOUCH) shl7.don
    if exist shl7.don echo  FUNCTIONS											>>$@
    if exist shl7.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL7TARGET).map
    if exist shl7.don (id2name $(SHL7TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl7.don del shl7.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP7HEAP)"!=""
    @echo NAME			$(APP7TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP7DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP7HEAP)								>>$@
    @echo STACKSIZE		$(APP7STACK)							>>$@
.ELSE			# "$(APP7HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL7TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF7EXPORT1)"!=""
    @echo $(DEF7EXPORT1)										>>$@
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    @echo $(DEF7EXPORT2)										>>$@
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    @echo $(DEF7EXPORT3)										>>$@
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    @echo $(DEF7EXPORT4)										>>$@
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    @echo $(DEF7EXPORT5)										>>$@
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    @echo $(DEF7EXPORT6)										>>$@
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    @echo $(DEF7EXPORT7)										>>$@
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    @echo $(DEF7EXPORT8)										>>$@
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    @echo $(DEF7EXPORT9)										>>$@
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    @echo $(DEF7EXPORT10)										>>$@
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    @echo $(DEF7EXPORT11)										>>$@
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    @echo $(DEF7EXPORT12)										>>$@
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    @echo $(DEF7EXPORT13)										>>$@
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    @echo $(DEF7EXPORT14)										>>$@
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    @echo $(DEF7EXPORT15)										>>$@
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    @echo $(DEF7EXPORT16)										>>$@
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    @echo $(DEF7EXPORT17)										>>$@
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    @echo $(DEF7EXPORT18)										>>$@
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    @echo $(DEF7EXPORT19)										>>$@
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    @echo $(DEF7EXPORT20)										>>$@
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
    $(TYPE) $(DEF7EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB7NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB7NAME)"!=""
.IF "$(DEF7CEXP)"!=""
    @$(LDUMP) -A $(DEF7CEXP) -E1 -F$(MISC)\$(SHL7TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF7CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL7TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF7CEXP)"!=""
.ENDIF			# "$(DEFLIB7NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB7NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB7NAME).lib  		>>tmp.cpf
.IF "$(DEF7CEXP)"!=""
    $(LDUMP) -A $(DEF7CEXP) -E1 -F$(MISC)\$(SHL7TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF7CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL7TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF7CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL7TARGET).flt $(SLB)\$(DEFLIB7NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB7NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL7TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB7NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL7TARGET).flt $(SHL7LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP7HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP7HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF7TARGETN) : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF7TARGETN) .PHONY : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL7TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB7NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL7TARGET).exp $(SLB)\$(DEFLIB7NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF7CEXP)"!=""
    @$(LDUMP2) -A $(DEF7CEXP) -E 20 -F $(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL7TARGET).exp
.ENDIF				# "$(DEFLIB7NAME)"!=""
.IF "$(DEF7EXPORT1)"!=""
    @echo $(DEF7EXPORT1)										>>$@
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    @echo $(DEF7EXPORT2)										>>$@
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    @echo $(DEF7EXPORT3)										>>$@
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    @echo $(DEF7EXPORT4)										>>$@
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    @echo $(DEF7EXPORT5)										>>$@
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    @echo $(DEF7EXPORT6)										>>$@
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    @echo $(DEF7EXPORT7)										>>$@
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    @echo $(DEF7EXPORT8)										>>$@
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    @echo $(DEF7EXPORT9)										>>$@
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    @echo $(DEF7EXPORT10)										>>$@
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    @echo $(DEF7EXPORT11)										>>$@
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    @echo $(DEF7EXPORT12)										>>$@
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    @echo $(DEF7EXPORT13)										>>$@
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    @echo $(DEF7EXPORT14)										>>$@
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    @echo $(DEF7EXPORT15)										>>$@
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    @echo $(DEF7EXPORT16)										>>$@
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    @echo $(DEF7EXPORT17)										>>$@
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    @echo $(DEF7EXPORT18)										>>$@
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    @echo $(DEF7EXPORT19)										>>$@
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    @echo $(DEF7EXPORT20)										>>$@
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
    $(TYPE) $(DEF7EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP7HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF7TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF8TARGETN)"!=""

.IF "$(DEFLIB8NAME)"!=""
.IF "$(UPDATER)"!=""
DEF8DEPN+=$(SLB)$/$(DEFLIB8NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL8TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB8NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL8TARGET).flt $(SLB)\$(DEFLIB8NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL8TARGET).dll 							>>$@
.IF "$(DEFLIB8NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL8TARGET).flt $(SHL8LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF8EXPORT1)"!=""
    sed -e '/$(DEF8EXPORT1)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    sed -e '/$(DEF8EXPORT2)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    sed -e '/$(DEF8EXPORT3)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    sed -e '/$(DEF8EXPORT4)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    sed -e '/$(DEF8EXPORT5)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    sed -e '/$(DEF8EXPORT6)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    sed -e '/$(DEF8EXPORT7)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    sed -e '/$(DEF8EXPORT8)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    sed -e '/$(DEF8EXPORT9)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    sed -e '/$(DEF8EXPORT10)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    sed -e '/$(DEF8EXPORT11)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    sed -e '/$(DEF8EXPORT12)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    sed -e '/$(DEF8EXPORT13)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    sed -e '/$(DEF8EXPORT14)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    sed -e '/$(DEF8EXPORT15)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    sed -e '/$(DEF8EXPORT16)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    sed -e '/$(DEF8EXPORT17)/d' $@ 	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    sed -e '/$(DEF8EXPORT18)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    sed -e '/$(DEF8EXPORT19)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    sed -e '/$(DEF8EXPORT20)/d' $@  	> $(MISC)\$(SHL8TARGET).tmp
    copy $(MISC)\$(SHL8TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL8TARGET).tmp del $(MISC)\$(SHL8TARGET).tmp
.IF "$(DEF8EXPORT1)"!=""
    @echo $(DEF8EXPORT1)										>>$@
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    @echo $(DEF8EXPORT2)										>>$@
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    @echo $(DEF8EXPORT3)										>>$@
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    @echo $(DEF8EXPORT4)										>>$@
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    @echo $(DEF8EXPORT5)										>>$@
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    @echo $(DEF8EXPORT6)										>>$@
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    @echo $(DEF8EXPORT7)										>>$@
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    @echo $(DEF8EXPORT8)										>>$@
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    @echo $(DEF8EXPORT9)										>>$@
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    @echo $(DEF8EXPORT10)										>>$@
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    @echo $(DEF8EXPORT11)										>>$@
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    @echo $(DEF8EXPORT12)										>>$@
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    @echo $(DEF8EXPORT13)										>>$@
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    @echo $(DEF8EXPORT14)										>>$@
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    @echo $(DEF8EXPORT15)										>>$@
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    @echo $(DEF8EXPORT16)										>>$@
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    @echo $(DEF8EXPORT17)										>>$@
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    @echo $(DEF8EXPORT18)										>>$@
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    @echo $(DEF8EXPORT19)										>>$@
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    @echo $(DEF8EXPORT20)										>>$@
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
    $(TYPE) $(DEF8EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL8TARGET).map $(TOUCH) shl8.don
    if exist shl8.don echo  FUNCTIONS											>>$@
    if exist shl8.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL8TARGET).map
    if exist shl8.don (id2name $(SHL8TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl8.don del shl8.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP8HEAP)"!=""
    @echo NAME			$(APP8TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP8DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP8HEAP)								>>$@
    @echo STACKSIZE		$(APP8STACK)							>>$@
.ELSE			# "$(APP8HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL8TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF8EXPORT1)"!=""
    @echo $(DEF8EXPORT1)										>>$@
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    @echo $(DEF8EXPORT2)										>>$@
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    @echo $(DEF8EXPORT3)										>>$@
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    @echo $(DEF8EXPORT4)										>>$@
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    @echo $(DEF8EXPORT5)										>>$@
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    @echo $(DEF8EXPORT6)										>>$@
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    @echo $(DEF8EXPORT7)										>>$@
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    @echo $(DEF8EXPORT8)										>>$@
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    @echo $(DEF8EXPORT9)										>>$@
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    @echo $(DEF8EXPORT10)										>>$@
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    @echo $(DEF8EXPORT11)										>>$@
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    @echo $(DEF8EXPORT12)										>>$@
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    @echo $(DEF8EXPORT13)										>>$@
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    @echo $(DEF8EXPORT14)										>>$@
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    @echo $(DEF8EXPORT15)										>>$@
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    @echo $(DEF8EXPORT16)										>>$@
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    @echo $(DEF8EXPORT17)										>>$@
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    @echo $(DEF8EXPORT18)										>>$@
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    @echo $(DEF8EXPORT19)										>>$@
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    @echo $(DEF8EXPORT20)										>>$@
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
    $(TYPE) $(DEF8EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB8NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB8NAME)"!=""
.IF "$(DEF8CEXP)"!=""
    @$(LDUMP) -A $(DEF8CEXP) -E1 -F$(MISC)\$(SHL8TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF8CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL8TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF8CEXP)"!=""
.ENDIF			# "$(DEFLIB8NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB8NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB8NAME).lib  		>>tmp.cpf
.IF "$(DEF8CEXP)"!=""
    $(LDUMP) -A $(DEF8CEXP) -E1 -F$(MISC)\$(SHL8TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF8CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL8TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF8CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL8TARGET).flt $(SLB)\$(DEFLIB8NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB8NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL8TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB8NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL8TARGET).flt $(SHL8LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP8HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP8HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF8TARGETN) : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF8TARGETN) .PHONY : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL8TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB8NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL8TARGET).exp $(SLB)\$(DEFLIB8NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF8CEXP)"!=""
    @$(LDUMP2) -A $(DEF8CEXP) -E 20 -F $(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL8TARGET).exp
.ENDIF				# "$(DEFLIB8NAME)"!=""
.IF "$(DEF8EXPORT1)"!=""
    @echo $(DEF8EXPORT1)										>>$@
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    @echo $(DEF8EXPORT2)										>>$@
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    @echo $(DEF8EXPORT3)										>>$@
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    @echo $(DEF8EXPORT4)										>>$@
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    @echo $(DEF8EXPORT5)										>>$@
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    @echo $(DEF8EXPORT6)										>>$@
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    @echo $(DEF8EXPORT7)										>>$@
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    @echo $(DEF8EXPORT8)										>>$@
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    @echo $(DEF8EXPORT9)										>>$@
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    @echo $(DEF8EXPORT10)										>>$@
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    @echo $(DEF8EXPORT11)										>>$@
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    @echo $(DEF8EXPORT12)										>>$@
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    @echo $(DEF8EXPORT13)										>>$@
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    @echo $(DEF8EXPORT14)										>>$@
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    @echo $(DEF8EXPORT15)										>>$@
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    @echo $(DEF8EXPORT16)										>>$@
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    @echo $(DEF8EXPORT17)										>>$@
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    @echo $(DEF8EXPORT18)										>>$@
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    @echo $(DEF8EXPORT19)										>>$@
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    @echo $(DEF8EXPORT20)										>>$@
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
    $(TYPE) $(DEF8EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP8HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF8TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF9TARGETN)"!=""

.IF "$(DEFLIB9NAME)"!=""
.IF "$(UPDATER)"!=""
DEF9DEPN+=$(SLB)$/$(DEFLIB9NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL9TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB9NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL9TARGET).flt $(SLB)\$(DEFLIB9NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL9TARGET).dll 							>>$@
.IF "$(DEFLIB9NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL9TARGET).flt $(SHL9LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF9EXPORT1)"!=""
    sed -e '/$(DEF9EXPORT1)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    sed -e '/$(DEF9EXPORT2)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    sed -e '/$(DEF9EXPORT3)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    sed -e '/$(DEF9EXPORT4)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    sed -e '/$(DEF9EXPORT5)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    sed -e '/$(DEF9EXPORT6)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    sed -e '/$(DEF9EXPORT7)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    sed -e '/$(DEF9EXPORT8)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    sed -e '/$(DEF9EXPORT9)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    sed -e '/$(DEF9EXPORT10)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    sed -e '/$(DEF9EXPORT11)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    sed -e '/$(DEF9EXPORT12)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    sed -e '/$(DEF9EXPORT13)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    sed -e '/$(DEF9EXPORT14)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    sed -e '/$(DEF9EXPORT15)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    sed -e '/$(DEF9EXPORT16)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    sed -e '/$(DEF9EXPORT17)/d' $@ 	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    sed -e '/$(DEF9EXPORT18)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    sed -e '/$(DEF9EXPORT19)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    sed -e '/$(DEF9EXPORT20)/d' $@  	> $(MISC)\$(SHL9TARGET).tmp
    copy $(MISC)\$(SHL9TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL9TARGET).tmp del $(MISC)\$(SHL9TARGET).tmp
.IF "$(DEF9EXPORT1)"!=""
    @echo $(DEF9EXPORT1)										>>$@
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    @echo $(DEF9EXPORT2)										>>$@
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    @echo $(DEF9EXPORT3)										>>$@
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    @echo $(DEF9EXPORT4)										>>$@
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    @echo $(DEF9EXPORT5)										>>$@
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    @echo $(DEF9EXPORT6)										>>$@
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    @echo $(DEF9EXPORT7)										>>$@
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    @echo $(DEF9EXPORT8)										>>$@
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    @echo $(DEF9EXPORT9)										>>$@
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    @echo $(DEF9EXPORT10)										>>$@
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    @echo $(DEF9EXPORT11)										>>$@
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    @echo $(DEF9EXPORT12)										>>$@
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    @echo $(DEF9EXPORT13)										>>$@
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    @echo $(DEF9EXPORT14)										>>$@
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    @echo $(DEF9EXPORT15)										>>$@
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    @echo $(DEF9EXPORT16)										>>$@
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    @echo $(DEF9EXPORT17)										>>$@
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    @echo $(DEF9EXPORT18)										>>$@
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    @echo $(DEF9EXPORT19)										>>$@
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    @echo $(DEF9EXPORT20)										>>$@
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
    $(TYPE) $(DEF9EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL9TARGET).map $(TOUCH) shl9.don
    if exist shl9.don echo  FUNCTIONS											>>$@
    if exist shl9.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL9TARGET).map
    if exist shl9.don (id2name $(SHL9TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl9.don del shl9.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP9HEAP)"!=""
    @echo NAME			$(APP9TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP9DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP9HEAP)								>>$@
    @echo STACKSIZE		$(APP9STACK)							>>$@
.ELSE			# "$(APP9HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL9TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF9EXPORT1)"!=""
    @echo $(DEF9EXPORT1)										>>$@
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    @echo $(DEF9EXPORT2)										>>$@
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    @echo $(DEF9EXPORT3)										>>$@
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    @echo $(DEF9EXPORT4)										>>$@
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    @echo $(DEF9EXPORT5)										>>$@
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    @echo $(DEF9EXPORT6)										>>$@
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    @echo $(DEF9EXPORT7)										>>$@
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    @echo $(DEF9EXPORT8)										>>$@
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    @echo $(DEF9EXPORT9)										>>$@
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    @echo $(DEF9EXPORT10)										>>$@
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    @echo $(DEF9EXPORT11)										>>$@
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    @echo $(DEF9EXPORT12)										>>$@
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    @echo $(DEF9EXPORT13)										>>$@
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    @echo $(DEF9EXPORT14)										>>$@
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    @echo $(DEF9EXPORT15)										>>$@
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    @echo $(DEF9EXPORT16)										>>$@
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    @echo $(DEF9EXPORT17)										>>$@
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    @echo $(DEF9EXPORT18)										>>$@
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    @echo $(DEF9EXPORT19)										>>$@
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    @echo $(DEF9EXPORT20)										>>$@
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
    $(TYPE) $(DEF9EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB9NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB9NAME)"!=""
.IF "$(DEF9CEXP)"!=""
    @$(LDUMP) -A $(DEF9CEXP) -E1 -F$(MISC)\$(SHL9TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF9CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL9TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF9CEXP)"!=""
.ENDIF			# "$(DEFLIB9NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB9NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB9NAME).lib  		>>tmp.cpf
.IF "$(DEF9CEXP)"!=""
    $(LDUMP) -A $(DEF9CEXP) -E1 -F$(MISC)\$(SHL9TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF9CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL9TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF9CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL9TARGET).flt $(SLB)\$(DEFLIB9NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB9NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL9TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB9NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL9TARGET).flt $(SHL9LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP9HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP9HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF9TARGETN) : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF9TARGETN) .PHONY : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL9TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB9NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL9TARGET).exp $(SLB)\$(DEFLIB9NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF9CEXP)"!=""
    @$(LDUMP2) -A $(DEF9CEXP) -E 20 -F $(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL9TARGET).exp
.ENDIF				# "$(DEFLIB9NAME)"!=""
.IF "$(DEF9EXPORT1)"!=""
    @echo $(DEF9EXPORT1)										>>$@
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    @echo $(DEF9EXPORT2)										>>$@
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    @echo $(DEF9EXPORT3)										>>$@
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    @echo $(DEF9EXPORT4)										>>$@
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    @echo $(DEF9EXPORT5)										>>$@
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    @echo $(DEF9EXPORT6)										>>$@
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    @echo $(DEF9EXPORT7)										>>$@
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    @echo $(DEF9EXPORT8)										>>$@
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    @echo $(DEF9EXPORT9)										>>$@
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    @echo $(DEF9EXPORT10)										>>$@
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    @echo $(DEF9EXPORT11)										>>$@
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    @echo $(DEF9EXPORT12)										>>$@
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    @echo $(DEF9EXPORT13)										>>$@
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    @echo $(DEF9EXPORT14)										>>$@
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    @echo $(DEF9EXPORT15)										>>$@
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    @echo $(DEF9EXPORT16)										>>$@
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    @echo $(DEF9EXPORT17)										>>$@
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    @echo $(DEF9EXPORT18)										>>$@
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    @echo $(DEF9EXPORT19)										>>$@
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    @echo $(DEF9EXPORT20)										>>$@
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
    $(TYPE) $(DEF9EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP9HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF9TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF10TARGETN)"!=""

.IF "$(DEFLIB10NAME)"!=""
.IF "$(UPDATER)"!=""
DEF10DEPN+=$(SLB)$/$(DEFLIB10NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL10TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB10NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL10TARGET).flt $(SLB)\$(DEFLIB10NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL10TARGET).dll 							>>$@
.IF "$(DEFLIB10NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL10TARGET).flt $(SHL10LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF10EXPORT1)"!=""
    sed -e '/$(DEF10EXPORT1)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    sed -e '/$(DEF10EXPORT2)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    sed -e '/$(DEF10EXPORT3)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    sed -e '/$(DEF10EXPORT4)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    sed -e '/$(DEF10EXPORT5)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    sed -e '/$(DEF10EXPORT6)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    sed -e '/$(DEF10EXPORT7)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    sed -e '/$(DEF10EXPORT8)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    sed -e '/$(DEF10EXPORT9)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    sed -e '/$(DEF10EXPORT10)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    sed -e '/$(DEF10EXPORT11)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    sed -e '/$(DEF10EXPORT12)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    sed -e '/$(DEF10EXPORT13)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    sed -e '/$(DEF10EXPORT14)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    sed -e '/$(DEF10EXPORT15)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    sed -e '/$(DEF10EXPORT16)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    sed -e '/$(DEF10EXPORT17)/d' $@ 	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    sed -e '/$(DEF10EXPORT18)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    sed -e '/$(DEF10EXPORT19)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    sed -e '/$(DEF10EXPORT20)/d' $@  	> $(MISC)\$(SHL10TARGET).tmp
    copy $(MISC)\$(SHL10TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL10TARGET).tmp del $(MISC)\$(SHL10TARGET).tmp
.IF "$(DEF10EXPORT1)"!=""
    @echo $(DEF10EXPORT1)										>>$@
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    @echo $(DEF10EXPORT2)										>>$@
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    @echo $(DEF10EXPORT3)										>>$@
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    @echo $(DEF10EXPORT4)										>>$@
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    @echo $(DEF10EXPORT5)										>>$@
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    @echo $(DEF10EXPORT6)										>>$@
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    @echo $(DEF10EXPORT7)										>>$@
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    @echo $(DEF10EXPORT8)										>>$@
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    @echo $(DEF10EXPORT9)										>>$@
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    @echo $(DEF10EXPORT10)										>>$@
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    @echo $(DEF10EXPORT11)										>>$@
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    @echo $(DEF10EXPORT12)										>>$@
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    @echo $(DEF10EXPORT13)										>>$@
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    @echo $(DEF10EXPORT14)										>>$@
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    @echo $(DEF10EXPORT15)										>>$@
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    @echo $(DEF10EXPORT16)										>>$@
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    @echo $(DEF10EXPORT17)										>>$@
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    @echo $(DEF10EXPORT18)										>>$@
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    @echo $(DEF10EXPORT19)										>>$@
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    @echo $(DEF10EXPORT20)										>>$@
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
    $(TYPE) $(DEF10EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL10TARGET).map $(TOUCH) shl10.don
    if exist shl10.don echo  FUNCTIONS											>>$@
    if exist shl10.don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL10TARGET).map
    if exist shl10.don (id2name $(SHL10TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl10.don del shl10.don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP10HEAP)"!=""
    @echo NAME			$(APP10TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP10DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP10HEAP)								>>$@
    @echo STACKSIZE		$(APP10STACK)							>>$@
.ELSE			# "$(APP10HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL10TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
#	getDescriptionFunc fuer alle!?
.IF "$(DESCRIPTION)"!=""
    @echo getDescriptionFunc	>>$@
.ENDIF			# "$(DESCRIPTION)"!=""
.IF "$(DEF10EXPORT1)"!=""
    @echo $(DEF10EXPORT1)										>>$@
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    @echo $(DEF10EXPORT2)										>>$@
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    @echo $(DEF10EXPORT3)										>>$@
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    @echo $(DEF10EXPORT4)										>>$@
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    @echo $(DEF10EXPORT5)										>>$@
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    @echo $(DEF10EXPORT6)										>>$@
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    @echo $(DEF10EXPORT7)										>>$@
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    @echo $(DEF10EXPORT8)										>>$@
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    @echo $(DEF10EXPORT9)										>>$@
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    @echo $(DEF10EXPORT10)										>>$@
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    @echo $(DEF10EXPORT11)										>>$@
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    @echo $(DEF10EXPORT12)										>>$@
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    @echo $(DEF10EXPORT13)										>>$@
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    @echo $(DEF10EXPORT14)										>>$@
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    @echo $(DEF10EXPORT15)										>>$@
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    @echo $(DEF10EXPORT16)										>>$@
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    @echo $(DEF10EXPORT17)										>>$@
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    @echo $(DEF10EXPORT18)										>>$@
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    @echo $(DEF10EXPORT19)										>>$@
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    @echo $(DEF10EXPORT20)										>>$@
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
    $(TYPE) $(DEF10EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB10NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB10NAME)"!=""
.IF "$(DEF10CEXP)"!=""
    @$(LDUMP) -A $(DEF10CEXP) -E1 -F$(MISC)\$(SHL10TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF10CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL10TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF10CEXP)"!=""
.ENDIF			# "$(DEFLIB10NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB10NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB10NAME).lib  		>>tmp.cpf
.IF "$(DEF10CEXP)"!=""
    $(LDUMP) -A $(DEF10CEXP) -E1 -F$(MISC)\$(SHL10TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF10CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL10TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF10CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL10TARGET).flt $(SLB)\$(DEFLIB10NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB10NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL10TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB10NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL10TARGET).flt $(SHL10LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP10HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP10HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF10TARGETN) : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF10TARGETN) .PHONY : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL10TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB10NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL10TARGET).exp $(SLB)\$(DEFLIB10NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF10CEXP)"!=""
    @$(LDUMP2) -A $(DEF10CEXP) -E 20 -F $(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL10TARGET).exp
.ENDIF				# "$(DEFLIB10NAME)"!=""
.IF "$(DEF10EXPORT1)"!=""
    @echo $(DEF10EXPORT1)										>>$@
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    @echo $(DEF10EXPORT2)										>>$@
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    @echo $(DEF10EXPORT3)										>>$@
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    @echo $(DEF10EXPORT4)										>>$@
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    @echo $(DEF10EXPORT5)										>>$@
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    @echo $(DEF10EXPORT6)										>>$@
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    @echo $(DEF10EXPORT7)										>>$@
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    @echo $(DEF10EXPORT8)										>>$@
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    @echo $(DEF10EXPORT9)										>>$@
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    @echo $(DEF10EXPORT10)										>>$@
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    @echo $(DEF10EXPORT11)										>>$@
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    @echo $(DEF10EXPORT12)										>>$@
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    @echo $(DEF10EXPORT13)										>>$@
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    @echo $(DEF10EXPORT14)										>>$@
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    @echo $(DEF10EXPORT15)										>>$@
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    @echo $(DEF10EXPORT16)										>>$@
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    @echo $(DEF10EXPORT17)										>>$@
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    @echo $(DEF10EXPORT18)										>>$@
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    @echo $(DEF10EXPORT19)										>>$@
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    @echo $(DEF10EXPORT20)										>>$@
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
    $(TYPE) $(DEF10EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP10HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
