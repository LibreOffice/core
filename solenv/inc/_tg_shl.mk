# unroll begin

.IF "$(OS)"=="AIX"
SHL1STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL1STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL1STDLIBS=
STDSHL=
.ELSE
SHL1ARCHIVES=
.ENDIF

SHL1DEF*=$(MISC)$/$(SHL1TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL1VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
SHL1VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL1VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL1IMPLIB)" != ""
USE_1IMPLIB=-implib:$(LB)$/$(SHL1IMPLIB).lib
.ELSE			# "$(SHL1IMPLIB)" != ""
USE_1IMPLIB=-implib:$(LB)$/i$(TARGET)1.lib
.ENDIF			# "$(SHL1IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_1IMPLIB_DEPS=$(LB)$/$(SHL1IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL1DEF=$(SHL1DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL1DEF=
SHL1DEPN+:=$(SHL1DEPNU)

# to activate vmaps remove "#"
#USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap

.IF "$(USE_SHL1VERSIONMAP)"!=""

.IF "$(DEF1EXPORTFILE)"!=""
.IF "$(SHL1VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF

$(USE_SHL1VERSIONMAP): \
                    $(SHL1OBJS)\
                    $(SHL1LIBS)\
                    $(DEF1EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL1OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL1OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL1OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL1OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL1LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF1EXPORTFILE) -o $@

.ELSE			# "$(DEF1EXPORTFILE)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF1EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF1EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL1VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL1VERSIONMAP)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF

$(USE_SHL1VERSIONMAP): $(SHL1VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL1VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL1VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL1VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL1RES)"!=""
SHL1RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL1RES))
SHL1ALLRES+=$(SHL1RES)
SHL1LINKRES*=$(MISC)$/$(SHL1TARGET).res
.ENDIF			# "$(SHL1RES)"!=""
.ENDIF

.IF "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""
SHL1DEFAULTRES*=$(MISC)$/$(SHL1TARGET)_def.res
SHL1ALLRES+=$(SHL1DEFAULTRES)
SHL1LINKRES*=$(MISC)$/$(SHL1TARGET).res
.ENDIF			# "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL1DESCRIPTION)"==""
#SHL1DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL1DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_description.obj
.ENDIF			# "$(NO_SHL1DESCRIPTION)"==""

.IF "$(SHL1TARGETN)"!=""
$(SHL1TARGETN) : \
                    $(SHL1OBJS)\
                    $(SHL1DESCRIPTIONOBJ)\
                    $(SHL1LIBS)\
                    $(USE_1IMPLIB_DEPS)\
                    $(USE_SHL1DEF)\
                    $(USE_SHL1VERSIONMAP)\
                    $(SHL1RES)\
                    $(SHL1VERSIONH)\
                    $(SHL1DEPN)
    @echo ------------------------------
    @echo Making: $(SHL1TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL1VERSIONOBJ:+"+\n") $(SHL1OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL1LIBS:+"+\n") $(SHL1STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL1DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL1LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL1STDLIBS:^"-l") $(SHL1OBJS) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) $(SHL1DEF)
.ENDIF
.IF "$(SHL1RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL1RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL1OBJS), $@, $(MISC)\$(@:b).map, $(SHL1LIBS) $(SHL1STDLIBS) $(STDSHL), $(SHL1DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL1DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL1DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL1ICON)" != ""
    @-+echo 1 ICON $(SHL1ICON) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL1TARGET)$(DLLPOST) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL1TARGET:b) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(SHL1DEFAULTRES)"!=""
.IF "$(SHL1ALLRES)"!=""
    +$(COPY) /b $(SHL1ALLRES:s/res /res+/) $(SHL1LINKRES)
