#*************************************************************************
#*
#*    $Workfile:   tg_slo.mk  $
#*
#*    Ersterstellung    MH 01.09.97
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:18 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/tg_slo.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
#*************************************************************************

MKFILENAME:=TG_SLO.MK

.IF "$(SLOTARGET)"!=""
$(SLOTARGET): $(SLOFILES) $(IDLSLOFILES)
.IF "$(MDB)" != ""
    @echo $(SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
#	@+$(RM) $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(IDLSLOFILES:f) $(RSLO)$/$(IDLPACKAGE)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
    +echo $(foreach,i,$(SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="MAC"
    +echo $(foreach,i,$(IDLSLOFILES:f) $(RSLO)$/$(IDLPACKAGE)$/$(i)) | xargs -n1 > $@
    +echo $(foreach,i,$(SLOFILES:f) $(RSLO)$/$(i)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(SLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ELSE
    $(LIBMGR) r $@ $(SLOFILES)
.ENDIF			# "$(COM)"=="ICC"
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(SLOTARGET)"!=""


.IF "$(SMRSLOTARGET)"!=""
$(SMRSLOTARGET):  $(SMRSLOFILES)
.IF "$(MDB)" != ""
    @+echo $(SMRSLOTARGET:s/ttt/\/)
    @+echo $(SMRSLOFILES)
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n":s/ttt/\/)) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$(@:s/ttt/\/) @$(mktmp $(&:+"\n":s/ttt/\/))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(SMRSLOFILES:f:s/ttt/\/) $(RPACKAGESLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LIBMGR) $(@:s/ttt/\/) $(LIBFLAGS) +$(SMRSLOFILES:+"\n+":^"&":s/ttt/\/)
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $(@:s/ttt/\/)
    $(LIBMGR) $(LIBFLAGS) $(@:s/ttt/\/) @$(mktmp $(&:+"&\n":s/ttt/\/);)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(SMRSLOTARGET)"!=""


.IF "$(S2USLOTARGET)"!=""
$(S2USLOTARGET): $(S2USLOFILES:s/ttt/\/)
.IF "$(MDB)" != ""
    @echo $(S2USLOTARGET)
    @echo $(&:+"\n":s/ttt/\/)
.ENDIF
    @echo ------------------------------
    @echo Making: $(@:s/ttt/\/)
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n":s/ttt/\/)) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$(@:s/ttt/\/) @$(mktmp $(&:+"\n":s/ttt/\/))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(S2USLOFILES:f:s/ttt/\/) $(RPACKAGESLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LIBMGR) $(@:s/ttt/\/) $(LIBFLAGS) +$(S2USLOFILES:+"\n+":^"&":s/ttt/\/)
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $(@:s/ttt/\/)
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n":s/ttt/\/);)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(S2USLOTARGET)"!=""


.IF "$(SVXLIGHTSLOTARGET)"!=""
$(SVXLIGHTSLOTARGET): $(REAL_SVXLIGHTSLOFILES)
.IF "$(MDB)" != ""
    @echo $(REAL_SVXLIGHTSLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_SVXLIGHTSLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_SVXLIGHTSLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(SVXLIGHTSLOTARGET)"!=""

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)SLOTARGET)"!=""
$($(SECOND_BUILD)SLOTARGET): $(REAL_$(SECOND_BUILD)SLOFILES)
.IF "$(MDB)" != ""
    @echo $(REAL_$(SECOND_BUILD)SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_$(SECOND_BUILD)SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_$(SECOND_BUILD)SLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$($(SECOND_BUILD)SLOTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

