# unroll begin

.IF "$(APP1LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP1LINKTYPE))
.ENDIF

.IF "$(APP1STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP1STACKN=$(LINKFLAGSTACK)$(APP1STACK)
.ENDIF
.ELSE
APP1STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP1NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP1OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP1DEF=$(APP1DEF)
.ELSE
USE_APP1DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP1DEPN=
USE_APP1DEF=
.ENDIF

.IF "$(APP1TARGETN)"!=""
$(APP1TARGETN): $(APP1OBJS) $(APP1LIBS) \
    $(APP1RES) \
    $(APP1ICON) $(APP1DEPN) $(USE_APP1DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP1OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP1LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP1STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP1OBJS) `cat /dev/null $(APP1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP1STDLIBS) $(APP1ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP1LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP1LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP1ICON)" != ""
    @-+echo 1 ICON $(APP1ICON) >> $(MISC)$/$(APP1LINKRES:b).rc
.ENDIF
.IF "$(APP1VERINFO)" != ""
     @-+echo #include  "$(APP1VERINFO)" >> $(MISC)$/$(APP1LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP1LINKRES:b).rc
.ENDIF			# "$(APP1LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP1BASEX) \
        $(APP1STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP1LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP1RES)) \
        $(APP1OBJS) \
        $(APP1LIBS) \
        $(APP1STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP1TARGET).lnk
        +-$(RM) $(MISC)\$(APP1TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP1TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP1BASEX) \
        $(APP1STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP1LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP1RES)) \
        $(APP1OBJS) \
        $(APP1LIBS) \
        $(APP1STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP1TARGET).lst
        +if exist $(MISC)\$(APP1TARGET).lst type $(MISC)\$(APP1TARGET).lst  >> $(MISC)\$(APP1TARGET).lnk
        $(LINK) @$(MISC)\$(APP1TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP1TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP1STACKN) $(STDOBJ) $(APP1OBJS), $@, $(MISC)\$(APP1TARGET).map, $(APP1LIBS) $(APP1STDLIBS) $(STDLIB), $(APP1DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP1RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP1RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP1TARGET).map
    @copy $(APP1TARGET).sym $(BIN)\$(APP1TARGET).sym
    @del $(APP1TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP1TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP2LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP2LINKTYPE))
.ENDIF

.IF "$(APP2STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP2STACKN=$(LINKFLAGSTACK)$(APP2STACK)
.ENDIF
.ELSE
APP2STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP2NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP2OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP2DEF=$(APP2DEF)
.ELSE
USE_APP2DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP2DEPN=
USE_APP2DEF=
.ENDIF

.IF "$(APP2TARGETN)"!=""
$(APP2TARGETN): $(APP2OBJS) $(APP2LIBS) \
    $(APP2RES) \
    $(APP2ICON) $(APP2DEPN) $(USE_APP2DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP2OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP2LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP2STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP2OBJS) `cat /dev/null $(APP2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP2STDLIBS) $(APP2ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP2LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP2LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP2ICON)" != ""
    @-+echo 1 ICON $(APP2ICON) >> $(MISC)$/$(APP2LINKRES:b).rc
.ENDIF
.IF "$(APP2VERINFO)" != ""
     @-+echo #include  "$(APP2VERINFO)" >> $(MISC)$/$(APP2LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP2LINKRES:b).rc
.ENDIF			# "$(APP2LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP2BASEX) \
        $(APP2STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP2LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP2RES)) \
        $(APP2OBJS) \
        $(APP2LIBS) \
        $(APP2STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP2TARGET).lnk
        +-$(RM) $(MISC)\$(APP2TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP2TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP2BASEX) \
        $(APP2STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP2LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP2RES)) \
        $(APP2OBJS) \
        $(APP2LIBS) \
        $(APP2STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP2TARGET).lst
        +if exist $(MISC)\$(APP2TARGET).lst type $(MISC)\$(APP2TARGET).lst  >> $(MISC)\$(APP2TARGET).lnk
        $(LINK) @$(MISC)\$(APP2TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP2TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP2STACKN) $(STDOBJ) $(APP2OBJS), $@, $(MISC)\$(APP2TARGET).map, $(APP2LIBS) $(APP2STDLIBS) $(STDLIB), $(APP2DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP2RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP2RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP2TARGET).map
    @copy $(APP2TARGET).sym $(BIN)\$(APP2TARGET).sym
    @del $(APP2TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP2TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP3LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP3LINKTYPE))