.ENDIF			# "$(SHL1ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL1STACK) $(SHL1BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL1TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) $(SHL1OBJS) \
        $(SHL1LIBS) \
        $(SHL1STDLIBS) \
        $(STDSHL) \
        $(SHL1LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL1STDLIBS) $(STDSHL) $(SHL1RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL1STACK) $(SHL1BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) $(SHL1OBJS) \
        $(SHL1LIBS) \
        $(SHL1STDLIBS) \
        $(STDSHL) \
        $(SHL1LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        $(SHL1STACK) -out:$(SHL1TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL1IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ)   \
        $(SHL1LIBS)                         \
        $(SHL1STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL1LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL1STACK) -out:$(BIN)$/_$(SHL1TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL1IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ)    \
        $(SHL1LIBS)                         \
        $(SHL1STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL1LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL1TARGET).lnk del $(MISC)$/$(SHL1TARGET).lnk
        +if exist $(MISC)$/$(SHL1TARGET).lst del $(MISC)$/$(SHL1TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL1BASEX) \
        $(SHL1STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL1IMPLIB).exp \
        $(STDOBJ) \
        $(SHL1OBJS) \
        $(SHL1STDLIBS) \
        $(STDSHL) \
        $(SHL1LINKRES) \
        ) >> $(MISC)$/$(SHL1TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL1TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL1TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL1VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL1LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL1VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL1VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL1TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL1TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL1OBJS) `cat /dev/null $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ)) $(SHL1STDLIBS) $(SHL1ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL1TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL2STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL2STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL2STDLIBS=
STDSHL=
.ELSE
SHL2ARCHIVES=
.ENDIF

SHL2DEF*=$(MISC)$/$(SHL2TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL2VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
SHL2VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL2VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL2IMPLIB)" != ""
USE_2IMPLIB=-implib:$(LB)$/$(SHL2IMPLIB).lib
.ELSE			# "$(SHL2IMPLIB)" != ""
USE_2IMPLIB=-implib:$(LB)$/i$(TARGET)2.lib
.ENDIF			# "$(SHL2IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_2IMPLIB_DEPS=$(LB)$/$(SHL2IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL2DEF=$(SHL2DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL2DEF=
SHL2DEPN+:=$(SHL2DEPNU)

# to activate vmaps remove "#"
#USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap

.IF "$(USE_SHL2VERSIONMAP)"!=""

.IF "$(DEF2EXPORTFILE)"!=""
.IF "$(SHL2VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF

$(USE_SHL2VERSIONMAP): \
                    $(SHL2OBJS)\
                    $(SHL2LIBS)\
                    $(DEF2EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL2OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL2OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL2OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL2OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL2LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF2EXPORTFILE) -o $@

.ELSE			# "$(DEF2EXPORTFILE)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF2EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF2EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL2VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL2VERSIONMAP)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF

$(USE_SHL2VERSIONMAP): $(SHL2VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL2VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL2VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL2VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL2RES)"!=""
SHL2RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL2RES))
SHL2ALLRES+=$(SHL2RES)
SHL2LINKRES*=$(MISC)$/$(SHL2TARGET).res
.ENDIF			# "$(SHL2RES)"!=""
.ENDIF

.IF "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""
SHL2DEFAULTRES*=$(MISC)$/$(SHL2TARGET)_def.res
SHL2ALLRES+=$(SHL2DEFAULTRES)
SHL2LINKRES*=$(MISC)$/$(SHL2TARGET).res
.ENDIF			# "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL2DESCRIPTION)"==""
#SHL2DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL2DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_description.obj
.ENDIF			# "$(NO_SHL2DESCRIPTION)"==""

.IF "$(SHL2TARGETN)"!=""
$(SHL2TARGETN) : \
                    $(SHL2OBJS)\
                    $(SHL2DESCRIPTIONOBJ)\
                    $(SHL2LIBS)\
                    $(USE_2IMPLIB_DEPS)\
                    $(USE_SHL2DEF)\
                    $(USE_SHL2VERSIONMAP)\
                    $(SHL2RES)\
                    $(SHL2VERSIONH)\
                    $(SHL2DEPN)
    @echo ------------------------------
    @echo Making: $(SHL2TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL2VERSIONOBJ:+"+\n") $(SHL2OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL2LIBS:+"+\n") $(SHL2STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL2DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL2LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL2STDLIBS:^"-l") $(SHL2OBJS) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) $(SHL2DEF)
.ENDIF
.IF "$(SHL2RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL2RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL2OBJS), $@, $(MISC)\$(@:b).map, $(SHL2LIBS) $(SHL2STDLIBS) $(STDSHL), $(SHL2DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL2DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL2DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL2ICON)" != ""
    @-+echo 1 ICON $(SHL2ICON) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL2TARGET)$(DLLPOST) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL2TARGET:b) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(SHL2DEFAULTRES)"!=""
.IF "$(SHL2ALLRES)"!=""
    +$(COPY) /b $(SHL2ALLRES:s/res /res+/) $(SHL2LINKRES)
.ENDIF			# "$(SHL2ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL2STACK) $(SHL2BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL2TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) $(SHL2OBJS) \
        $(SHL2LIBS) \
        $(SHL2STDLIBS) \
        $(STDSHL) \
        $(SHL2LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL2STDLIBS) $(STDSHL) $(SHL2RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL2STACK) $(SHL2BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) $(SHL2OBJS) \
        $(SHL2LIBS) \
        $(SHL2STDLIBS) \
        $(STDSHL) \
        $(SHL2LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        $(SHL2STACK) -out:$(SHL2TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL2IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ)   \
        $(SHL2LIBS)                         \
        $(SHL2STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL2LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL2STACK) -out:$(BIN)$/_$(SHL2TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL2IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ)    \
        $(SHL2LIBS)                         \
        $(SHL2STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL2LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL2TARGET).lnk del $(MISC)$/$(SHL2TARGET).lnk
        +if exist $(MISC)$/$(SHL2TARGET).lst del $(MISC)$/$(SHL2TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL2BASEX) \
        $(SHL2STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL2IMPLIB).exp \
        $(STDOBJ) \
        $(SHL2OBJS) \
        $(SHL2STDLIBS) \
        $(STDSHL) \
        $(SHL2LINKRES) \
        ) >> $(MISC)$/$(SHL2TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL2TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL2TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL2VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL2LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL2VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL2VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL2TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL2TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL2OBJS) `cat /dev/null $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ)) $(SHL2STDLIBS) $(SHL2ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL2TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL3STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL3STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL3STDLIBS=
STDSHL=
.ELSE
SHL3ARCHIVES=
.ENDIF

SHL3DEF*=$(MISC)$/$(SHL3TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL3VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
SHL3VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL3VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL3IMPLIB)" != ""
USE_3IMPLIB=-implib:$(LB)$/$(SHL3IMPLIB).lib
.ELSE			# "$(SHL3IMPLIB)" != ""
USE_3IMPLIB=-implib:$(LB)$/i$(TARGET)3.lib
.ENDIF			# "$(SHL3IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_3IMPLIB_DEPS=$(LB)$/$(SHL3IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL3DEF=$(SHL3DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL3DEF=
SHL3DEPN+:=$(SHL3DEPNU)

# to activate vmaps remove "#"
#USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap

.IF "$(USE_SHL3VERSIONMAP)"!=""

.IF "$(DEF3EXPORTFILE)"!=""
.IF "$(SHL3VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF

$(USE_SHL3VERSIONMAP): \
                    $(SHL3OBJS)\
                    $(SHL3LIBS)\
                    $(DEF3EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL3OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL3OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL3OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL3OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL3LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF3EXPORTFILE) -o $@

.ELSE			# "$(DEF3EXPORTFILE)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF3EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF3EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL3VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL3VERSIONMAP)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF

$(USE_SHL3VERSIONMAP): $(SHL3VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL3VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL3VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL3VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL3RES)"!=""
SHL3RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL3RES))
SHL3ALLRES+=$(SHL3RES)
SHL3LINKRES*=$(MISC)$/$(SHL3TARGET).res
.ENDIF			# "$(SHL3RES)"!=""
.ENDIF

.IF "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""
SHL3DEFAULTRES*=$(MISC)$/$(SHL3TARGET)_def.res
SHL3ALLRES+=$(SHL3DEFAULTRES)
SHL3LINKRES*=$(MISC)$/$(SHL3TARGET).res
.ENDIF			# "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL3DESCRIPTION)"==""
#SHL3DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL3DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_description.obj
.ENDIF			# "$(NO_SHL3DESCRIPTION)"==""

.IF "$(SHL3TARGETN)"!=""
$(SHL3TARGETN) : \
                    $(SHL3OBJS)\
                    $(SHL3DESCRIPTIONOBJ)\
                    $(SHL3LIBS)\
                    $(USE_3IMPLIB_DEPS)\
                    $(USE_SHL3DEF)\
                    $(USE_SHL3VERSIONMAP)\
                    $(SHL3RES)\
                    $(SHL3VERSIONH)\
                    $(SHL3DEPN)
    @echo ------------------------------
    @echo Making: $(SHL3TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL3VERSIONOBJ:+"+\n") $(SHL3OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL3LIBS:+"+\n") $(SHL3STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL3DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL3LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL3STDLIBS:^"-l") $(SHL3OBJS) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) $(SHL3DEF)
.ENDIF
.IF "$(SHL3RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL3RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL3OBJS), $@, $(MISC)\$(@:b).map, $(SHL3LIBS) $(SHL3STDLIBS) $(STDSHL), $(SHL3DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL3DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL3DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL3ICON)" != ""
    @-+echo 1 ICON $(SHL3ICON) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL3TARGET)$(DLLPOST) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL3TARGET:b) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(SHL3DEFAULTRES)"!=""
.IF "$(SHL3ALLRES)"!=""
    +$(COPY) /b $(SHL3ALLRES:s/res /res+/) $(SHL3LINKRES)
