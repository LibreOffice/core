#*************************************************************************
#*
#*    $Workfile:   tg_def.mk  $
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:17 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/tg_def.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
#*************************************************************************


# -------
# - DEF -
# -------
.IF "$(MULTI_DEF_FLAG)" == ""
$(DEF1TARGETN) .NULL : DEF1

$(DEF2TARGETN) .NULL : DEF2

$(DEF3TARGETN) .NULL : DEF3

$(DEF4TARGETN) .NULL : DEF4

$(DEF5TARGETN) .NULL : DEF5

$(DEF6TARGETN) .NULL : DEF6

$(DEF7TARGETN) .NULL : DEF7

$(DEF8TARGETN) .NULL : DEF8

$(DEF9TARGETN) .NULL : DEF9
.ENDIF



.IF "$(MULTI_DEF_FLAG)"==""
DEF1 DEF2 DEF3 DEF4 DEF5 DEF6 DEF7 DEF8 DEF9:  \
                $($@:+"DEPN")
#				$(SLB)$/$($(@:s/DEF/DEFLIB/:+"NAME")).lib
.IF "$(GUI)"=="MAC"
    @dmake "$(DEF$(TNR)TARGETN)" MULTI_DEF_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @dmake $(DEF$(TNR)TARGETN) MULTI_DEF_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE			# "$(MULTI_DEF_FLAG)"==""


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF$(TNR)TARGETN)"!=""

.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(UPDATER)"!=""
DEF$(TNR)DEPN+=$(SLB)$/$(DEFLIB$(TNR)NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WIN"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(COM)" == "BLC"
    @echo name $(BIN)$/$(SHL$(TNR)TARGET).dll 							>>$@
    @echo DESCRIPTION 'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR) ' >$@
    @echo EXETYPE		WINDOWS 								>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL MOVEABLE DISCARDABLE 		>>$@
    @echo DATA			PRELOAD MOVEABLE SINGLE 				>>$@
    @echo HEAPSIZE		0										>>$@
    @echo EXPORTS												>>$@
.IF "$(DEFLIB$(TNR)NAME)"!=""
        +ldump -E2 -A -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SLB)\$(DEFLIB$(TNR)NAME).lib         >>$@
        @echo 	WEP	>>$@
.ENDIF
.ELSE			# "$(COM)" == "BLC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)$/$(SHL$(TNR)TARGET).dll 							>>$@
.IF "$(DEFLIB$(TNR)NAME)"!=""
    +r:\solenv\wini\ldump -A -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)LIBS)	>>tmp.def
    @+gawk -f r:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF
.ENDIF			# "$(COM)" == "BLC"
.IF "$(DEF$(TNR)EXPORT1)"!=""
    sed -e '/$(DEF$(TNR)EXPORT1)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    sed -e '/$(DEF$(TNR)EXPORT2)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    sed -e '/$(DEF$(TNR)EXPORT3)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    sed -e '/$(DEF$(TNR)EXPORT4)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    sed -e '/$(DEF$(TNR)EXPORT5)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    sed -e '/$(DEF$(TNR)EXPORT6)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    sed -e '/$(DEF$(TNR)EXPORT7)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    sed -e '/$(DEF$(TNR)EXPORT8)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    sed -e '/$(DEF$(TNR)EXPORT9)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    sed -e '/$(DEF$(TNR)EXPORT10)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    sed -e '/$(DEF$(TNR)EXPORT11)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    sed -e '/$(DEF$(TNR)EXPORT12)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    sed -e '/$(DEF$(TNR)EXPORT13)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    sed -e '/$(DEF$(TNR)EXPORT14)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    sed -e '/$(DEF$(TNR)EXPORT15)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    sed -e '/$(DEF$(TNR)EXPORT16)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    sed -e '/$(DEF$(TNR)EXPORT17)/d' $@ 	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    sed -e '/$(DEF$(TNR)EXPORT18)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    sed -e '/$(DEF$(TNR)EXPORT19)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    sed -e '/$(DEF$(TNR)EXPORT20)/d' $@  	> $(MISC)\$(SHL$(TNR)TARGET).tmp
    copy $(MISC)\$(SHL$(TNR)TARGET).tmp $@