.ENDIF

.IF "$(APP3STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP3STACKN=$(LINKFLAGSTACK)$(APP3STACK)
.ENDIF
.ELSE
APP3STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP3NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP3OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP3DEF=$(APP3DEF)
.ELSE
USE_APP3DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP3DEPN=
USE_APP3DEF=
.ENDIF

.IF "$(APP3TARGETN)"!=""
$(APP3TARGETN): $(APP3OBJS) $(APP3LIBS) \
    $(APP3RES) \
    $(APP3ICON) $(APP3DEPN) $(USE_APP3DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP3OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP3LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP3STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP3OBJS) `cat /dev/null $(APP3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP3STDLIBS) $(APP3ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP3LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP3LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP3ICON)" != ""
    @-+echo 1 ICON $(APP3ICON) >> $(MISC)$/$(APP3LINKRES:b).rc
.ENDIF
.IF "$(APP3VERINFO)" != ""
     @-+echo #include  "$(APP3VERINFO)" >> $(MISC)$/$(APP3LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP3LINKRES:b).rc
.ENDIF			# "$(APP3LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP3BASEX) \
        $(APP3STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP3LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP3RES)) \
        $(APP3OBJS) \
        $(APP3LIBS) \
        $(APP3STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP3TARGET).lnk
        +-$(RM) $(MISC)\$(APP3TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP3TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP3BASEX) \
        $(APP3STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP3LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP3RES)) \
        $(APP3OBJS) \
        $(APP3LIBS) \
        $(APP3STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP3TARGET).lst
        +if exist $(MISC)\$(APP3TARGET).lst type $(MISC)\$(APP3TARGET).lst  >> $(MISC)\$(APP3TARGET).lnk
        $(LINK) @$(MISC)\$(APP3TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP3TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP3STACKN) $(STDOBJ) $(APP3OBJS), $@, $(MISC)\$(APP3TARGET).map, $(APP3LIBS) $(APP3STDLIBS) $(STDLIB), $(APP3DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP3RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP3RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP3TARGET).map
    @copy $(APP3TARGET).sym $(BIN)\$(APP3TARGET).sym
    @del $(APP3TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP3TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP4LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP4LINKTYPE))
.ENDIF

.IF "$(APP4STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP4STACKN=$(LINKFLAGSTACK)$(APP4STACK)
.ENDIF
.ELSE
APP4STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP4NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP4OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP4DEF=$(APP4DEF)
.ELSE
USE_APP4DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP4DEPN=
USE_APP4DEF=
.ENDIF

.IF "$(APP4TARGETN)"!=""
$(APP4TARGETN): $(APP4OBJS) $(APP4LIBS) \
    $(APP4RES) \
    $(APP4ICON) $(APP4DEPN) $(USE_APP4DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP4OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP4LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP4STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP4OBJS) `cat /dev/null $(APP4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP4STDLIBS) $(APP4ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP4LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP4LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP4ICON)" != ""
    @-+echo 1 ICON $(APP4ICON) >> $(MISC)$/$(APP4LINKRES:b).rc
.ENDIF
.IF "$(APP4VERINFO)" != ""
     @-+echo #include  "$(APP4VERINFO)" >> $(MISC)$/$(APP4LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP4LINKRES:b).rc
.ENDIF			# "$(APP4LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP4BASEX) \
        $(APP4STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP4LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP4RES)) \
        $(APP4OBJS) \
        $(APP4LIBS) \
        $(APP4STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP4TARGET).lnk
        +-$(RM) $(MISC)\$(APP4TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP4TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP4BASEX) \
        $(APP4STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP4LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP4RES)) \
        $(APP4OBJS) \
        $(APP4LIBS) \
        $(APP4STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP4TARGET).lst
        +if exist $(MISC)\$(APP4TARGET).lst type $(MISC)\$(APP4TARGET).lst  >> $(MISC)\$(APP4TARGET).lnk
        $(LINK) @$(MISC)\$(APP4TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP4TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP4STACKN) $(STDOBJ) $(APP4OBJS), $@, $(MISC)\$(APP4TARGET).map, $(APP4LIBS) $(APP4STDLIBS) $(STDLIB), $(APP4DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP4RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP4RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP4TARGET).map
    @copy $(APP4TARGET).sym $(BIN)\$(APP4TARGET).sym
    @del $(APP4TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP4TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP5LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP5LINKTYPE))