.ENDIF			# "$(SHL3ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL3STACK) $(SHL3BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL3TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) $(SHL3OBJS) \
        $(SHL3LIBS) \
        $(SHL3STDLIBS) \
        $(STDSHL) \
        $(SHL3LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL3STDLIBS) $(STDSHL) $(SHL3RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL3STACK) $(SHL3BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) $(SHL3OBJS) \
        $(SHL3LIBS) \
        $(SHL3STDLIBS) \
        $(STDSHL) \
        $(SHL3LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        $(SHL3STACK) -out:$(SHL3TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL3IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ)   \
        $(SHL3LIBS)                         \
        $(SHL3STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL3LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL3STACK) -out:$(BIN)$/_$(SHL3TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL3IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ)    \
        $(SHL3LIBS)                         \
        $(SHL3STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL3LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL3TARGET).lnk del $(MISC)$/$(SHL3TARGET).lnk
        +if exist $(MISC)$/$(SHL3TARGET).lst del $(MISC)$/$(SHL3TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL3BASEX) \
        $(SHL3STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL3IMPLIB).exp \
        $(STDOBJ) \
        $(SHL3OBJS) \
        $(SHL3STDLIBS) \
        $(STDSHL) \
        $(SHL3LINKRES) \
        ) >> $(MISC)$/$(SHL3TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL3TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL3TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL3VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL3LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL3VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL3VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL3TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL3TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL3OBJS) `cat /dev/null $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ)) $(SHL3STDLIBS) $(SHL3ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL3TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL4STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL4STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL4STDLIBS=
STDSHL=
.ELSE
SHL4ARCHIVES=
.ENDIF

SHL4DEF*=$(MISC)$/$(SHL4TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL4VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
SHL4VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL4VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL4IMPLIB)" != ""
USE_4IMPLIB=-implib:$(LB)$/$(SHL4IMPLIB).lib
.ELSE			# "$(SHL4IMPLIB)" != ""
USE_4IMPLIB=-implib:$(LB)$/i$(TARGET)4.lib
.ENDIF			# "$(SHL4IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_4IMPLIB_DEPS=$(LB)$/$(SHL4IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL4DEF=$(SHL4DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL4DEF=
SHL4DEPN+:=$(SHL4DEPNU)

# to activate vmaps remove "#"
#USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap

.IF "$(USE_SHL4VERSIONMAP)"!=""

.IF "$(DEF4EXPORTFILE)"!=""
.IF "$(SHL4VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF

$(USE_SHL4VERSIONMAP): \
                    $(SHL4OBJS)\
                    $(SHL4LIBS)\
                    $(DEF4EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL4OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL4OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL4OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL4OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL4LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF4EXPORTFILE) -o $@

.ELSE			# "$(DEF4EXPORTFILE)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF4EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF4EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL4VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL4VERSIONMAP)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF

$(USE_SHL4VERSIONMAP): $(SHL4VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL4VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL4VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL4VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL4RES)"!=""
SHL4RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL4RES))
SHL4ALLRES+=$(SHL4RES)
SHL4LINKRES*=$(MISC)$/$(SHL4TARGET).res
.ENDIF			# "$(SHL4RES)"!=""
.ENDIF

.IF "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""
SHL4DEFAULTRES*=$(MISC)$/$(SHL4TARGET)_def.res
SHL4ALLRES+=$(SHL4DEFAULTRES)
SHL4LINKRES*=$(MISC)$/$(SHL4TARGET).res
.ENDIF			# "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL4DESCRIPTION)"==""
#SHL4DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL4DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_description.obj
.ENDIF			# "$(NO_SHL4DESCRIPTION)"==""

.IF "$(SHL4TARGETN)"!=""
$(SHL4TARGETN) : \
                    $(SHL4OBJS)\
                    $(SHL4DESCRIPTIONOBJ)\
                    $(SHL4LIBS)\
                    $(USE_4IMPLIB_DEPS)\
                    $(USE_SHL4DEF)\
                    $(USE_SHL4VERSIONMAP)\
                    $(SHL4RES)\
                    $(SHL4VERSIONH)\
                    $(SHL4DEPN)
    @echo ------------------------------
    @echo Making: $(SHL4TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL4VERSIONOBJ:+"+\n") $(SHL4OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL4LIBS:+"+\n") $(SHL4STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL4DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL4LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL4STDLIBS:^"-l") $(SHL4OBJS) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) $(SHL4DEF)
.ENDIF
.IF "$(SHL4RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL4RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL4OBJS), $@, $(MISC)\$(@:b).map, $(SHL4LIBS) $(SHL4STDLIBS) $(STDSHL), $(SHL4DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL4DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL4DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL4ICON)" != ""
    @-+echo 1 ICON $(SHL4ICON) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL4TARGET)$(DLLPOST) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL4TARGET:b) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(SHL4DEFAULTRES)"!=""
.IF "$(SHL4ALLRES)"!=""
    +$(COPY) /b $(SHL4ALLRES:s/res /res+/) $(SHL4LINKRES)
.ENDIF			# "$(SHL4ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL4STACK) $(SHL4BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL4TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) $(SHL4OBJS) \
        $(SHL4LIBS) \
        $(SHL4STDLIBS) \
        $(STDSHL) \
        $(SHL4LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL4STDLIBS) $(STDSHL) $(SHL4RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL4STACK) $(SHL4BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) $(SHL4OBJS) \
        $(SHL4LIBS) \
        $(SHL4STDLIBS) \
        $(STDSHL) \
        $(SHL4LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        $(SHL4STACK) -out:$(SHL4TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL4IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ)   \
        $(SHL4LIBS)                         \
        $(SHL4STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL4LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL4STACK) -out:$(BIN)$/_$(SHL4TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL4IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ)    \
        $(SHL4LIBS)                         \
        $(SHL4STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL4LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL4TARGET).lnk del $(MISC)$/$(SHL4TARGET).lnk
        +if exist $(MISC)$/$(SHL4TARGET).lst del $(MISC)$/$(SHL4TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL4BASEX) \
        $(SHL4STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL4IMPLIB).exp \
        $(STDOBJ) \
        $(SHL4OBJS) \
        $(SHL4STDLIBS) \
        $(STDSHL) \
        $(SHL4LINKRES) \
        ) >> $(MISC)$/$(SHL4TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL4TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL4TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL4VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL4LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL4VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL4VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL4TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL4TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL4OBJS) `cat /dev/null $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ)) $(SHL4STDLIBS) $(SHL4ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL4TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL5STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL5STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL5STDLIBS=
STDSHL=
.ELSE
SHL5ARCHIVES=
.ENDIF

SHL5DEF*=$(MISC)$/$(SHL5TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL5VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
SHL5VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL5VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL5IMPLIB)" != ""
USE_5IMPLIB=-implib:$(LB)$/$(SHL5IMPLIB).lib
.ELSE			# "$(SHL5IMPLIB)" != ""
USE_5IMPLIB=-implib:$(LB)$/i$(TARGET)5.lib
.ENDIF			# "$(SHL5IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_5IMPLIB_DEPS=$(LB)$/$(SHL5IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL5DEF=$(SHL5DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL5DEF=
SHL5DEPN+:=$(SHL5DEPNU)

# to activate vmaps remove "#"
#USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap

.IF "$(USE_SHL5VERSIONMAP)"!=""

.IF "$(DEF5EXPORTFILE)"!=""
.IF "$(SHL5VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF

$(USE_SHL5VERSIONMAP): \
                    $(SHL5OBJS)\
                    $(SHL5LIBS)\
                    $(DEF5EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL5OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL5OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL5OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL5OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL5LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF5EXPORTFILE) -o $@

.ELSE			# "$(DEF5EXPORTFILE)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF5EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF5EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL5VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL5VERSIONMAP)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF

$(USE_SHL5VERSIONMAP): $(SHL5VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL5VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL5VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL5VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL5RES)"!=""
SHL5RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL5RES))
SHL5ALLRES+=$(SHL5RES)
SHL5LINKRES*=$(MISC)$/$(SHL5TARGET).res
.ENDIF			# "$(SHL5RES)"!=""
.ENDIF

.IF "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""
SHL5DEFAULTRES*=$(MISC)$/$(SHL5TARGET)_def.res
SHL5ALLRES+=$(SHL5DEFAULTRES)
SHL5LINKRES*=$(MISC)$/$(SHL5TARGET).res
.ENDIF			# "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL5DESCRIPTION)"==""
#SHL5DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL5DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_description.obj
.ENDIF			# "$(NO_SHL5DESCRIPTION)"==""

