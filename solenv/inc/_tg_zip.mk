# unroll begin

.IF "$(ZIP1TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP1TARGET).dpzz : $(ZIP1TARGETN)

ZIP1HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP1HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP1LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP1TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP2TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP2TARGET).dpzz : $(ZIP2TARGETN)

ZIP2HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP2HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP2LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP2TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP3TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP3TARGET).dpzz : $(ZIP3TARGETN)

ZIP3HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP3HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP3LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP3TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP4TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP4TARGET).dpzz : $(ZIP4TARGETN)

ZIP4HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP4HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP4LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP4TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP5TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP5TARGET).dpzz : $(ZIP5TARGETN)

ZIP5HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP5HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP5LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP5TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP6TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP6TARGET).dpzz : $(ZIP6TARGETN)

ZIP6HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP6HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP6LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP6TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP7TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP7TARGET).dpzz : $(ZIP7TARGETN)

ZIP7HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP7HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP7LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP7TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP8TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP8TARGET).dpzz : $(ZIP8TARGETN)

ZIP8HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP8HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP8LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP8TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP9TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP9TARGET).dpzz : $(ZIP9TARGETN)

ZIP9HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP9HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP9LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP9TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(ZIP10TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP10TARGET).dpzz : $(ZIP10TARGETN)

ZIP10HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP10TARGET)}
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
    @+-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )) -x delzip
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
    @+-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )) -x delzip
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP10HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(common_build_zip)"!=""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP10LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP10TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
#next Target