.ENDIF

.IF "$(APP5STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP5STACKN=$(LINKFLAGSTACK)$(APP5STACK)
.ENDIF
.ELSE
APP5STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP5NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP5OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP5DEF=$(APP5DEF)
.ELSE
USE_APP5DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP5DEPN=
USE_APP5DEF=
.ENDIF

.IF "$(APP5TARGETN)"!=""
$(APP5TARGETN): $(APP5OBJS) $(APP5LIBS) \
    $(APP5RES) \
    $(APP5ICON) $(APP5DEPN) $(USE_APP5DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP5OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP5LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP5STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP5OBJS) `cat /dev/null $(APP5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP5STDLIBS) $(APP5ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP5LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP5LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP5ICON)" != ""
    @-+echo 1 ICON $(APP5ICON) >> $(MISC)$/$(APP5LINKRES:b).rc
.ENDIF
.IF "$(APP5VERINFO)" != ""
     @-+echo #include  "$(APP5VERINFO)" >> $(MISC)$/$(APP5LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP5LINKRES:b).rc
.ENDIF			# "$(APP5LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP5BASEX) \
        $(APP5STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP5LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP5RES)) \
        $(APP5OBJS) \
        $(APP5LIBS) \
        $(APP5STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP5TARGET).lnk
        +-$(RM) $(MISC)\$(APP5TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP5TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP5BASEX) \
        $(APP5STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP5LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP5RES)) \
        $(APP5OBJS) \
        $(APP5LIBS) \
        $(APP5STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP5TARGET).lst
        +if exist $(MISC)\$(APP5TARGET).lst type $(MISC)\$(APP5TARGET).lst  >> $(MISC)\$(APP5TARGET).lnk
        $(LINK) @$(MISC)\$(APP5TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP5TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP5STACKN) $(STDOBJ) $(APP5OBJS), $@, $(MISC)\$(APP5TARGET).map, $(APP5LIBS) $(APP5STDLIBS) $(STDLIB), $(APP5DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP5RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP5RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP5TARGET).map
    @copy $(APP5TARGET).sym $(BIN)\$(APP5TARGET).sym
    @del $(APP5TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP5TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP6LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP6LINKTYPE))
.ENDIF

.IF "$(APP6STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP6STACKN=$(LINKFLAGSTACK)$(APP6STACK)
.ENDIF
.ELSE
APP6STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP6NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP6OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP6DEF=$(APP6DEF)
.ELSE
USE_APP6DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP6DEPN=
USE_APP6DEF=
.ENDIF

.IF "$(APP6TARGETN)"!=""
$(APP6TARGETN): $(APP6OBJS) $(APP6LIBS) \
    $(APP6RES) \
    $(APP6ICON) $(APP6DEPN) $(USE_APP6DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP6OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP6LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP6STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP6OBJS) `cat /dev/null $(APP6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP6STDLIBS) $(APP6ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP6LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP6LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP6ICON)" != ""
    @-+echo 1 ICON $(APP6ICON) >> $(MISC)$/$(APP6LINKRES:b).rc
.ENDIF
.IF "$(APP6VERINFO)" != ""
     @-+echo #include  "$(APP6VERINFO)" >> $(MISC)$/$(APP6LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP6LINKRES:b).rc
.ENDIF			# "$(APP6LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP6BASEX) \
        $(APP6STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP6LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP6RES)) \
        $(APP6OBJS) \
        $(APP6LIBS) \
        $(APP6STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP6TARGET).lnk
        +-$(RM) $(MISC)\$(APP6TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP6TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP6BASEX) \
        $(APP6STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP6LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP6RES)) \
        $(APP6OBJS) \
        $(APP6LIBS) \
        $(APP6STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP6TARGET).lst
        +if exist $(MISC)\$(APP6TARGET).lst type $(MISC)\$(APP6TARGET).lst  >> $(MISC)\$(APP6TARGET).lnk
        $(LINK) @$(MISC)\$(APP6TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP6TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP6STACKN) $(STDOBJ) $(APP6OBJS), $@, $(MISC)\$(APP6TARGET).map, $(APP6LIBS) $(APP6STDLIBS) $(STDLIB), $(APP6DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP6RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP6RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP6TARGET).map
    @copy $(APP6TARGET).sym $(BIN)\$(APP6TARGET).sym
    @del $(APP6TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP6TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP7LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP7LINKTYPE))