.IF "$(SHL5TARGETN)"!=""
$(SHL5TARGETN) : \
                    $(SHL5OBJS)\
                    $(SHL5DESCRIPTIONOBJ)\
                    $(SHL5LIBS)\
                    $(USE_5IMPLIB_DEPS)\
                    $(USE_SHL5DEF)\
                    $(USE_SHL5VERSIONMAP)\
                    $(SHL5RES)\
                    $(SHL5VERSIONH)\
                    $(SHL5DEPN)
    @echo ------------------------------
    @echo Making: $(SHL5TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL5VERSIONOBJ:+"+\n") $(SHL5OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL5LIBS:+"+\n") $(SHL5STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL5DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL5LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL5STDLIBS:^"-l") $(SHL5OBJS) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) $(SHL5DEF)
.ENDIF
.IF "$(SHL5RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL5RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL5OBJS), $@, $(MISC)\$(@:b).map, $(SHL5LIBS) $(SHL5STDLIBS) $(STDSHL), $(SHL5DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL5DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL5DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL5ICON)" != ""
    @-+echo 1 ICON $(SHL5ICON) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL5TARGET)$(DLLPOST) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL5TARGET:b) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(SHL5DEFAULTRES)"!=""
.IF "$(SHL5ALLRES)"!=""
    +$(COPY) /b $(SHL5ALLRES:s/res /res+/) $(SHL5LINKRES)
.ENDIF			# "$(SHL5ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL5STACK) $(SHL5BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL5TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) $(SHL5OBJS) \
        $(SHL5LIBS) \
        $(SHL5STDLIBS) \
        $(STDSHL) \
        $(SHL5LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL5STDLIBS) $(STDSHL) $(SHL5RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL5STACK) $(SHL5BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) $(SHL5OBJS) \
        $(SHL5LIBS) \
        $(SHL5STDLIBS) \
        $(STDSHL) \
        $(SHL5LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        $(SHL5STACK) -out:$(SHL5TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL5IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ)   \
        $(SHL5LIBS)                         \
        $(SHL5STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL5LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL5STACK) -out:$(BIN)$/_$(SHL5TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL5IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ)    \
        $(SHL5LIBS)                         \
        $(SHL5STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL5LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL5TARGET).lnk del $(MISC)$/$(SHL5TARGET).lnk
        +if exist $(MISC)$/$(SHL5TARGET).lst del $(MISC)$/$(SHL5TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL5BASEX) \
        $(SHL5STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL5IMPLIB).exp \
        $(STDOBJ) \
        $(SHL5OBJS) \
        $(SHL5STDLIBS) \
        $(STDSHL) \
        $(SHL5LINKRES) \
        ) >> $(MISC)$/$(SHL5TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL5TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL5TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL5VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL5LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL5VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL5VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL5TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL5TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL5OBJS) `cat /dev/null $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ)) $(SHL5STDLIBS) $(SHL5ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL5TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL6STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL6STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL6STDLIBS=
STDSHL=
.ELSE
SHL6ARCHIVES=
.ENDIF

SHL6DEF*=$(MISC)$/$(SHL6TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL6VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
SHL6VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL6VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL6IMPLIB)" != ""
USE_6IMPLIB=-implib:$(LB)$/$(SHL6IMPLIB).lib
.ELSE			# "$(SHL6IMPLIB)" != ""
USE_6IMPLIB=-implib:$(LB)$/i$(TARGET)6.lib
.ENDIF			# "$(SHL6IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_6IMPLIB_DEPS=$(LB)$/$(SHL6IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL6DEF=$(SHL6DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL6DEF=
SHL6DEPN+:=$(SHL6DEPNU)

# to activate vmaps remove "#"
#USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap

.IF "$(USE_SHL6VERSIONMAP)"!=""

.IF "$(DEF6EXPORTFILE)"!=""
.IF "$(SHL6VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF

$(USE_SHL6VERSIONMAP): \
                    $(SHL6OBJS)\
                    $(SHL6LIBS)\
                    $(DEF6EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL6OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL6OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL6OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL6OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL6LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF6EXPORTFILE) -o $@

.ELSE			# "$(DEF6EXPORTFILE)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF6EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF6EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL6VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL6VERSIONMAP)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF

$(USE_SHL6VERSIONMAP): $(SHL6VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL6VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL6VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL6VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL6RES)"!=""
SHL6RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL6RES))
SHL6ALLRES+=$(SHL6RES)
SHL6LINKRES*=$(MISC)$/$(SHL6TARGET).res
.ENDIF			# "$(SHL6RES)"!=""
.ENDIF

.IF "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""
SHL6DEFAULTRES*=$(MISC)$/$(SHL6TARGET)_def.res
SHL6ALLRES+=$(SHL6DEFAULTRES)
SHL6LINKRES*=$(MISC)$/$(SHL6TARGET).res
.ENDIF			# "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL6DESCRIPTION)"==""
#SHL6DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL6DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_description.obj
.ENDIF			# "$(NO_SHL6DESCRIPTION)"==""

.IF "$(SHL6TARGETN)"!=""
$(SHL6TARGETN) : \
                    $(SHL6OBJS)\
                    $(SHL6DESCRIPTIONOBJ)\
                    $(SHL6LIBS)\
                    $(USE_6IMPLIB_DEPS)\
                    $(USE_SHL6DEF)\
                    $(USE_SHL6VERSIONMAP)\
                    $(SHL6RES)\
                    $(SHL6VERSIONH)\
                    $(SHL6DEPN)
    @echo ------------------------------
    @echo Making: $(SHL6TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL6VERSIONOBJ:+"+\n") $(SHL6OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL6LIBS:+"+\n") $(SHL6STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL6DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL6LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL6STDLIBS:^"-l") $(SHL6OBJS) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) $(SHL6DEF)
.ENDIF
.IF "$(SHL6RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL6RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL6OBJS), $@, $(MISC)\$(@:b).map, $(SHL6LIBS) $(SHL6STDLIBS) $(STDSHL), $(SHL6DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL6DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL6DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL6ICON)" != ""
    @-+echo 1 ICON $(SHL6ICON) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL6TARGET)$(DLLPOST) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL6TARGET:b) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(SHL6DEFAULTRES)"!=""
.IF "$(SHL6ALLRES)"!=""
    +$(COPY) /b $(SHL6ALLRES:s/res /res+/) $(SHL6LINKRES)
.ENDIF			# "$(SHL6ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL6STACK) $(SHL6BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL6TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) $(SHL6OBJS) \
        $(SHL6LIBS) \
        $(SHL6STDLIBS) \
        $(STDSHL) \
        $(SHL6LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL6STDLIBS) $(STDSHL) $(SHL6RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL6STACK) $(SHL6BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) $(SHL6OBJS) \
        $(SHL6LIBS) \
        $(SHL6STDLIBS) \
        $(STDSHL) \
        $(SHL6LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        $(SHL6STACK) -out:$(SHL6TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL6IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ)   \
        $(SHL6LIBS)                         \
        $(SHL6STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL6LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL6STACK) -out:$(BIN)$/_$(SHL6TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL6IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ)    \
        $(SHL6LIBS)                         \
        $(SHL6STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL6LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL6TARGET).lnk del $(MISC)$/$(SHL6TARGET).lnk
        +if exist $(MISC)$/$(SHL6TARGET).lst del $(MISC)$/$(SHL6TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL6BASEX) \
        $(SHL6STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL6IMPLIB).exp \
        $(STDOBJ) \
        $(SHL6OBJS) \
        $(SHL6STDLIBS) \
        $(STDSHL) \
        $(SHL6LINKRES) \
        ) >> $(MISC)$/$(SHL6TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL6TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL6TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL6VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL6LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL6VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL6VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL6TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL6TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL6OBJS) `cat /dev/null $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ)) $(SHL6STDLIBS) $(SHL6ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL6TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL7STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL7STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL7STDLIBS=
STDSHL=
.ELSE
SHL7ARCHIVES=
.ENDIF

SHL7DEF*=$(MISC)$/$(SHL7TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL7VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
SHL7VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL7VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL7IMPLIB)" != ""
USE_7IMPLIB=-implib:$(LB)$/$(SHL7IMPLIB).lib
.ELSE			# "$(SHL7IMPLIB)" != ""
USE_7IMPLIB=-implib:$(LB)$/i$(TARGET)7.lib
.ENDIF			# "$(SHL7IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_7IMPLIB_DEPS=$(LB)$/$(SHL7IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL7DEF=$(SHL7DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL7DEF=
SHL7DEPN+:=$(SHL7DEPNU)

# to activate vmaps remove "#"
#USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap

.IF "$(USE_SHL7VERSIONMAP)"!=""

.IF "$(DEF7EXPORTFILE)"!=""
.IF "$(SHL7VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF

$(USE_SHL7VERSIONMAP): \
                    $(SHL7OBJS)\
                    $(SHL7LIBS)\
                    $(DEF7EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL7OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL7OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL7OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL7OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL7LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF7EXPORTFILE) -o $@

.ELSE			# "$(DEF7EXPORTFILE)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF7EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF7EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL7VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL7VERSIONMAP)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF

$(USE_SHL7VERSIONMAP): $(SHL7VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL7VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL7VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL7VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL7RES)"!=""
SHL7RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL7RES))
SHL7ALLRES+=$(SHL7RES)
SHL7LINKRES*=$(MISC)$/$(SHL7TARGET).res
.ENDIF			# "$(SHL7RES)"!=""
.ENDIF