.ENDIF
    @+if exist $(MISC)\$(SHL$(TNR)TARGET).tmp del $(MISC)\$(SHL$(TNR)TARGET).tmp
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    $(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@
.ENDIF
.IF "$(funcord)" != ""
    if exist $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL$(TNR)TARGET).map $(TOUCH) shl$(TNR).don
    if exist shl$(TNR).don echo  FUNCTIONS											>>$@
    if exist shl$(TNR).don copy $(SRC_O)\$(UPD)\$(PRJNAME)\winmsci.siz\misc\$(SHL$(TNR)TARGET).map
    if exist shl$(TNR).don (id2name $(SHL$(TNR)TARGET).map | ford - ..\inc\segdefs_.hxx >>$@ )
    if exist shl$(TNR).don del shl$(TNR).don

.ENDIF			# "$(funcord)" != ""
.ENDIF			# "$(GUI)"=="WIN"

.IF "$(GUI)"=="OS2"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
.IF "$(APP$(TNR)HEAP)"!=""
    @echo NAME			$(APP$(TNR)TARGET) WINDOWAPI					>$@
    @echo DESCRIPTION	'$(APP$(TNR)DES) $(UPD)'						>>$@
.IF "$(COM)"!="BLC"
    @echo STUB			'os2stub.exe'							>>$@
.ENDIF
    @echo EXETYPE		OS2										>>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE						>>$@
    @echo HEAPSIZE		$(APP$(TNR)HEAP)								>>$@
    @echo STACKSIZE		$(APP$(TNR)STACK)							>>$@
.ELSE			# "$(APP$(TNR)HEAP)"!=""
.IF "$(COM)"!="WTC"
    @echo LIBRARY		$(SHL$(TNR)TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR)' >>$@
    @echo PROTMODE												>>$@
    @echo CODE			LOADONCALL								>>$@
    @echo DATA			PRELOAD MULTIPLE NONSHARED				>>$@
    @echo EXPORTS												>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    $(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@
.ENDIF
.IF "$(COM)"=="ICC"
.IF "$(COMEX)"=="3"
    @cppfilt  /b /p /p /n /o $(SLB)\$(DEFLIB$(TNR)NAME).lib  			>>tmp.cpf
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)CEXP)"!=""
    @$(LDUMP) -A $(DEF$(TNR)CEXP) -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF$(TNR)CEXP)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF$(TNR)CEXP)"!=""
.ENDIF			# "$(DEFLIB$(TNR)NAME)"!=""
    @-+$(RM) tmp.cpf
    @echo SEGMENTS												>>$@
    @echo  __MSGSEG32 CLASS 'CODE'								>>$@
.ENDIF			# "$(COMEX)"=="3"
.ELSE			# "$(COM)"=="ICC"
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(COM)"=="GCC"
    cppfilt  /b  /p /n /o $(SLB)\$(DEFLIB$(TNR)NAME).lib  		>>tmp.cpf
.IF "$(DEF$(TNR)CEXP)"!=""
    $(LDUMP) -A $(DEF$(TNR)CEXP) -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt tmp.cpf	>>$@
.ELSE			# "$(DEF$(TNR)CEXP)"!=""
    $(LDUMP) -A -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt tmp.cpf	        	>>$@
.ENDIF			# "$(DEF$(TNR)CEXP)"!=""
.ELSE			# "$(COM)"=="GCC"
    @$(LDUMP) -E1 -A -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SLB)\$(DEFLIB$(TNR)NAME).lib >>$@
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(DEFLIB$(TNR)NAME)"!=""
.ENDIF			# "$(COM)"=="ICC"
.ELSE			# "$(COM)"!="WTC"
    @echo option DESCRIPTION 'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR) ' >$@
    @echo name $(BIN)\$(SHL$(TNR)TARGET).dll 							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1				    	>>$@
.ENDIF
.IF "$(DEFLIB$(TNR)NAME)"!=""
    @$(LDUMP) -A -E1 -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)LIBS)			>>tmp.def
    @$(AWK) -f s:\util\exp.awk tmp.def								>>$@
    @-+$(RM) tmp.def
.ENDIF
.ENDIF			# "$(COM)"!="WTC"
.ENDIF			# "$(APP$(TNR)HEAP)"!=""
.ENDIF			# "$(GUI)"=="OS2"


.IF "$(GUI)"=="WNT"
.IF "$(APP$(TNR)HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL$(TNR)TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB$(TNR)NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL$(TNR)TARGET).exp $(SLB)\$(DEFLIB$(TNR)NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF$(TNR)CEXP)"!=""
    @$(LDUMP2) -A $(DEF$(TNR)CEXP) -E 20 -F $(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL$(TNR)TARGET).exp
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    $(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP$(TNR)HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer UNIX nur dummy
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF			# "$(MULTI_DEF_FLAG)"==""

