# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP1TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP1TARGET).dpzz : $(ZIP1TARGETN)

.IF "$(common_build_zip)"!=""
ZIP1HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP1HELPVAR=$(BIN)$/$(ZIP1TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP1TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP1TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP1LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP1LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP1LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP1LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP2TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP2TARGET).dpzz : $(ZIP2TARGETN)

.IF "$(common_build_zip)"!=""
ZIP2HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP2HELPVAR=$(BIN)$/$(ZIP2TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP2TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP2TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP2LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP2LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP2LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP2LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP3TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP3TARGET).dpzz : $(ZIP3TARGETN)

.IF "$(common_build_zip)"!=""
ZIP3HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP3HELPVAR=$(BIN)$/$(ZIP3TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP3TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP3TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP3LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP3LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP3LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP3LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP4TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP4TARGET).dpzz : $(ZIP4TARGETN)

.IF "$(common_build_zip)"!=""
ZIP4HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP4HELPVAR=$(BIN)$/$(ZIP4TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP4TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP4TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP4LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP4LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP4LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP4LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP5TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP5TARGET).dpzz : $(ZIP5TARGETN)

.IF "$(common_build_zip)"!=""
ZIP5HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP5HELPVAR=$(BIN)$/$(ZIP5TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP5TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP5TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP5LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP5LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP5LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP5LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP6TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP6TARGET).dpzz : $(ZIP6TARGETN)

.IF "$(common_build_zip)"!=""
ZIP6HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP6HELPVAR=$(BIN)$/$(ZIP6TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP6TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP6TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP6LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP6LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP6LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP6LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP7TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP7TARGET).dpzz : $(ZIP7TARGETN)

.IF "$(common_build_zip)"!=""
ZIP7HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP7HELPVAR=$(BIN)$/$(ZIP7TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP7TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP7TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP7LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP7LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP7LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP7LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP8TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP8TARGET).dpzz : $(ZIP8TARGETN)

.IF "$(common_build_zip)"!=""
ZIP8HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP8HELPVAR=$(BIN)$/$(ZIP8TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP8TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP8TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP8LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP8LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP8LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP8LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP9TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP9TARGET).dpzz : $(ZIP9TARGETN)

.IF "$(common_build_zip)"!=""
ZIP9HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP9HELPVAR=$(BIN)$/$(ZIP9TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP9TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP9TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP9LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP9LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP9LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP9LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP10TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP10TARGET).dpzz : $(ZIP10TARGETN)

.IF "$(common_build_zip)"!=""
ZIP10HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP10TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP10HELPVAR=$(BIN)$/$(ZIP10TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP10TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP10TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP10LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP10LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP10LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP10LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