.IF "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""
SHL7DEFAULTRES*=$(MISC)$/$(SHL7TARGET)_def.res
SHL7ALLRES+=$(SHL7DEFAULTRES)
SHL7LINKRES*=$(MISC)$/$(SHL7TARGET).res
.ENDIF			# "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL7DESCRIPTION)"==""
#SHL7DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL7DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_description.obj
.ENDIF			# "$(NO_SHL7DESCRIPTION)"==""

.IF "$(SHL7TARGETN)"!=""
$(SHL7TARGETN) : \
                    $(SHL7OBJS)\
                    $(SHL7DESCRIPTIONOBJ)\
                    $(SHL7LIBS)\
                    $(USE_7IMPLIB_DEPS)\
                    $(USE_SHL7DEF)\
                    $(USE_SHL7VERSIONMAP)\
                    $(SHL7RES)\
                    $(SHL7VERSIONH)\
                    $(SHL7DEPN)
    @echo ------------------------------
    @echo Making: $(SHL7TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL7VERSIONOBJ:+"+\n") $(SHL7OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL7LIBS:+"+\n") $(SHL7STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL7DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL7LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL7STDLIBS:^"-l") $(SHL7OBJS) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) $(SHL7DEF)
.ENDIF
.IF "$(SHL7RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL7RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL7OBJS), $@, $(MISC)\$(@:b).map, $(SHL7LIBS) $(SHL7STDLIBS) $(STDSHL), $(SHL7DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL7DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL7DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL7ICON)" != ""
    @-+echo 1 ICON $(SHL7ICON) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL7TARGET)$(DLLPOST) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL7TARGET:b) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(SHL7DEFAULTRES)"!=""
.IF "$(SHL7ALLRES)"!=""
    +$(COPY) /b $(SHL7ALLRES:s/res /res+/) $(SHL7LINKRES)