.ENDIF

.IF "$(APP7STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP7STACKN=$(LINKFLAGSTACK)$(APP7STACK)
.ENDIF
.ELSE
APP7STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP7NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP7OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP7DEF=$(APP7DEF)
.ELSE
USE_APP7DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP7DEPN=
USE_APP7DEF=
.ENDIF

.IF "$(APP7TARGETN)"!=""
$(APP7TARGETN): $(APP7OBJS) $(APP7LIBS) \
    $(APP7RES) \
    $(APP7ICON) $(APP7DEPN) $(USE_APP7DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP7OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP7LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP7STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP7OBJS) `cat /dev/null $(APP7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP7STDLIBS) $(APP7ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP7LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP7LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP7ICON)" != ""
    @-+echo 1 ICON $(APP7ICON) >> $(MISC)$/$(APP7LINKRES:b).rc
.ENDIF
.IF "$(APP7VERINFO)" != ""
     @-+echo #include  "$(APP7VERINFO)" >> $(MISC)$/$(APP7LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP7LINKRES:b).rc
.ENDIF			# "$(APP7LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP7BASEX) \
        $(APP7STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP7LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP7RES)) \
        $(APP7OBJS) \
        $(APP7LIBS) \
        $(APP7STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP7TARGET).lnk
        +-$(RM) $(MISC)\$(APP7TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP7TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP7BASEX) \
        $(APP7STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP7LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP7RES)) \
        $(APP7OBJS) \
        $(APP7LIBS) \
        $(APP7STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP7TARGET).lst
        +if exist $(MISC)\$(APP7TARGET).lst type $(MISC)\$(APP7TARGET).lst  >> $(MISC)\$(APP7TARGET).lnk
        $(LINK) @$(MISC)\$(APP7TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP7TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP7STACKN) $(STDOBJ) $(APP7OBJS), $@, $(MISC)\$(APP7TARGET).map, $(APP7LIBS) $(APP7STDLIBS) $(STDLIB), $(APP7DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP7RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP7RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP7TARGET).map
    @copy $(APP7TARGET).sym $(BIN)\$(APP7TARGET).sym
    @del $(APP7TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP7TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP8LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP8LINKTYPE))
.ENDIF

.IF "$(APP8STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP8STACKN=$(LINKFLAGSTACK)$(APP8STACK)
.ENDIF
.ELSE
APP8STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP8NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP8OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP8DEF=$(APP8DEF)
.ELSE
USE_APP8DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP8DEPN=
USE_APP8DEF=
.ENDIF

.IF "$(APP8TARGETN)"!=""
$(APP8TARGETN): $(APP8OBJS) $(APP8LIBS) \
    $(APP8RES) \
    $(APP8ICON) $(APP8DEPN) $(USE_APP8DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP8OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP8LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP8STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP8OBJS) `cat /dev/null $(APP8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP8STDLIBS) $(APP8ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP8LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP8LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP8ICON)" != ""
    @-+echo 1 ICON $(APP8ICON) >> $(MISC)$/$(APP8LINKRES:b).rc
.ENDIF
.IF "$(APP8VERINFO)" != ""
     @-+echo #include  "$(APP8VERINFO)" >> $(MISC)$/$(APP8LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP8LINKRES:b).rc
.ENDIF			# "$(APP8LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP8BASEX) \
        $(APP8STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP8LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP8RES)) \
        $(APP8OBJS) \
        $(APP8LIBS) \
        $(APP8STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP8TARGET).lnk
        +-$(RM) $(MISC)\$(APP8TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP8TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP8BASEX) \
        $(APP8STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP8LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP8RES)) \
        $(APP8OBJS) \
        $(APP8LIBS) \
        $(APP8STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP8TARGET).lst
        +if exist $(MISC)\$(APP8TARGET).lst type $(MISC)\$(APP8TARGET).lst  >> $(MISC)\$(APP8TARGET).lnk
        $(LINK) @$(MISC)\$(APP8TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP8TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP8STACKN) $(STDOBJ) $(APP8OBJS), $@, $(MISC)\$(APP8TARGET).map, $(APP8LIBS) $(APP8STDLIBS) $(STDLIB), $(APP8DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP8RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP8RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP8TARGET).map
    @copy $(APP8TARGET).sym $(BIN)\$(APP8TARGET).sym
    @del $(APP8TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP8TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP9LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP9LINKTYPE))
.ENDIF

