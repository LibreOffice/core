# unroll begin

.IF "$(LIB1TARGET)" != ""
.IF "$(LIB1ARCHIV)" != ""

$(LIB1ARCHIV) :	$(LIB1TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB1ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB1FLAGS) $(LIBFLAGS) $(LIB1ARCHIV) `cat $(LIB1TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB1ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB1ARCHIV) >> $(MISC)/$(LIB1ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB1ARCHIV) >> $(MISC)/$(LIB1ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB1ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB1ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB1ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB1FLAGS) $(LIBFLAGS) $(LIB1ARCHIV) `cat $(LIB1TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB1ARCHIV:b).cmd
    @+echo  ranlib $(LIB1ARCHIV) >> $(MISC)/$(LIB1ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB1ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB1ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB1ARCHIV)" != ""

$(LIB1TARGET) :	$(LIB1FILES) \
                        $(LIB1OBJFILES) \
                        $(LIB1DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB1FILES)
    @echo using: $(LIB1TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB1OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB1FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB1TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB1TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB1FILES) $(LIB1OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB1OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB1OBJFILES)) > $(null,$(LIB1OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB1OBJFILES)"!=""    
.IF "$(LIB1FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB1FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB1FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB1OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB1FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB1FILES) $(LIB1OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB1OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB1OBJFILES)) > $(null,$(LIB1OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB1OBJFILES)"!=""    
.IF "$(LIB1FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB1FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB1FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB1FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB1OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB1OBJFILES) $(LIB1FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB1TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB2TARGET)" != ""
.IF "$(LIB2ARCHIV)" != ""

$(LIB2ARCHIV) :	$(LIB2TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB2ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB2FLAGS) $(LIBFLAGS) $(LIB2ARCHIV) `cat $(LIB2TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB2ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB2ARCHIV) >> $(MISC)/$(LIB2ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB2ARCHIV) >> $(MISC)/$(LIB2ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB2ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB2ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB2ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB2FLAGS) $(LIBFLAGS) $(LIB2ARCHIV) `cat $(LIB2TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB2ARCHIV:b).cmd
    @+echo  ranlib $(LIB2ARCHIV) >> $(MISC)/$(LIB2ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB2ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB2ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB2ARCHIV)" != ""

$(LIB2TARGET) :	$(LIB2FILES) \
                        $(LIB2OBJFILES) \
                        $(LIB2DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB2FILES)
    @echo using: $(LIB2TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB2OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB2FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB2TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB2TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB2FILES) $(LIB2OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB2OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB2OBJFILES)) > $(null,$(LIB2OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB2OBJFILES)"!=""    
.IF "$(LIB2FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB2FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB2FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB2OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB2FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB2FILES) $(LIB2OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB2OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB2OBJFILES)) > $(null,$(LIB2OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB2OBJFILES)"!=""    
.IF "$(LIB2FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB2FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB2FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB2FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB2OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB2OBJFILES) $(LIB2FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB2TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB3TARGET)" != ""
.IF "$(LIB3ARCHIV)" != ""

$(LIB3ARCHIV) :	$(LIB3TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB3ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB3FLAGS) $(LIBFLAGS) $(LIB3ARCHIV) `cat $(LIB3TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB3ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB3ARCHIV) >> $(MISC)/$(LIB3ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB3ARCHIV) >> $(MISC)/$(LIB3ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB3ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB3ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB3ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB3FLAGS) $(LIBFLAGS) $(LIB3ARCHIV) `cat $(LIB3TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB3ARCHIV:b).cmd
    @+echo  ranlib $(LIB3ARCHIV) >> $(MISC)/$(LIB3ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB3ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB3ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB3ARCHIV)" != ""

$(LIB3TARGET) :	$(LIB3FILES) \
                        $(LIB3OBJFILES) \
                        $(LIB3DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB3FILES)
    @echo using: $(LIB3TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB3OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB3FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB3TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB3TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB3FILES) $(LIB3OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB3OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB3OBJFILES)) > $(null,$(LIB3OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB3OBJFILES)"!=""    
.IF "$(LIB3FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB3FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB3FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB3OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB3FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB3FILES) $(LIB3OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB3OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB3OBJFILES)) > $(null,$(LIB3OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB3OBJFILES)"!=""    
.IF "$(LIB3FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB3FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB3FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB3FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB3OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB3OBJFILES) $(LIB3FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB3TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB4TARGET)" != ""
.IF "$(LIB4ARCHIV)" != ""

$(LIB4ARCHIV) :	$(LIB4TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB4ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB4FLAGS) $(LIBFLAGS) $(LIB4ARCHIV) `cat $(LIB4TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB4ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB4ARCHIV) >> $(MISC)/$(LIB4ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB4ARCHIV) >> $(MISC)/$(LIB4ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB4ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB4ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB4ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB4FLAGS) $(LIBFLAGS) $(LIB4ARCHIV) `cat $(LIB4TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB4ARCHIV:b).cmd
    @+echo  ranlib $(LIB4ARCHIV) >> $(MISC)/$(LIB4ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB4ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB4ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB4ARCHIV)" != ""

$(LIB4TARGET) :	$(LIB4FILES) \
                        $(LIB4OBJFILES) \
                        $(LIB4DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB4FILES)
    @echo using: $(LIB4TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB4OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB4FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB4TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB4TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB4FILES) $(LIB4OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB4OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB4OBJFILES)) > $(null,$(LIB4OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB4OBJFILES)"!=""    
.IF "$(LIB4FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB4FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB4FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB4OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB4FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB4FILES) $(LIB4OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB4OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB4OBJFILES)) > $(null,$(LIB4OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB4OBJFILES)"!=""    
.IF "$(LIB4FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB4FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB4FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB4FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB4OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB4OBJFILES) $(LIB4FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB4TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB5TARGET)" != ""
.IF "$(LIB5ARCHIV)" != ""

$(LIB5ARCHIV) :	$(LIB5TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB5ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB5FLAGS) $(LIBFLAGS) $(LIB5ARCHIV) `cat $(LIB5TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB5ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB5ARCHIV) >> $(MISC)/$(LIB5ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB5ARCHIV) >> $(MISC)/$(LIB5ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB5ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB5ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB5ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB5FLAGS) $(LIBFLAGS) $(LIB5ARCHIV) `cat $(LIB5TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB5ARCHIV:b).cmd
    @+echo  ranlib $(LIB5ARCHIV) >> $(MISC)/$(LIB5ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB5ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB5ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB5ARCHIV)" != ""

$(LIB5TARGET) :	$(LIB5FILES) \
                        $(LIB5OBJFILES) \
                        $(LIB5DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB5FILES)
    @echo using: $(LIB5TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB5OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB5FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB5TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB5TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB5FILES) $(LIB5OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB5OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB5OBJFILES)) > $(null,$(LIB5OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB5OBJFILES)"!=""    
.IF "$(LIB5FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB5FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB5FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB5OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB5FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB5FILES) $(LIB5OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB5OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB5OBJFILES)) > $(null,$(LIB5OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB5OBJFILES)"!=""    
.IF "$(LIB5FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB5FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB5FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB5FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB5OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB5OBJFILES) $(LIB5FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB5TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB6TARGET)" != ""
.IF "$(LIB6ARCHIV)" != ""

$(LIB6ARCHIV) :	$(LIB6TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB6ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB6FLAGS) $(LIBFLAGS) $(LIB6ARCHIV) `cat $(LIB6TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB6ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB6ARCHIV) >> $(MISC)/$(LIB6ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB6ARCHIV) >> $(MISC)/$(LIB6ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB6ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB6ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB6ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB6FLAGS) $(LIBFLAGS) $(LIB6ARCHIV) `cat $(LIB6TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB6ARCHIV:b).cmd
    @+echo  ranlib $(LIB6ARCHIV) >> $(MISC)/$(LIB6ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB6ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB6ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB6ARCHIV)" != ""

$(LIB6TARGET) :	$(LIB6FILES) \
                        $(LIB6OBJFILES) \
                        $(LIB6DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB6FILES)
    @echo using: $(LIB6TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB6OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB6FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB6TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB6TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB6FILES) $(LIB6OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB6OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB6OBJFILES)) > $(null,$(LIB6OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB6OBJFILES)"!=""    
.IF "$(LIB6FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB6FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB6FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB6OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB6FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB6FILES) $(LIB6OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB6OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB6OBJFILES)) > $(null,$(LIB6OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB6OBJFILES)"!=""    
.IF "$(LIB6FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB6FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB6FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB6FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB6OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB6OBJFILES) $(LIB6FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB6TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB7TARGET)" != ""
.IF "$(LIB7ARCHIV)" != ""

$(LIB7ARCHIV) :	$(LIB7TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB7ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB7FLAGS) $(LIBFLAGS) $(LIB7ARCHIV) `cat $(LIB7TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB7ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB7ARCHIV) >> $(MISC)/$(LIB7ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB7ARCHIV) >> $(MISC)/$(LIB7ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB7ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB7ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB7ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB7FLAGS) $(LIBFLAGS) $(LIB7ARCHIV) `cat $(LIB7TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB7ARCHIV:b).cmd
    @+echo  ranlib $(LIB7ARCHIV) >> $(MISC)/$(LIB7ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB7ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB7ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB7ARCHIV)" != ""

$(LIB7TARGET) :	$(LIB7FILES) \
                        $(LIB7OBJFILES) \
                        $(LIB7DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB7FILES)
    @echo using: $(LIB7TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB7OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB7FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB7TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB7TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB7FILES) $(LIB7OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB7OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB7OBJFILES)) > $(null,$(LIB7OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB7OBJFILES)"!=""    
.IF "$(LIB7FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB7FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB7FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB7OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB7FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB7FILES) $(LIB7OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB7OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB7OBJFILES)) > $(null,$(LIB7OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB7OBJFILES)"!=""    
.IF "$(LIB7FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB7FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB7FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB7FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB7OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB7OBJFILES) $(LIB7FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB7TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB8TARGET)" != ""
.IF "$(LIB8ARCHIV)" != ""

$(LIB8ARCHIV) :	$(LIB8TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB8ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB8FLAGS) $(LIBFLAGS) $(LIB8ARCHIV) `cat $(LIB8TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB8ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB8ARCHIV) >> $(MISC)/$(LIB8ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB8ARCHIV) >> $(MISC)/$(LIB8ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB8ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB8ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB8ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB8FLAGS) $(LIBFLAGS) $(LIB8ARCHIV) `cat $(LIB8TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB8ARCHIV:b).cmd
    @+echo  ranlib $(LIB8ARCHIV) >> $(MISC)/$(LIB8ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB8ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB8ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB8ARCHIV)" != ""

$(LIB8TARGET) :	$(LIB8FILES) \
                        $(LIB8OBJFILES) \
                        $(LIB8DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB8FILES)
    @echo using: $(LIB8TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB8OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB8FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB8TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB8TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB8FILES) $(LIB8OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB8OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB8OBJFILES)) > $(null,$(LIB8OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB8OBJFILES)"!=""    
.IF "$(LIB8FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB8FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB8FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB8OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB8FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB8FILES) $(LIB8OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB8OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB8OBJFILES)) > $(null,$(LIB8OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB8OBJFILES)"!=""    
.IF "$(LIB8FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB8FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB8FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB8FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB8OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB8OBJFILES) $(LIB8FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB8TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB9TARGET)" != ""
.IF "$(LIB9ARCHIV)" != ""

$(LIB9ARCHIV) :	$(LIB9TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB9ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB9FLAGS) $(LIBFLAGS) $(LIB9ARCHIV) `cat $(LIB9TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB9ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB9ARCHIV) >> $(MISC)/$(LIB9ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB9ARCHIV) >> $(MISC)/$(LIB9ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB9ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB9ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB9ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB9FLAGS) $(LIBFLAGS) $(LIB9ARCHIV) `cat $(LIB9TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB9ARCHIV:b).cmd
    @+echo  ranlib $(LIB9ARCHIV) >> $(MISC)/$(LIB9ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB9ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB9ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB9ARCHIV)" != ""

$(LIB9TARGET) :	$(LIB9FILES) \
                        $(LIB9OBJFILES) \
                        $(LIB9DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB9FILES)
    @echo using: $(LIB9TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB9OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB9FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB9TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB9TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB9FILES) $(LIB9OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB9OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB9OBJFILES)) > $(null,$(LIB9OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB9OBJFILES)"!=""    
.IF "$(LIB9FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB9FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB9FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB9OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB9FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB9FILES) $(LIB9OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB9OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB9OBJFILES)) > $(null,$(LIB9OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB9OBJFILES)"!=""    
.IF "$(LIB9FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB9FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB9FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB9FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB9OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB9OBJFILES) $(LIB9FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB9TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll begin

.IF "$(LIB10TARGET)" != ""
.IF "$(LIB10ARCHIV)" != ""

$(LIB10ARCHIV) :	$(LIB10TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB10ARCHIV:b).cmd
    @echo $(LIBMGR) $(LIB10FLAGS) $(LIBFLAGS) $(LIB10ARCHIV) `cat $(LIB10TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB10ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB10ARCHIV) >> $(MISC)/$(LIB10ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB10ARCHIV) >> $(MISC)/$(LIB10ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB10ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB10ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB10ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB10FLAGS) $(LIBFLAGS) $(LIB10ARCHIV) `cat $(LIB10TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB10ARCHIV:b).cmd
    @+echo  ranlib $(LIB10ARCHIV) >> $(MISC)/$(LIB10ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB10ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB10ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB10ARCHIV)" != ""

$(LIB10TARGET) :	$(LIB10FILES) \
                        $(LIB10OBJFILES) \
                        $(LIB10DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB10FILES)
    @echo using: $(LIB10TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB10OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB10FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB10TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB10TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB10FILES) $(LIB10OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB10OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB10OBJFILES)) > $(null,$(LIB10OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB10OBJFILES)"!=""    
.IF "$(LIB10FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB10FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB10FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB10OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB10FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB10FILES) $(LIB10OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB10OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB10OBJFILES)) > $(null,$(LIB10OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB10OBJFILES)"!=""    
.IF "$(LIB10FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB10FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB10FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB10FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB10OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB10OBJFILES) $(LIB10FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB10TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