.ENDIF			# "$(SHL7ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL7STACK) $(SHL7BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL7TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) $(SHL7OBJS) \
        $(SHL7LIBS) \
        $(SHL7STDLIBS) \
        $(STDSHL) \
        $(SHL7LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL7STDLIBS) $(STDSHL) $(SHL7RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL7STACK) $(SHL7BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) $(SHL7OBJS) \
        $(SHL7LIBS) \
        $(SHL7STDLIBS) \
        $(STDSHL) \
        $(SHL7LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        $(SHL7STACK) -out:$(SHL7TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL7IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ)   \
        $(SHL7LIBS)                         \
        $(SHL7STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL7LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL7STACK) -out:$(BIN)$/_$(SHL7TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL7IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ)    \
        $(SHL7LIBS)                         \
        $(SHL7STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL7LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL7TARGET).lnk del $(MISC)$/$(SHL7TARGET).lnk
        +if exist $(MISC)$/$(SHL7TARGET).lst del $(MISC)$/$(SHL7TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL7BASEX) \
        $(SHL7STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL7IMPLIB).exp \
        $(STDOBJ) \
        $(SHL7OBJS) \
        $(SHL7STDLIBS) \
        $(STDSHL) \
        $(SHL7LINKRES) \
        ) >> $(MISC)$/$(SHL7TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL7TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL7TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL7VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL7LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL7VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL7VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL7TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL7TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL7OBJS) `cat /dev/null $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ)) $(SHL7STDLIBS) $(SHL7ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL7TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL8STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL8STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL8STDLIBS=
STDSHL=
.ELSE
SHL8ARCHIVES=
.ENDIF

SHL8DEF*=$(MISC)$/$(SHL8TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL8VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
SHL8VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL8VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL8IMPLIB)" != ""
USE_8IMPLIB=-implib:$(LB)$/$(SHL8IMPLIB).lib
.ELSE			# "$(SHL8IMPLIB)" != ""
USE_8IMPLIB=-implib:$(LB)$/i$(TARGET)8.lib
.ENDIF			# "$(SHL8IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_8IMPLIB_DEPS=$(LB)$/$(SHL8IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL8DEF=$(SHL8DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL8DEF=
SHL8DEPN+:=$(SHL8DEPNU)

# to activate vmaps remove "#"
#USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap

.IF "$(USE_SHL8VERSIONMAP)"!=""

.IF "$(DEF8EXPORTFILE)"!=""
.IF "$(SHL8VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF

$(USE_SHL8VERSIONMAP): \
                    $(SHL8OBJS)\
                    $(SHL8LIBS)\
                    $(DEF8EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL8OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL8OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL8OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL8OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL8LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF8EXPORTFILE) -o $@

.ELSE			# "$(DEF8EXPORTFILE)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF8EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF8EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL8VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL8VERSIONMAP)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF

$(USE_SHL8VERSIONMAP): $(SHL8VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL8VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL8VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL8VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL8RES)"!=""
SHL8RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL8RES))
SHL8ALLRES+=$(SHL8RES)
SHL8LINKRES*=$(MISC)$/$(SHL8TARGET).res
.ENDIF			# "$(SHL8RES)"!=""
.ENDIF

.IF "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""
SHL8DEFAULTRES*=$(MISC)$/$(SHL8TARGET)_def.res
SHL8ALLRES+=$(SHL8DEFAULTRES)
SHL8LINKRES*=$(MISC)$/$(SHL8TARGET).res
.ENDIF			# "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL8DESCRIPTION)"==""
#SHL8DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL8DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_description.obj
.ENDIF			# "$(NO_SHL8DESCRIPTION)"==""

.IF "$(SHL8TARGETN)"!=""
$(SHL8TARGETN) : \
                    $(SHL8OBJS)\
                    $(SHL8DESCRIPTIONOBJ)\
                    $(SHL8LIBS)\
                    $(USE_8IMPLIB_DEPS)\
                    $(USE_SHL8DEF)\
                    $(USE_SHL8VERSIONMAP)\
                    $(SHL8RES)\
                    $(SHL8VERSIONH)\
                    $(SHL8DEPN)
    @echo ------------------------------
    @echo Making: $(SHL8TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL8VERSIONOBJ:+"+\n") $(SHL8OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL8LIBS:+"+\n") $(SHL8STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL8DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL8LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL8STDLIBS:^"-l") $(SHL8OBJS) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) $(SHL8DEF)
.ENDIF
.IF "$(SHL8RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL8RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL8OBJS), $@, $(MISC)\$(@:b).map, $(SHL8LIBS) $(SHL8STDLIBS) $(STDSHL), $(SHL8DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL8DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL8DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL8ICON)" != ""
    @-+echo 1 ICON $(SHL8ICON) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL8TARGET)$(DLLPOST) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL8TARGET:b) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(SHL8DEFAULTRES)"!=""
.IF "$(SHL8ALLRES)"!=""
    +$(COPY) /b $(SHL8ALLRES:s/res /res+/) $(SHL8LINKRES)
.ENDIF			# "$(SHL8ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL8STACK) $(SHL8BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL8TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) $(SHL8OBJS) \
        $(SHL8LIBS) \
        $(SHL8STDLIBS) \
        $(STDSHL) \
        $(SHL8LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL8STDLIBS) $(STDSHL) $(SHL8RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL8STACK) $(SHL8BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) $(SHL8OBJS) \
        $(SHL8LIBS) \
        $(SHL8STDLIBS) \
        $(STDSHL) \
        $(SHL8LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        $(SHL8STACK) -out:$(SHL8TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL8IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ)   \
        $(SHL8LIBS)                         \
        $(SHL8STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL8LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL8STACK) -out:$(BIN)$/_$(SHL8TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL8IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ)    \
        $(SHL8LIBS)                         \
        $(SHL8STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL8LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL8TARGET).lnk del $(MISC)$/$(SHL8TARGET).lnk
        +if exist $(MISC)$/$(SHL8TARGET).lst del $(MISC)$/$(SHL8TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL8BASEX) \
        $(SHL8STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL8IMPLIB).exp \
        $(STDOBJ) \
        $(SHL8OBJS) \
        $(SHL8STDLIBS) \
        $(STDSHL) \
        $(SHL8LINKRES) \
        ) >> $(MISC)$/$(SHL8TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL8TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL8TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL8VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL8LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL8VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL8VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL8TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL8TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL8OBJS) `cat /dev/null $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ)) $(SHL8STDLIBS) $(SHL8ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL8TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL9STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL9STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL9STDLIBS=
STDSHL=
.ELSE
SHL9ARCHIVES=
.ENDIF

SHL9DEF*=$(MISC)$/$(SHL9TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL9VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
SHL9VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL9VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL9IMPLIB)" != ""
USE_9IMPLIB=-implib:$(LB)$/$(SHL9IMPLIB).lib
.ELSE			# "$(SHL9IMPLIB)" != ""
USE_9IMPLIB=-implib:$(LB)$/i$(TARGET)9.lib
.ENDIF			# "$(SHL9IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_9IMPLIB_DEPS=$(LB)$/$(SHL9IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL9DEF=$(SHL9DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL9DEF=
SHL9DEPN+:=$(SHL9DEPNU)

# to activate vmaps remove "#"
#USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap

.IF "$(USE_SHL9VERSIONMAP)"!=""

.IF "$(DEF9EXPORTFILE)"!=""
.IF "$(SHL9VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF

$(USE_SHL9VERSIONMAP): \
                    $(SHL9OBJS)\
                    $(SHL9LIBS)\
                    $(DEF9EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL9OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL9OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL9OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL9OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL9LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF9EXPORTFILE) -o $@

.ELSE			# "$(DEF9EXPORTFILE)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF9EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF9EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL9VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL9VERSIONMAP)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF

$(USE_SHL9VERSIONMAP): $(SHL9VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL9VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL9VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL9VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL9RES)"!=""
SHL9RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL9RES))
SHL9ALLRES+=$(SHL9RES)
SHL9LINKRES*=$(MISC)$/$(SHL9TARGET).res
.ENDIF			# "$(SHL9RES)"!=""
.ENDIF

.IF "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""
SHL9DEFAULTRES*=$(MISC)$/$(SHL9TARGET)_def.res
SHL9ALLRES+=$(SHL9DEFAULTRES)
SHL9LINKRES*=$(MISC)$/$(SHL9TARGET).res
.ENDIF			# "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL9DESCRIPTION)"==""
#SHL9DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL9DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_description.obj
.ENDIF			# "$(NO_SHL9DESCRIPTION)"==""

.IF "$(SHL9TARGETN)"!=""
$(SHL9TARGETN) : \
                    $(SHL9OBJS)\
                    $(SHL9DESCRIPTIONOBJ)\
                    $(SHL9LIBS)\
                    $(USE_9IMPLIB_DEPS)\
                    $(USE_SHL9DEF)\
                    $(USE_SHL9VERSIONMAP)\
                    $(SHL9RES)\
                    $(SHL9VERSIONH)\
                    $(SHL9DEPN)
    @echo ------------------------------
    @echo Making: $(SHL9TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL9VERSIONOBJ:+"+\n") $(SHL9OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL9LIBS:+"+\n") $(SHL9STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL9DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL9LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL9STDLIBS:^"-l") $(SHL9OBJS) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) $(SHL9DEF)
.ENDIF
.IF "$(SHL9RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL9RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL9OBJS), $@, $(MISC)\$(@:b).map, $(SHL9LIBS) $(SHL9STDLIBS) $(STDSHL), $(SHL9DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL9DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL9DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL9ICON)" != ""
    @-+echo 1 ICON $(SHL9ICON) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL9TARGET)$(DLLPOST) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL9TARGET:b) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(SHL9DEFAULTRES)"!=""
.IF "$(SHL9ALLRES)"!=""
    +$(COPY) /b $(SHL9ALLRES:s/res /res+/) $(SHL9LINKRES)