.IF "$(APP9STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP9STACKN=$(LINKFLAGSTACK)$(APP9STACK)
.ENDIF
.ELSE
APP9STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP9NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP9OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP9DEF=$(APP9DEF)
.ELSE
USE_APP9DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP9DEPN=
USE_APP9DEF=
.ENDIF

.IF "$(APP9TARGETN)"!=""
$(APP9TARGETN): $(APP9OBJS) $(APP9LIBS) \
    $(APP9RES) \
    $(APP9ICON) $(APP9DEPN) $(USE_APP9DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP9OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP9LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP9STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP9OBJS) `cat /dev/null $(APP9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP9STDLIBS) $(APP9ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP9LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP9LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP9ICON)" != ""
    @-+echo 1 ICON $(APP9ICON) >> $(MISC)$/$(APP9LINKRES:b).rc
.ENDIF
.IF "$(APP9VERINFO)" != ""
     @-+echo #include  "$(APP9VERINFO)" >> $(MISC)$/$(APP9LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP9LINKRES:b).rc
.ENDIF			# "$(APP9LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP9BASEX) \
        $(APP9STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP9LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP9RES)) \
        $(APP9OBJS) \
        $(APP9LIBS) \
        $(APP9STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP9TARGET).lnk
        +-$(RM) $(MISC)\$(APP9TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP9TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP9BASEX) \
        $(APP9STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP9LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP9RES)) \
        $(APP9OBJS) \
        $(APP9LIBS) \
        $(APP9STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP9TARGET).lst
        +if exist $(MISC)\$(APP9TARGET).lst type $(MISC)\$(APP9TARGET).lst  >> $(MISC)\$(APP9TARGET).lnk
        $(LINK) @$(MISC)\$(APP9TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP9TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP9STACKN) $(STDOBJ) $(APP9OBJS), $@, $(MISC)\$(APP9TARGET).map, $(APP9LIBS) $(APP9STDLIBS) $(STDLIB), $(APP9DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP9RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP9RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP9TARGET).map
    @copy $(APP9TARGET).sym $(BIN)\$(APP9TARGET).sym
    @del $(APP9TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP9TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(APP10LINKTYPE)" != ""
#darf nur STATIC oder SHARED sein
APP_LINKTYPE=$(APPLINK$(APP10LINKTYPE))
.ENDIF

.IF "$(APP10STACK)" != ""
.IF "$(LINKFLAGSTACK)" != ""
APP10STACKN=$(LINKFLAGSTACK)$(APP10STACK)
.ENDIF
.ELSE
APP10STACKN=
.ENDIF
.IF "$(VCL)" != ""
.IF "$(APP10NOSAL)"==""
.IF "$(TARGETTYPE)" == "GUI"
APP10OBJS+= $(STDOBJVCL)
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)" != "WNT" || "$(GUI)$(COM)"=="WNTBLC"
USE_APP10DEF=$(APP10DEF)
.ELSE
USE_APP10DEF=
.ENDIF

.IF "$(GUI)" == "UNX"
APP10DEPN=
USE_APP10DEF=
.ENDIF

.IF "$(APP10TARGETN)"!=""
$(APP10TARGETN): $(APP10OBJS) $(APP10LIBS) \
    $(APP10RES) \
    $(APP10ICON) $(APP10DEPN) $(USE_APP10DEF)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo unx
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) -L$(PRJ)$/$(INPATH)$/lib $(SOLARLIB) $(STDSLO) \
    -o $@ $(APP10OBJS:s/.obj/.o/)\
    `cat /dev/null $(APP10LIBS) | sed s\#$(ROUT)\#$(OUT)\#g` \
    $(APP_LINKTYPE) $(APP10STDLIBS) $(STDLIB) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @source $(MISC)$/$(@:b).cmd
    @ls -l $@
.IF "$(OS)"=="MACOSX"
.IF "$(TARGETTYPE)"=="GUI"
    @+-$(RM) -R $@.app
    @echo "Making: $@.app"
    @$(MKDIRHIER) $@.app$/Contents$/MacOS
    @$(COPY) $@ $@.app$/Contents$/MacOS$/$(@:f)
    @$(RM) $@
    @echo '#\!/bin/sh' >> $@
    @echo 'exec `dirname $$0`$/$(@:f).app$/Contents$/MacOS$/$(@:f)' >> $@
    @chmod a+x $@
.ENDIF		# "$(TARGETTYPE)"=="GUI"
.ENDIF		# "$(OS)"=="MACOSX"
.ENDIF
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM $@.idb
    $(LINK) $(LINKFLAGS) $(LINKFLAGSAPP) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(APP10OBJS) `cat /dev/null $(APP10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(VERSIONOBJ)) $(APP10STDLIBS) $(APP10ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF                  # "$(GUI)"=="MAC"

.IF "$(GUI)" == "WNT"
    @+-$(MKDIR) $(@:d:d) >& $(NULLDEV)
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(APP10LINKRES)" != ""
    @+-$(RM) $(MISC)$/$(APP10LINKRES:b).rc >& $(NULLDEV)
.IF "$(APP10ICON)" != ""
    @-+echo 1 ICON $(APP10ICON) >> $(MISC)$/$(APP10LINKRES:b).rc
.ENDIF
.IF "$(APP10VERINFO)" != ""
     @-+echo #include  "$(APP10VERINFO)" >> $(MISC)$/$(APP10LINKRES:b).rc
.ENDIF
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(APP10LINKRES:b).rc
.ENDIF			# "$(APP10LINKRES)" != ""
.IF "$(linkinc)" == ""
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP10BASEX) \
        $(APP10STACKN) \
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        $(STDOBJ) \
        $(APP10LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP10RES)) \
        $(APP10OBJS) \
        $(APP10LIBS) \
        $(APP10STDLIBS) \
        $(STDLIB) \
        )
.ELSE
        +-$(RM) $(MISC)\$(APP10TARGET).lnk
        +-$(RM) $(MISC)\$(APP10TARGET).lst
        +-$(RM) $(MISC)\linkobj.lst
        +for %_i in ($(MISC)\*.obj) do type %_i >> $(MISC)\linkobj.lst
    type $(mktmp,$(MISC)\$(APP10TARGET).lst
        $(LINKFLAGS) \
        $(LINKFLAGSAPP) $(APP10BASEX) \
        $(APP10STACKN) \
        -out:$@ \
        $(STDOBJ) \
        $(APP10LINKRES) \
        $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP10RES)) \
        $(APP10OBJS) \
        $(APP10LIBS) \
        $(APP10STDLIBS) \
        $(STDLIB))
        sed -e 's/\(\.\.\\\)\{2,4\}/..\\/g' $(MISC)\linkobj.lst >> $(MISC)\$(APP10TARGET).lst
        +if exist $(MISC)\$(APP10TARGET).lst type $(MISC)\$(APP10TARGET).lst  >> $(MISC)\$(APP10TARGET).lnk
        $(LINK) @$(MISC)\$(APP10TARGET).lnk
.ENDIF		# "$(linkinc)" == ""

.IF "$(APP10TARGET)" == "loader"
    +perl5 loader.pl $@
    +$(COPY) /b $(@)+$(@:d)unloader.exe $(@:d)_new.exe
    +$(RM) $@
    +$(RENAME) $(@:d)_new.exe $(@:d)loader.exe
.ENDIF			# "$(TARGET)" == "setup"

.ENDIF			# "$(GUI)" == "WNT"

.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSAPP) $(APP10STACKN) $(STDOBJ) $(APP10OBJS), $@, $(MISC)\$(APP10TARGET).map, $(APP10LIBS) $(APP10STDLIBS) $(STDLIB), $(APP10DEF)) >&  $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ELSE
    @+echo ------------------------
    @+echo No valid Environment!!!
    @+echo ------------------------
    force_dmake_to_error
.ENDIF			# "$(COM)"=="BLC"
.IF "$(TARGETTYPE)" == "GUI"
.IF "$(APP10RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(subst,$/res$/,$/res{$(subst,$(BIN), $(@:d))} $(APP10RES)) $@
.ELSE
    $(RCSETVERSION)
.ENDIF
.IF "$(MAPSYM)" != ""
    mapfix $(MISC)\$(@B).map
    $(MAPSYM) $(MAPSYMFLAGS) $(MISC)\$(APP10TARGET).map
    @copy $(APP10TARGET).sym $(BIN)\$(APP10TARGET).sym
    @del $(APP10TARGET).sym
.ENDIF			# "$(MAPSYM)" != ""
.ENDIF			# "$(TARGETTYPE)" == "GUI"
.ENDIF			# "$(GUI)" == "WIN"

.ENDIF			# "$(APP10TARGETN)"!=""


# Anweisungen fuer das Linken