.ENDIF			# "$(SHL9ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL9STACK) $(SHL9BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL9TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) $(SHL9OBJS) \
        $(SHL9LIBS) \
        $(SHL9STDLIBS) \
        $(STDSHL) \
        $(SHL9LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL9STDLIBS) $(STDSHL) $(SHL9RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL9STACK) $(SHL9BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) $(SHL9OBJS) \
        $(SHL9LIBS) \
        $(SHL9STDLIBS) \
        $(STDSHL) \
        $(SHL9LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        $(SHL9STACK) -out:$(SHL9TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL9IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ)   \
        $(SHL9LIBS)                         \
        $(SHL9STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL9LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL9STACK) -out:$(BIN)$/_$(SHL9TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL9IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ)    \
        $(SHL9LIBS)                         \
        $(SHL9STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL9LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL9TARGET).lnk del $(MISC)$/$(SHL9TARGET).lnk
        +if exist $(MISC)$/$(SHL9TARGET).lst del $(MISC)$/$(SHL9TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL9BASEX) \
        $(SHL9STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL9IMPLIB).exp \
        $(STDOBJ) \
        $(SHL9OBJS) \
        $(SHL9STDLIBS) \
        $(STDSHL) \
        $(SHL9LINKRES) \
        ) >> $(MISC)$/$(SHL9TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL9TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL9TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL9VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL9LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL9VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL9VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL9TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL9TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL9OBJS) `cat /dev/null $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ)) $(SHL9STDLIBS) $(SHL9ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL9TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(OS)"=="AIX"
SHL10STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)"=="MACOSX"
SHL10STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL10STDLIBS=
STDSHL=
.ELSE
SHL10ARCHIVES=
.ENDIF

SHL10DEF*=$(MISC)$/$(SHL10TARGET).def

.IF "$(VERSIONOBJ)"!=""
.IF "$(UPDATER)"=="YES"
SHL10VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
SHL10VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
SHL10VERSIONOBJ:=$(VERSIONOBJ)
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "MSC"
.IF "$(SHL10IMPLIB)" != ""
USE_10IMPLIB=-implib:$(LB)$/$(SHL10IMPLIB).lib
.ELSE			# "$(SHL10IMPLIB)" != ""
USE_10IMPLIB=-implib:$(LB)$/i$(TARGET)10.lib
.ENDIF			# "$(SHL10IMPLIB)" != ""
.ENDIF			# "$(COM)" == "MSC"

.IF "$(USE_DEFFILE)"==""
USE_10IMPLIB_DEPS=$(LB)$/$(SHL10IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL10DEF=$(SHL10DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL10DEF=
SHL10DEPN+:=$(SHL10DEPNU)

# to activate vmaps remove "#"
#USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap

.IF "$(USE_SHL10VERSIONMAP)"!=""

.IF "$(DEF10EXPORTFILE)"!=""
.IF "$(SHL10VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF

$(USE_SHL10VERSIONMAP): \
                    $(SHL10OBJS)\
                    $(SHL10LIBS)\
                    $(DEF10EXPORTFILE)
    @+$(RM) $@.dump
.IF "$(SHL10OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-+nm $(SHL10OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @+nm $(SHL10OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL10OBJS)!"=""
    @+$(TYPE) /dev/null $(SHL10LIBS:s/.lib/.dump/) >> $@.dump
    @+$(SOLARENV)$/bin$/genmap -d $@.dump -e $(DEF10EXPORTFILE) -o $@

.ELSE			# "$(DEF10EXPORTFILE)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) .PHONY: 
    @+echo -----------------------------
    @+echo DEF10EXPORTFILE not set!
    @+echo -----------------------------
#	force_dmake_to_error
.ENDIF			# "$(DEF10EXPORTFILE)"!=""
.ELSE			# "$(USE_SHL10VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL10VERSIONMAP)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10VERSIONMAP:f)
.IF "$(OS)"!="MACOSX"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF

$(USE_SHL10VERSIONMAP): $(SHL10VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(SHL10VERSIONMAP) > $@
    @+chmod a+w $@
    
.ENDIF			# "$(SHL10VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL10VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(NO_REC_RES)"!=""
.IF "$(SHL10RES)"!=""
SHL10RES!:=$(subst,$(RES)$/,$(RES)$/$(defaultlangext)$/ $(SHL10RES))
SHL10ALLRES+=$(SHL10RES)
SHL10LINKRES*=$(MISC)$/$(SHL10TARGET).res
.ENDIF			# "$(SHL10RES)"!=""
.ENDIF

.IF "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""
SHL10DEFAULTRES*=$(MISC)$/$(SHL10TARGET)_def.res
SHL10ALLRES+=$(SHL10DEFAULTRES)
SHL10LINKRES*=$(MISC)$/$(SHL10TARGET).res
.ENDIF			# "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""

.IF "$(NO_SHL10DESCRIPTION)"==""
#SHL10DESCRIPTIONOBJ*=$(SLO)$/default_description.obj
SHL10DESCRIPTIONOBJ*=$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_description.obj
.ENDIF			# "$(NO_SHL10DESCRIPTION)"==""

.IF "$(SHL10TARGETN)"!=""
$(SHL10TARGETN) : \
                    $(SHL10OBJS)\
                    $(SHL10DESCRIPTIONOBJ)\
                    $(SHL10LIBS)\
                    $(USE_10IMPLIB_DEPS)\
                    $(USE_SHL10DEF)\
                    $(USE_SHL10VERSIONMAP)\
                    $(SHL10RES)\
                    $(SHL10VERSIONH)\
                    $(SHL10DEPN)
    @echo ------------------------------
    @echo Making: $(SHL10TARGETN)
.IF "$(UPDATER)"=="YES"
        @-+$(RM) $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj 
.ENDIF
.IF "$(GUI)"=="OS2"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="ICC"
        $(CC) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj /Ge+ /Gs+ /Gt+ /Gd+ -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE			# "$(COM)"=="ICC" 
        $(CC) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj -Zomf -Zso -Zsys -DOS2 $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="ICC" 
.ENDIF			# "$(UPDATER)"=="YES"
#
#	todo: try with $(LINKEXTENDLINE)!
#
    +-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) @$(mktmp \
        $(STDSLO:+"+\n") $(SHL10VERSIONOBJ:+"+\n") $(SHL10OBJS:+"+\n")), \
        $(@), \
        $(MISC)$/$(@:b).map, \
        @$(mktmp $(SHL10LIBS:+"+\n") $(SHL10STDLIBS:+"+\n") $(STDSHL:+"+\n")), \
        $(SHL10DEF:+"\n")
.ELSE
    $(LINK) -o $@ -Zdll -Zmap=$(MISC)$/$(@:b).map -L$(LB)  $(SHL10LIBS:^"-l") -Ln:\toolkit4\lib -Ln:\emx09d\lib\mt  -Ln:\emx09d\lib -L$(SOLARLIBDIR) $(STDSLO) $(STDSHL:^"-l") $(SHL10STDLIBS:^"-l") $(SHL10OBJS) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) $(SHL10DEF)
.ENDIF
.IF "$(SHL10RES)" != ""
    $(RCLINK) $(RCLINKFLAGS) $(SHL10RES) $@
.ENDIF			# "$(COMEX)"=="3"
.ENDIF			# "$(GUI)"=="OS2"
.IF "$(GUI)" == "WIN"
.IF "$(COM)"=="BLC"
    +$(LINK) @$(mktmp$ $(LINKFLAGS) $(LINKFLAGSSHL) $(STDSLO) $(SHL10OBJS), $@, $(MISC)\$(@:b).map, $(SHL10LIBS) $(SHL10STDLIBS) $(STDSHL), $(SHL10DEF)) >& $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(TYPE) $(TMP)$/$(PRJNAME)$(USER).tmp
    @+$(RM) $(TMP)$/$(PRJNAME)$(USER).tmp
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)" == "WNT"
    @+if not exist $(FUNCORD) $(TOUCH) $(FUNCORD)
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            gcc -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE
            cl -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL10DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL10DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL10ICON)" != ""
    @-+echo 1 ICON $(SHL10ICON) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL10TARGET)$(DLLPOST) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL10TARGET:b) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(SHL10DEFAULTRES)"!=""
.IF "$(SHL10ALLRES)"!=""
    +$(COPY) /b $(SHL10ALLRES:s/res /res+/) $(SHL10LINKRES)
.ENDIF			# "$(SHL10ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL10STACK) $(SHL10BASEX)	\
        /DEBUG /PDB:NONE \
        -out:$(BIN)$/_$(SHL10TARGET).dll \
        -map:$(MISC)$/_$(@:b).map \
        -def:$(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) $(SHL10OBJS) \
        $(SHL10LIBS) \
        $(SHL10STDLIBS) \
        $(STDSHL) \
        $(SHL10LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL10STDLIBS) $(STDSHL) $(SHL10RES) >> $(MISC)$/$(@:b).cmd
    $(MISC)$/$(@:b).cmd
.ELSE
    $(LINK) @$(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL10STACK) $(SHL10BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) $(SHL10OBJS) \
        $(SHL10LIBS) \
        $(SHL10STDLIBS) \
        $(STDSHL) \
        $(SHL10LINKRES) \
    )
.ENDIF			# "$(COM)"=="GCC"
.ELSE			# "$(USE_DEFFILE)"!=""
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        $(SHL10STACK) -out:$(SHL10TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL10IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ)   \
        $(SHL10LIBS)                         \
        $(SHL10STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL10LINKRES) \
    )
.IF "$(BOTH)"!=""
.IF "$(PRODUCT)"=="full"
.IF "$(UPDATER)"=="YES"
#
#	product mit debug gelinkt
#
    $(LINK) @$(mktmp	$(LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        /DEBUG /PDB:NONE \
        $(SHL10STACK) -out:$(BIN)$/_$(SHL10TARGET).dll	\
        -map:$(MISC)$/_$(@:B).map				\
        $(LB)$/$(SHL10IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ)    \
        $(SHL10LIBS)                         \
        $(SHL10STDLIBS)                      \
        $(STDSHL)                           \
        $(SHL10LINKRES) \
    )
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(PRODUCT)"=="full"
.ENDIF			# "$(BOTH)"!=""
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +if exist $(MISC)$/$(SHL10TARGET).lnk del $(MISC)$/$(SHL10TARGET).lnk
        +if exist $(MISC)$/$(SHL10TARGET).lst del $(MISC)$/$(SHL10TARGET).lst
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
        +type $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL10BASEX) \
        $(SHL10STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL10IMPLIB).exp \
        $(STDOBJ) \
        $(SHL10OBJS) \
        $(SHL10STDLIBS) \
        $(STDSHL) \
        $(SHL10LINKRES) \
        ) >> $(MISC)$/$(SHL10TARGET).lnk
        +type $(MISC)$/linkinc.ls  >> $(MISC)$/$(SHL10TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL10TARGET).lnk
#		+if exist $(MISC)$/linkinc.ls del $(MISC)$/linkinc.ls
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.cxx
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL10VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL10LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(OS)"=="S390"
    +mv -f ($@:s/$(DLLPOST)/.x/) $(LB)
.ENDIF
    @ls -l $@
.IF "$(OS)"=="MACOSX"
# This is a hack as libstatic and libcppuhelper have a circular dependency
.IF "$(PRJNAME)"=="cppuhelper"
    @echo "------------------------------"
    @echo "Rerunning static data member initializations"
    @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)"
.ENDIF
.IF "$(SHL10VERSIONMAP)"!=""
    @strip -i -r -u -s $(SHL10VERSIONMAP) $@
.ENDIF
    @echo "Making: $@.framework"
    @create-bundle $@
.ENDIF
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="MACOSX"
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL10TARGETN).framework
.ELSE
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL10TARGETN)
.ENDIF
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL10OBJS) `cat /dev/null $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ)) $(SHL10STDLIBS) $(SHL10ARCHIVES) $(STDSHL) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.ENDIF			# "$(SHL10TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL1IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL1LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL1TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL1IMPLIBN):	\
                    $(SHL1DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL1IMPLIBN):	\
                    $(SHL1LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL1IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL1IMPLIBN) \
    -def:$(SHL1DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL1TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL1DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL1TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL2IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL2LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL2TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL2IMPLIBN):	\
                    $(SHL2DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL2IMPLIBN):	\
                    $(SHL2LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL2IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL2IMPLIBN) \
    -def:$(SHL2DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL2TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL2DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL2TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL3IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL3LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL3TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL3IMPLIBN):	\
                    $(SHL3DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL3IMPLIBN):	\
                    $(SHL3LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL3IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL3IMPLIBN) \
    -def:$(SHL3DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL3TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL3DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL3TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL4IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL4LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL4TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL4IMPLIBN):	\
                    $(SHL4DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL4IMPLIBN):	\
                    $(SHL4LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL4IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL4IMPLIBN) \
    -def:$(SHL4DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL4TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL4DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL4TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL5IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL5LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL5TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL5IMPLIBN):	\
                    $(SHL5DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL5IMPLIBN):	\
                    $(SHL5LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL5IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL5IMPLIBN) \
    -def:$(SHL5DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL5TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL5DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL5TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL6IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL6LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL6TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL6IMPLIBN):	\
                    $(SHL6DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL6IMPLIBN):	\
                    $(SHL6LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL6IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL6IMPLIBN) \
    -def:$(SHL6DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL6TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL6DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL6TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL7IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL7LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL7TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL7IMPLIBN):	\
                    $(SHL7DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL7IMPLIBN):	\
                    $(SHL7LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL7IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL7IMPLIBN) \
    -def:$(SHL7DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL7TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL7DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL7TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL8IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL8LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL8TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL8IMPLIBN):	\
                    $(SHL8DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL8IMPLIBN):	\
                    $(SHL8LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL8IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL8IMPLIBN) \
    -def:$(SHL8DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL8TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL8DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL8TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL9IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL9LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL9TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL9IMPLIBN):	\
                    $(SHL9DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL9IMPLIBN):	\
                    $(SHL9LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL9IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL9IMPLIBN) \
    -def:$(SHL9DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL9TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL9DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL9TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL10IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIBDEPN=$(SHL10LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHLTARGET=$(SHL10TARGETN)
.ENDIF

.IF "$(GUI)" != "UNX"
$(SHL10IMPLIBN):	\
                    $(SHL10DEF) \
                    $(USE_SHLTARGET) \
                    $(USELIBDEPN)
.ELSE
$(SHL10IMPLIBN):	\
                    $(SHL10LIBS)
.ENDIF
    @echo ------------------------------
    @echo Making: $(SHL10IMPLIBN)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL10IMPLIBN) \
    -def:$(SHL10DEF) )
.ELSE			# "$(GUI)" == "WNT"
    @+if exist $@ $(TOUCH) $@
    @+if not exist $@ echo rebuild $(SHL10TARGETN) to get $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL10DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL10TARGETN)
.ENDIF
.ELSE
    @echo keine ImportLibs auf Mac und *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
