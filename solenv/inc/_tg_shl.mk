# unroll begin

.IF "$(SHL1TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL1STDLIBS=
STDSHL=
.ELSE
SHL1ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL1CODETYPE)"=="C"
SHL1LINKER=$(LINKC)
SHL1STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL1LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL1CODETYPE)"=="C"
SHL1LINKER=$(LINK)
SHL1STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL1LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL1CODETYPE)"=="C"

SHL1RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL1RPATH)*=/ERROR:/Bad_SHL1RPATH_value
SHL1LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL1RPATH))

.IF "$(SHL1USE_EXPORTS)"==""
SHL1DEF*=$(MISC)/$(SHL1TARGET).def
.ENDIF			# "$(SHL1USE_EXPORTS)"==""

EXTRALIBPATHS1=$(EXTRALIBPATHS)
.IF "$(SHL1NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS1+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL1NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL1VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL1VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL1VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL1VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL1VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL1VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL1VERSIONOBJ:b).h/ > $@

.INIT : $(SHL1VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL1IMPLIB)" == ""
SHL1IMPLIB=i$(TARGET)_t1
.ENDIF			# "$(SHL1IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_1IMPLIB=-implib:$(LB)/$(SHL1IMPLIB).lib
SHL1IMPLIBN=$(LB)/$(SHL1IMPLIB).lib
.ELSE
USE_1IMPLIB=-Wl,--out-implib=$(SHL1IMPLIBN)
SHL1IMPLIBN=$(LB)/lib$(SHL1IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL1IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_1IMPLIB_DEPS=$(LB)/$(SHL1IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL1DEF=$(SHL1DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL1DEF=
SHL1DEPN+:=$(SHL1DEPNU)

.IF "$(SHL1VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL1VERSIONMAP=$(MISC)/$(SHL1TARGET).vmap
.ENDIF			# "$(SHL1VERSIONMAP)"==""

.IF "$(USE_SHL1VERSIONMAP)"!=""

.IF "$(SHL1FILTERFILE)"!=""
.IF "$(SHL1VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL1VERSIONMAP=$(MISC)/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF

$(USE_SHL1VERSIONMAP): \
                    $(SHL1OBJS)\
                    $(SHL1LIBS)\
                    $(SHL1FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL1OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL1OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL1OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL1OBJS)!"=""
    @$(TYPE) /dev/null $(SHL1LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL1INTERFACE) -f $(SHL1FILTERFILE) -m $@

.ELSE			# "$(SHL1FILTERFILE)"!=""
USE_SHL1VERSIONMAP=$(MISC)/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL1FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL1FILTERFILE)"!=""
.ELSE			# "$(USE_SHL1VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL1VERSIONMAP)"!=""
USE_SHL1VERSIONMAP=$(MISC)/$(SHL1VERSIONMAP:b)_$(SHL1TARGET)$(SHL1VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL1VERSIONMAP): $(SHL1OBJS) $(SHL1LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL1VERSIONMAP) .ERRREMOVE: $(SHL1VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL1VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL1VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL1VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL1OBJS)"!=""
    -echo $(foreach,i,$(SHL1OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL1LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL1LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL1VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL1VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL1SONAME=\"$(SONAME_SWITCH)$(SHL1TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL1RES)"!=""
SHL1ALLRES+=$(SHL1RES)
SHL1LINKRES*=$(MISC)/$(SHL1TARGET).res
SHL1LINKRESO*=$(MISC)/$(SHL1TARGET)_res.o
.ENDIF			# "$(SHL1RES)"!=""

.IF "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""
SHL1DEFAULTRES*=$(MISC)/$(SHL1TARGET)_def.res
SHL1ALLRES+=$(SHL1DEFAULTRES)
SHL1LINKRES*=$(MISC)/$(SHL1TARGET).res
SHL1LINKRESO*=$(MISC)/$(SHL1TARGET)_res.o
.ENDIF			# "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL1TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL1LIBS)"!=""
$(MISC)/$(SHL1TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL1LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL1LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL1TARGETN:b)_linkinc.ls
$(SHL1TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL1IMPLIBN)" != ""
$(SHL1IMPLIBN) : $(SHL1TARGETN)
.ENDIF

$(SHL1TARGETN) : \
                    $(SHL1OBJS)\
                    $(SHL1LIBS)\
                    $(USE_1IMPLIB_DEPS)\
                    $(USE_SHL1DEF)\
                    $(USE_SHL1VERSIONMAP)\
                    $(SHL1RES)\
                    $(SHL1DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL1DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL1DEFAULTRES:b).rc
.IF "$(SHL1ICON)" != ""
    @echo 1 ICON $(SHL1ICON) >> $(MISC)/$(SHL1DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL1ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL1DEFAULTRES:b).rc
.ELSE			# "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL1TARGET)$(DLLPOST) >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL1TARGET:b) >> $(MISC)/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL1DEFAULTRES) $(MISC)/$(SHL1DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL1DEFAULTRES:b).rc $(SHL1DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL1DEFAULTRES)"!=""
.IF "$(SHL1ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL1ALLRES) > $(SHL1LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL1LINKRES) $(SHL1LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL1ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL1LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL1STDLIBS) -Wl,--end-group \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL1OBJLIST:=$(STDOBJ) $(SHL1OBJS) $(SHL1LINKRESO) $(shell $(TYPE) /dev/null $(SHL1LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB1NAME)"!=""	# do not have to include objs
    @noop $(assign DEF1OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF1OBJLIST) $(assign ALL1OBJLIST:=$(ALL1OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB1NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL1LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL1STDLIBS) -Wl,--end-group \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL1USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL1LINKER) @$(mktmp \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL1STACK) $(SHL1BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1OBJS) \
        $(SHL1LIBS) \
        $(SHL1STDLIBS) \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL1LINKER) @$(mktmp	$(SHL1LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        $(SHL1STACK) -out:$(SHL1TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL1IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ) \
        $(SHL1LIBS)                         \
        $(SHL1STDLIBS)                      \
        $(SHL1STDSHL) $(STDSHL1)                           \
        $(SHL1LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL1USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL1LINKER) @$(mktmp	$(SHL1LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        $(SHL1STACK) -out:$(SHL1TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_1IMPLIB) \
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ))   \
        $(SHL1LIBS) \
        @$(mktmp $(SHL1STDLIBS)                      \
        $(SHL1STDSHL) $(STDSHL1)                           \
        $(SHL1LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL1USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL1TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL1TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL1BASEX) \
        $(SHL1STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL1IMPLIB).exp \
        $(STDOBJ) \
        $(SHL1OBJS) \
        $(SHL1STDLIBS) \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRES) \
        ) >> $(MISC)/$(SHL1TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL1TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL1TARGET).lnk
        $(COMMAND_ECHO)$(SHL1LINKER) @$(MISC)/$(SHL1TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_1.cmd
    @echo $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) \
    `cat /dev/null $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL1LINKER) $(SHL1LINKFLAGS) $(SHL1VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(SHL1STDSHL) $(STDSHL1) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_1.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_1.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_1.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_1.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_1.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL1RPATH) $@
.IF "$(SHL1CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL1CREATEJNILIB)"!=""
.IF "$(SHL1NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS1) $(SHL1TARGETN)
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB1FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL1OBJS)) $(shell cat /dev/null $(LIB1TARGET) $(SHL1LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_1.cmd
    @echo $(SHL1LINKER) $(SHL1LINKFLAGS) $(SHL1SONAME) $(LINKFLAGSSHL) $(SHL1VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL1LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(SHL1STDSHL) $(STDSHL1) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_1.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_1.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_1.cmd
.IF "$(SHL1NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL1TARGETN) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS1) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS1) -- -s $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL1TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL1TARGETN:f) $(SHL1TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL1TARGETN)"!=""

# unroll begin

.IF "$(SHL2TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL2STDLIBS=
STDSHL=
.ELSE
SHL2ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL2CODETYPE)"=="C"
SHL2LINKER=$(LINKC)
SHL2STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL2LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL2CODETYPE)"=="C"
SHL2LINKER=$(LINK)
SHL2STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL2LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL2CODETYPE)"=="C"

SHL2RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL2RPATH)*=/ERROR:/Bad_SHL2RPATH_value
SHL2LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL2RPATH))

.IF "$(SHL2USE_EXPORTS)"==""
SHL2DEF*=$(MISC)/$(SHL2TARGET).def
.ENDIF			# "$(SHL2USE_EXPORTS)"==""

EXTRALIBPATHS2=$(EXTRALIBPATHS)
.IF "$(SHL2NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS2+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL2NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL2VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL2VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL2VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL2VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL2VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL2VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL2VERSIONOBJ:b).h/ > $@

.INIT : $(SHL2VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL2IMPLIB)" == ""
SHL2IMPLIB=i$(TARGET)_t2
.ENDIF			# "$(SHL2IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_2IMPLIB=-implib:$(LB)/$(SHL2IMPLIB).lib
SHL2IMPLIBN=$(LB)/$(SHL2IMPLIB).lib
.ELSE
USE_2IMPLIB=-Wl,--out-implib=$(SHL2IMPLIBN)
SHL2IMPLIBN=$(LB)/lib$(SHL2IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL2IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_2IMPLIB_DEPS=$(LB)/$(SHL2IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL2DEF=$(SHL2DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL2DEF=
SHL2DEPN+:=$(SHL2DEPNU)

.IF "$(SHL2VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL2VERSIONMAP=$(MISC)/$(SHL2TARGET).vmap
.ENDIF			# "$(SHL2VERSIONMAP)"==""

.IF "$(USE_SHL2VERSIONMAP)"!=""

.IF "$(SHL2FILTERFILE)"!=""
.IF "$(SHL2VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL2VERSIONMAP=$(MISC)/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF

$(USE_SHL2VERSIONMAP): \
                    $(SHL2OBJS)\
                    $(SHL2LIBS)\
                    $(SHL2FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL2OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL2OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL2OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL2OBJS)!"=""
    @$(TYPE) /dev/null $(SHL2LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL2INTERFACE) -f $(SHL2FILTERFILE) -m $@

.ELSE			# "$(SHL2FILTERFILE)"!=""
USE_SHL2VERSIONMAP=$(MISC)/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL2FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL2FILTERFILE)"!=""
.ELSE			# "$(USE_SHL2VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL2VERSIONMAP)"!=""
USE_SHL2VERSIONMAP=$(MISC)/$(SHL2VERSIONMAP:b)_$(SHL2TARGET)$(SHL2VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL2VERSIONMAP): $(SHL2OBJS) $(SHL2LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL2VERSIONMAP) .ERRREMOVE: $(SHL2VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL2VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL2VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL2VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL2OBJS)"!=""
    -echo $(foreach,i,$(SHL2OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL2LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL2LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL2VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL2VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL2SONAME=\"$(SONAME_SWITCH)$(SHL2TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL2RES)"!=""
SHL2ALLRES+=$(SHL2RES)
SHL2LINKRES*=$(MISC)/$(SHL2TARGET).res
SHL2LINKRESO*=$(MISC)/$(SHL2TARGET)_res.o
.ENDIF			# "$(SHL2RES)"!=""

.IF "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""
SHL2DEFAULTRES*=$(MISC)/$(SHL2TARGET)_def.res
SHL2ALLRES+=$(SHL2DEFAULTRES)
SHL2LINKRES*=$(MISC)/$(SHL2TARGET).res
SHL2LINKRESO*=$(MISC)/$(SHL2TARGET)_res.o
.ENDIF			# "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL2TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL2LIBS)"!=""
$(MISC)/$(SHL2TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL2LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL2LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL2TARGETN:b)_linkinc.ls
$(SHL2TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL2IMPLIBN)" != ""
$(SHL2IMPLIBN) : $(SHL2TARGETN)
.ENDIF

$(SHL2TARGETN) : \
                    $(SHL2OBJS)\
                    $(SHL2LIBS)\
                    $(USE_2IMPLIB_DEPS)\
                    $(USE_SHL2DEF)\
                    $(USE_SHL2VERSIONMAP)\
                    $(SHL2RES)\
                    $(SHL2DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL2DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL2DEFAULTRES:b).rc
.IF "$(SHL2ICON)" != ""
    @echo 1 ICON $(SHL2ICON) >> $(MISC)/$(SHL2DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL2ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL2DEFAULTRES:b).rc
.ELSE			# "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL2TARGET)$(DLLPOST) >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL2TARGET:b) >> $(MISC)/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL2DEFAULTRES) $(MISC)/$(SHL2DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL2DEFAULTRES:b).rc $(SHL2DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL2DEFAULTRES)"!=""
.IF "$(SHL2ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL2ALLRES) > $(SHL2LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL2LINKRES) $(SHL2LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL2ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL2LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL2STDLIBS) -Wl,--end-group \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL2OBJLIST:=$(STDOBJ) $(SHL2OBJS) $(SHL2LINKRESO) $(shell $(TYPE) /dev/null $(SHL2LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB2NAME)"!=""	# do not have to include objs
    @noop $(assign DEF2OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF2OBJLIST) $(assign ALL2OBJLIST:=$(ALL2OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB2NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL2LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL2STDLIBS) -Wl,--end-group \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL2USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL2LINKER) @$(mktmp \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL2STACK) $(SHL2BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2OBJS) \
        $(SHL2LIBS) \
        $(SHL2STDLIBS) \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL2LINKER) @$(mktmp	$(SHL2LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        $(SHL2STACK) -out:$(SHL2TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL2IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ) \
        $(SHL2LIBS)                         \
        $(SHL2STDLIBS)                      \
        $(SHL2STDSHL) $(STDSHL2)                           \
        $(SHL2LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL2USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL2LINKER) @$(mktmp	$(SHL2LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        $(SHL2STACK) -out:$(SHL2TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_2IMPLIB) \
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ))   \
        $(SHL2LIBS) \
        @$(mktmp $(SHL2STDLIBS)                      \
        $(SHL2STDSHL) $(STDSHL2)                           \
        $(SHL2LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL2USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL2TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL2TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL2BASEX) \
        $(SHL2STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL2IMPLIB).exp \
        $(STDOBJ) \
        $(SHL2OBJS) \
        $(SHL2STDLIBS) \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRES) \
        ) >> $(MISC)/$(SHL2TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL2TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL2TARGET).lnk
        $(COMMAND_ECHO)$(SHL2LINKER) @$(MISC)/$(SHL2TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_2.cmd
    @echo $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) \
    `cat /dev/null $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL2LINKER) $(SHL2LINKFLAGS) $(SHL2VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(SHL2STDSHL) $(STDSHL2) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_2.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_2.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_2.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_2.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_2.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL2RPATH) $@
.IF "$(SHL2CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL2CREATEJNILIB)"!=""
.IF "$(SHL2NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS2) $(SHL2TARGETN)
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB2FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL2OBJS)) $(shell cat /dev/null $(LIB2TARGET) $(SHL2LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_2.cmd
    @echo $(SHL2LINKER) $(SHL2LINKFLAGS) $(SHL2SONAME) $(LINKFLAGSSHL) $(SHL2VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL2LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(SHL2STDSHL) $(STDSHL2) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_2.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_2.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_2.cmd
.IF "$(SHL2NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL2TARGETN) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS2) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS2) -- -s $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL2TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL2TARGETN:f) $(SHL2TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL2TARGETN)"!=""

# unroll begin

.IF "$(SHL3TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL3STDLIBS=
STDSHL=
.ELSE
SHL3ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL3CODETYPE)"=="C"
SHL3LINKER=$(LINKC)
SHL3STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL3LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL3CODETYPE)"=="C"
SHL3LINKER=$(LINK)
SHL3STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL3LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL3CODETYPE)"=="C"

SHL3RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL3RPATH)*=/ERROR:/Bad_SHL3RPATH_value
SHL3LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL3RPATH))

.IF "$(SHL3USE_EXPORTS)"==""
SHL3DEF*=$(MISC)/$(SHL3TARGET).def
.ENDIF			# "$(SHL3USE_EXPORTS)"==""

EXTRALIBPATHS3=$(EXTRALIBPATHS)
.IF "$(SHL3NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS3+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL3NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL3VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL3VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL3VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL3VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL3VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL3VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL3VERSIONOBJ:b).h/ > $@

.INIT : $(SHL3VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL3IMPLIB)" == ""
SHL3IMPLIB=i$(TARGET)_t3
.ENDIF			# "$(SHL3IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_3IMPLIB=-implib:$(LB)/$(SHL3IMPLIB).lib
SHL3IMPLIBN=$(LB)/$(SHL3IMPLIB).lib
.ELSE
USE_3IMPLIB=-Wl,--out-implib=$(SHL3IMPLIBN)
SHL3IMPLIBN=$(LB)/lib$(SHL3IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL3IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_3IMPLIB_DEPS=$(LB)/$(SHL3IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL3DEF=$(SHL3DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL3DEF=
SHL3DEPN+:=$(SHL3DEPNU)

.IF "$(SHL3VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL3VERSIONMAP=$(MISC)/$(SHL3TARGET).vmap
.ENDIF			# "$(SHL3VERSIONMAP)"==""

.IF "$(USE_SHL3VERSIONMAP)"!=""

.IF "$(SHL3FILTERFILE)"!=""
.IF "$(SHL3VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL3VERSIONMAP=$(MISC)/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF

$(USE_SHL3VERSIONMAP): \
                    $(SHL3OBJS)\
                    $(SHL3LIBS)\
                    $(SHL3FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL3OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL3OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL3OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL3OBJS)!"=""
    @$(TYPE) /dev/null $(SHL3LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL3INTERFACE) -f $(SHL3FILTERFILE) -m $@

.ELSE			# "$(SHL3FILTERFILE)"!=""
USE_SHL3VERSIONMAP=$(MISC)/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL3FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL3FILTERFILE)"!=""
.ELSE			# "$(USE_SHL3VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL3VERSIONMAP)"!=""
USE_SHL3VERSIONMAP=$(MISC)/$(SHL3VERSIONMAP:b)_$(SHL3TARGET)$(SHL3VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL3VERSIONMAP): $(SHL3OBJS) $(SHL3LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL3VERSIONMAP) .ERRREMOVE: $(SHL3VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL3VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL3VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL3VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL3OBJS)"!=""
    -echo $(foreach,i,$(SHL3OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL3LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL3LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL3VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL3VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL3SONAME=\"$(SONAME_SWITCH)$(SHL3TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL3RES)"!=""
SHL3ALLRES+=$(SHL3RES)
SHL3LINKRES*=$(MISC)/$(SHL3TARGET).res
SHL3LINKRESO*=$(MISC)/$(SHL3TARGET)_res.o
.ENDIF			# "$(SHL3RES)"!=""

.IF "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""
SHL3DEFAULTRES*=$(MISC)/$(SHL3TARGET)_def.res
SHL3ALLRES+=$(SHL3DEFAULTRES)
SHL3LINKRES*=$(MISC)/$(SHL3TARGET).res
SHL3LINKRESO*=$(MISC)/$(SHL3TARGET)_res.o
.ENDIF			# "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL3TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL3LIBS)"!=""
$(MISC)/$(SHL3TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL3LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL3LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL3TARGETN:b)_linkinc.ls
$(SHL3TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL3IMPLIBN)" != ""
$(SHL3IMPLIBN) : $(SHL3TARGETN)
.ENDIF

$(SHL3TARGETN) : \
                    $(SHL3OBJS)\
                    $(SHL3LIBS)\
                    $(USE_3IMPLIB_DEPS)\
                    $(USE_SHL3DEF)\
                    $(USE_SHL3VERSIONMAP)\
                    $(SHL3RES)\
                    $(SHL3DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL3DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL3DEFAULTRES:b).rc
.IF "$(SHL3ICON)" != ""
    @echo 1 ICON $(SHL3ICON) >> $(MISC)/$(SHL3DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL3ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL3DEFAULTRES:b).rc
.ELSE			# "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL3TARGET)$(DLLPOST) >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL3TARGET:b) >> $(MISC)/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL3DEFAULTRES) $(MISC)/$(SHL3DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL3DEFAULTRES:b).rc $(SHL3DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL3DEFAULTRES)"!=""
.IF "$(SHL3ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL3ALLRES) > $(SHL3LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL3LINKRES) $(SHL3LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL3ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL3LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL3STDLIBS) -Wl,--end-group \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL3OBJLIST:=$(STDOBJ) $(SHL3OBJS) $(SHL3LINKRESO) $(shell $(TYPE) /dev/null $(SHL3LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB3NAME)"!=""	# do not have to include objs
    @noop $(assign DEF3OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF3OBJLIST) $(assign ALL3OBJLIST:=$(ALL3OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB3NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL3LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL3STDLIBS) -Wl,--end-group \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL3USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL3LINKER) @$(mktmp \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL3STACK) $(SHL3BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3OBJS) \
        $(SHL3LIBS) \
        $(SHL3STDLIBS) \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL3LINKER) @$(mktmp	$(SHL3LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        $(SHL3STACK) -out:$(SHL3TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL3IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ) \
        $(SHL3LIBS)                         \
        $(SHL3STDLIBS)                      \
        $(SHL3STDSHL) $(STDSHL3)                           \
        $(SHL3LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL3USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL3LINKER) @$(mktmp	$(SHL3LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        $(SHL3STACK) -out:$(SHL3TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_3IMPLIB) \
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ))   \
        $(SHL3LIBS) \
        @$(mktmp $(SHL3STDLIBS)                      \
        $(SHL3STDSHL) $(STDSHL3)                           \
        $(SHL3LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL3USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL3TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL3TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL3BASEX) \
        $(SHL3STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL3IMPLIB).exp \
        $(STDOBJ) \
        $(SHL3OBJS) \
        $(SHL3STDLIBS) \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRES) \
        ) >> $(MISC)/$(SHL3TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL3TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL3TARGET).lnk
        $(COMMAND_ECHO)$(SHL3LINKER) @$(MISC)/$(SHL3TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_3.cmd
    @echo $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) \
    `cat /dev/null $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL3LINKER) $(SHL3LINKFLAGS) $(SHL3VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(SHL3STDSHL) $(STDSHL3) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_3.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_3.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_3.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_3.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_3.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL3RPATH) $@
.IF "$(SHL3CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL3CREATEJNILIB)"!=""
.IF "$(SHL3NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS3) $(SHL3TARGETN)
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB3FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL3OBJS)) $(shell cat /dev/null $(LIB3TARGET) $(SHL3LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_3.cmd
    @echo $(SHL3LINKER) $(SHL3LINKFLAGS) $(SHL3SONAME) $(LINKFLAGSSHL) $(SHL3VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL3LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(SHL3STDSHL) $(STDSHL3) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_3.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_3.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_3.cmd
.IF "$(SHL3NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL3TARGETN) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS3) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS3) -- -s $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL3TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL3TARGETN:f) $(SHL3TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL3TARGETN)"!=""

# unroll begin

.IF "$(SHL4TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL4STDLIBS=
STDSHL=
.ELSE
SHL4ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL4CODETYPE)"=="C"
SHL4LINKER=$(LINKC)
SHL4STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL4LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL4CODETYPE)"=="C"
SHL4LINKER=$(LINK)
SHL4STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL4LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL4CODETYPE)"=="C"

SHL4RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL4RPATH)*=/ERROR:/Bad_SHL4RPATH_value
SHL4LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL4RPATH))

.IF "$(SHL4USE_EXPORTS)"==""
SHL4DEF*=$(MISC)/$(SHL4TARGET).def
.ENDIF			# "$(SHL4USE_EXPORTS)"==""

EXTRALIBPATHS4=$(EXTRALIBPATHS)
.IF "$(SHL4NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS4+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL4NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL4VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL4VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL4VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL4VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL4VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL4VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL4VERSIONOBJ:b).h/ > $@

.INIT : $(SHL4VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL4IMPLIB)" == ""
SHL4IMPLIB=i$(TARGET)_t4
.ENDIF			# "$(SHL4IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_4IMPLIB=-implib:$(LB)/$(SHL4IMPLIB).lib
SHL4IMPLIBN=$(LB)/$(SHL4IMPLIB).lib
.ELSE
USE_4IMPLIB=-Wl,--out-implib=$(SHL4IMPLIBN)
SHL4IMPLIBN=$(LB)/lib$(SHL4IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL4IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_4IMPLIB_DEPS=$(LB)/$(SHL4IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL4DEF=$(SHL4DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL4DEF=
SHL4DEPN+:=$(SHL4DEPNU)

.IF "$(SHL4VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL4VERSIONMAP=$(MISC)/$(SHL4TARGET).vmap
.ENDIF			# "$(SHL4VERSIONMAP)"==""

.IF "$(USE_SHL4VERSIONMAP)"!=""

.IF "$(SHL4FILTERFILE)"!=""
.IF "$(SHL4VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL4VERSIONMAP=$(MISC)/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF

$(USE_SHL4VERSIONMAP): \
                    $(SHL4OBJS)\
                    $(SHL4LIBS)\
                    $(SHL4FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL4OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL4OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL4OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL4OBJS)!"=""
    @$(TYPE) /dev/null $(SHL4LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL4INTERFACE) -f $(SHL4FILTERFILE) -m $@

.ELSE			# "$(SHL4FILTERFILE)"!=""
USE_SHL4VERSIONMAP=$(MISC)/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL4FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL4FILTERFILE)"!=""
.ELSE			# "$(USE_SHL4VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL4VERSIONMAP)"!=""
USE_SHL4VERSIONMAP=$(MISC)/$(SHL4VERSIONMAP:b)_$(SHL4TARGET)$(SHL4VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL4VERSIONMAP): $(SHL4OBJS) $(SHL4LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL4VERSIONMAP) .ERRREMOVE: $(SHL4VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL4VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL4VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL4VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL4OBJS)"!=""
    -echo $(foreach,i,$(SHL4OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL4LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL4LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL4VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL4VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL4SONAME=\"$(SONAME_SWITCH)$(SHL4TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL4RES)"!=""
SHL4ALLRES+=$(SHL4RES)
SHL4LINKRES*=$(MISC)/$(SHL4TARGET).res
SHL4LINKRESO*=$(MISC)/$(SHL4TARGET)_res.o
.ENDIF			# "$(SHL4RES)"!=""

.IF "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""
SHL4DEFAULTRES*=$(MISC)/$(SHL4TARGET)_def.res
SHL4ALLRES+=$(SHL4DEFAULTRES)
SHL4LINKRES*=$(MISC)/$(SHL4TARGET).res
SHL4LINKRESO*=$(MISC)/$(SHL4TARGET)_res.o
.ENDIF			# "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL4TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL4LIBS)"!=""
$(MISC)/$(SHL4TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL4LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL4LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL4TARGETN:b)_linkinc.ls
$(SHL4TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL4IMPLIBN)" != ""
$(SHL4IMPLIBN) : $(SHL4TARGETN)
.ENDIF

$(SHL4TARGETN) : \
                    $(SHL4OBJS)\
                    $(SHL4LIBS)\
                    $(USE_4IMPLIB_DEPS)\
                    $(USE_SHL4DEF)\
                    $(USE_SHL4VERSIONMAP)\
                    $(SHL4RES)\
                    $(SHL4DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL4DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL4DEFAULTRES:b).rc
.IF "$(SHL4ICON)" != ""
    @echo 1 ICON $(SHL4ICON) >> $(MISC)/$(SHL4DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL4ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL4DEFAULTRES:b).rc
.ELSE			# "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL4TARGET)$(DLLPOST) >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL4TARGET:b) >> $(MISC)/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL4DEFAULTRES) $(MISC)/$(SHL4DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL4DEFAULTRES:b).rc $(SHL4DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL4DEFAULTRES)"!=""
.IF "$(SHL4ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL4ALLRES) > $(SHL4LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL4LINKRES) $(SHL4LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL4ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL4LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL4STDLIBS) -Wl,--end-group \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL4OBJLIST:=$(STDOBJ) $(SHL4OBJS) $(SHL4LINKRESO) $(shell $(TYPE) /dev/null $(SHL4LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB4NAME)"!=""	# do not have to include objs
    @noop $(assign DEF4OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF4OBJLIST) $(assign ALL4OBJLIST:=$(ALL4OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB4NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL4LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL4STDLIBS) -Wl,--end-group \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL4USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL4LINKER) @$(mktmp \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL4STACK) $(SHL4BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4OBJS) \
        $(SHL4LIBS) \
        $(SHL4STDLIBS) \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL4LINKER) @$(mktmp	$(SHL4LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        $(SHL4STACK) -out:$(SHL4TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL4IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ) \
        $(SHL4LIBS)                         \
        $(SHL4STDLIBS)                      \
        $(SHL4STDSHL) $(STDSHL4)                           \
        $(SHL4LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL4USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL4LINKER) @$(mktmp	$(SHL4LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        $(SHL4STACK) -out:$(SHL4TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_4IMPLIB) \
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ))   \
        $(SHL4LIBS) \
        @$(mktmp $(SHL4STDLIBS)                      \
        $(SHL4STDSHL) $(STDSHL4)                           \
        $(SHL4LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL4USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL4TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL4TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL4BASEX) \
        $(SHL4STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL4IMPLIB).exp \
        $(STDOBJ) \
        $(SHL4OBJS) \
        $(SHL4STDLIBS) \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRES) \
        ) >> $(MISC)/$(SHL4TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL4TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL4TARGET).lnk
        $(COMMAND_ECHO)$(SHL4LINKER) @$(MISC)/$(SHL4TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_4.cmd
    @echo $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) \
    `cat /dev/null $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL4LINKER) $(SHL4LINKFLAGS) $(SHL4VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(SHL4STDSHL) $(STDSHL4) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_4.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_4.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_4.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_4.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_4.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL4RPATH) $@
.IF "$(SHL4CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL4CREATEJNILIB)"!=""
.IF "$(SHL4NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS4) $(SHL4TARGETN)
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB4FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL4OBJS)) $(shell cat /dev/null $(LIB4TARGET) $(SHL4LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_4.cmd
    @echo $(SHL4LINKER) $(SHL4LINKFLAGS) $(SHL4SONAME) $(LINKFLAGSSHL) $(SHL4VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL4LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(SHL4STDSHL) $(STDSHL4) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_4.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_4.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_4.cmd
.IF "$(SHL4NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL4TARGETN) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS4) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS4) -- -s $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL4TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL4TARGETN:f) $(SHL4TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL4TARGETN)"!=""

# unroll begin

.IF "$(SHL5TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL5STDLIBS=
STDSHL=
.ELSE
SHL5ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL5CODETYPE)"=="C"
SHL5LINKER=$(LINKC)
SHL5STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL5LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL5CODETYPE)"=="C"
SHL5LINKER=$(LINK)
SHL5STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL5LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL5CODETYPE)"=="C"

SHL5RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL5RPATH)*=/ERROR:/Bad_SHL5RPATH_value
SHL5LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL5RPATH))

.IF "$(SHL5USE_EXPORTS)"==""
SHL5DEF*=$(MISC)/$(SHL5TARGET).def
.ENDIF			# "$(SHL5USE_EXPORTS)"==""

EXTRALIBPATHS5=$(EXTRALIBPATHS)
.IF "$(SHL5NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS5+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL5NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL5VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL5VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL5VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL5VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL5VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL5VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL5VERSIONOBJ:b).h/ > $@

.INIT : $(SHL5VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL5IMPLIB)" == ""
SHL5IMPLIB=i$(TARGET)_t5
.ENDIF			# "$(SHL5IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_5IMPLIB=-implib:$(LB)/$(SHL5IMPLIB).lib
SHL5IMPLIBN=$(LB)/$(SHL5IMPLIB).lib
.ELSE
USE_5IMPLIB=-Wl,--out-implib=$(SHL5IMPLIBN)
SHL5IMPLIBN=$(LB)/lib$(SHL5IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL5IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_5IMPLIB_DEPS=$(LB)/$(SHL5IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL5DEF=$(SHL5DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL5DEF=
SHL5DEPN+:=$(SHL5DEPNU)

.IF "$(SHL5VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL5VERSIONMAP=$(MISC)/$(SHL5TARGET).vmap
.ENDIF			# "$(SHL5VERSIONMAP)"==""

.IF "$(USE_SHL5VERSIONMAP)"!=""

.IF "$(SHL5FILTERFILE)"!=""
.IF "$(SHL5VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL5VERSIONMAP=$(MISC)/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF

$(USE_SHL5VERSIONMAP): \
                    $(SHL5OBJS)\
                    $(SHL5LIBS)\
                    $(SHL5FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL5OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL5OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL5OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL5OBJS)!"=""
    @$(TYPE) /dev/null $(SHL5LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL5INTERFACE) -f $(SHL5FILTERFILE) -m $@

.ELSE			# "$(SHL5FILTERFILE)"!=""
USE_SHL5VERSIONMAP=$(MISC)/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL5FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL5FILTERFILE)"!=""
.ELSE			# "$(USE_SHL5VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL5VERSIONMAP)"!=""
USE_SHL5VERSIONMAP=$(MISC)/$(SHL5VERSIONMAP:b)_$(SHL5TARGET)$(SHL5VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL5VERSIONMAP): $(SHL5OBJS) $(SHL5LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL5VERSIONMAP) .ERRREMOVE: $(SHL5VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL5VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL5VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL5VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL5OBJS)"!=""
    -echo $(foreach,i,$(SHL5OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL5LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL5LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL5VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL5VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL5SONAME=\"$(SONAME_SWITCH)$(SHL5TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL5RES)"!=""
SHL5ALLRES+=$(SHL5RES)
SHL5LINKRES*=$(MISC)/$(SHL5TARGET).res
SHL5LINKRESO*=$(MISC)/$(SHL5TARGET)_res.o
.ENDIF			# "$(SHL5RES)"!=""

.IF "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""
SHL5DEFAULTRES*=$(MISC)/$(SHL5TARGET)_def.res
SHL5ALLRES+=$(SHL5DEFAULTRES)
SHL5LINKRES*=$(MISC)/$(SHL5TARGET).res
SHL5LINKRESO*=$(MISC)/$(SHL5TARGET)_res.o
.ENDIF			# "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL5TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL5LIBS)"!=""
$(MISC)/$(SHL5TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL5LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL5LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL5TARGETN:b)_linkinc.ls
$(SHL5TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL5IMPLIBN)" != ""
$(SHL5IMPLIBN) : $(SHL5TARGETN)
.ENDIF

$(SHL5TARGETN) : \
                    $(SHL5OBJS)\
                    $(SHL5LIBS)\
                    $(USE_5IMPLIB_DEPS)\
                    $(USE_SHL5DEF)\
                    $(USE_SHL5VERSIONMAP)\
                    $(SHL5RES)\
                    $(SHL5DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL5DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL5DEFAULTRES:b).rc
.IF "$(SHL5ICON)" != ""
    @echo 1 ICON $(SHL5ICON) >> $(MISC)/$(SHL5DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL5ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL5DEFAULTRES:b).rc
.ELSE			# "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL5TARGET)$(DLLPOST) >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL5TARGET:b) >> $(MISC)/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL5DEFAULTRES) $(MISC)/$(SHL5DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL5DEFAULTRES:b).rc $(SHL5DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL5DEFAULTRES)"!=""
.IF "$(SHL5ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL5ALLRES) > $(SHL5LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL5LINKRES) $(SHL5LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL5ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL5LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL5STDLIBS) -Wl,--end-group \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL5OBJLIST:=$(STDOBJ) $(SHL5OBJS) $(SHL5LINKRESO) $(shell $(TYPE) /dev/null $(SHL5LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB5NAME)"!=""	# do not have to include objs
    @noop $(assign DEF5OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF5OBJLIST) $(assign ALL5OBJLIST:=$(ALL5OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB5NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL5LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL5STDLIBS) -Wl,--end-group \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL5USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL5LINKER) @$(mktmp \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL5STACK) $(SHL5BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5OBJS) \
        $(SHL5LIBS) \
        $(SHL5STDLIBS) \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL5LINKER) @$(mktmp	$(SHL5LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        $(SHL5STACK) -out:$(SHL5TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL5IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ) \
        $(SHL5LIBS)                         \
        $(SHL5STDLIBS)                      \
        $(SHL5STDSHL) $(STDSHL5)                           \
        $(SHL5LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL5USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL5LINKER) @$(mktmp	$(SHL5LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        $(SHL5STACK) -out:$(SHL5TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_5IMPLIB) \
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ))   \
        $(SHL5LIBS) \
        @$(mktmp $(SHL5STDLIBS)                      \
        $(SHL5STDSHL) $(STDSHL5)                           \
        $(SHL5LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL5USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL5TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL5TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL5BASEX) \
        $(SHL5STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL5IMPLIB).exp \
        $(STDOBJ) \
        $(SHL5OBJS) \
        $(SHL5STDLIBS) \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRES) \
        ) >> $(MISC)/$(SHL5TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL5TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL5TARGET).lnk
        $(COMMAND_ECHO)$(SHL5LINKER) @$(MISC)/$(SHL5TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_5.cmd
    @echo $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) \
    `cat /dev/null $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL5LINKER) $(SHL5LINKFLAGS) $(SHL5VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(SHL5STDSHL) $(STDSHL5) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_5.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_5.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_5.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_5.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_5.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL5RPATH) $@
.IF "$(SHL5CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL5CREATEJNILIB)"!=""
.IF "$(SHL5NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS5) $(SHL5TARGETN)
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB5FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL5OBJS)) $(shell cat /dev/null $(LIB5TARGET) $(SHL5LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_5.cmd
    @echo $(SHL5LINKER) $(SHL5LINKFLAGS) $(SHL5SONAME) $(LINKFLAGSSHL) $(SHL5VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL5LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(SHL5STDSHL) $(STDSHL5) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_5.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_5.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_5.cmd
.IF "$(SHL5NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL5TARGETN) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS5) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS5) -- -s $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL5TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL5TARGETN:f) $(SHL5TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL5TARGETN)"!=""

# unroll begin

.IF "$(SHL6TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL6STDLIBS=
STDSHL=
.ELSE
SHL6ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL6CODETYPE)"=="C"
SHL6LINKER=$(LINKC)
SHL6STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL6LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL6CODETYPE)"=="C"
SHL6LINKER=$(LINK)
SHL6STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL6LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL6CODETYPE)"=="C"

SHL6RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL6RPATH)*=/ERROR:/Bad_SHL6RPATH_value
SHL6LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL6RPATH))

.IF "$(SHL6USE_EXPORTS)"==""
SHL6DEF*=$(MISC)/$(SHL6TARGET).def
.ENDIF			# "$(SHL6USE_EXPORTS)"==""

EXTRALIBPATHS6=$(EXTRALIBPATHS)
.IF "$(SHL6NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS6+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL6NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL6VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL6VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL6VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL6VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL6VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL6VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL6VERSIONOBJ:b).h/ > $@

.INIT : $(SHL6VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL6IMPLIB)" == ""
SHL6IMPLIB=i$(TARGET)_t6
.ENDIF			# "$(SHL6IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_6IMPLIB=-implib:$(LB)/$(SHL6IMPLIB).lib
SHL6IMPLIBN=$(LB)/$(SHL6IMPLIB).lib
.ELSE
USE_6IMPLIB=-Wl,--out-implib=$(SHL6IMPLIBN)
SHL6IMPLIBN=$(LB)/lib$(SHL6IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL6IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_6IMPLIB_DEPS=$(LB)/$(SHL6IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL6DEF=$(SHL6DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL6DEF=
SHL6DEPN+:=$(SHL6DEPNU)

.IF "$(SHL6VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL6VERSIONMAP=$(MISC)/$(SHL6TARGET).vmap
.ENDIF			# "$(SHL6VERSIONMAP)"==""

.IF "$(USE_SHL6VERSIONMAP)"!=""

.IF "$(SHL6FILTERFILE)"!=""
.IF "$(SHL6VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL6VERSIONMAP=$(MISC)/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF

$(USE_SHL6VERSIONMAP): \
                    $(SHL6OBJS)\
                    $(SHL6LIBS)\
                    $(SHL6FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL6OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL6OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL6OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL6OBJS)!"=""
    @$(TYPE) /dev/null $(SHL6LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL6INTERFACE) -f $(SHL6FILTERFILE) -m $@

.ELSE			# "$(SHL6FILTERFILE)"!=""
USE_SHL6VERSIONMAP=$(MISC)/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL6FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL6FILTERFILE)"!=""
.ELSE			# "$(USE_SHL6VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL6VERSIONMAP)"!=""
USE_SHL6VERSIONMAP=$(MISC)/$(SHL6VERSIONMAP:b)_$(SHL6TARGET)$(SHL6VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL6VERSIONMAP): $(SHL6OBJS) $(SHL6LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL6VERSIONMAP) .ERRREMOVE: $(SHL6VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL6VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL6VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL6VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL6OBJS)"!=""
    -echo $(foreach,i,$(SHL6OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL6LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL6LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL6VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL6VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL6SONAME=\"$(SONAME_SWITCH)$(SHL6TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL6RES)"!=""
SHL6ALLRES+=$(SHL6RES)
SHL6LINKRES*=$(MISC)/$(SHL6TARGET).res
SHL6LINKRESO*=$(MISC)/$(SHL6TARGET)_res.o
.ENDIF			# "$(SHL6RES)"!=""

.IF "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""
SHL6DEFAULTRES*=$(MISC)/$(SHL6TARGET)_def.res
SHL6ALLRES+=$(SHL6DEFAULTRES)
SHL6LINKRES*=$(MISC)/$(SHL6TARGET).res
SHL6LINKRESO*=$(MISC)/$(SHL6TARGET)_res.o
.ENDIF			# "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL6TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL6LIBS)"!=""
$(MISC)/$(SHL6TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL6LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL6LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL6TARGETN:b)_linkinc.ls
$(SHL6TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL6IMPLIBN)" != ""
$(SHL6IMPLIBN) : $(SHL6TARGETN)
.ENDIF

$(SHL6TARGETN) : \
                    $(SHL6OBJS)\
                    $(SHL6LIBS)\
                    $(USE_6IMPLIB_DEPS)\
                    $(USE_SHL6DEF)\
                    $(USE_SHL6VERSIONMAP)\
                    $(SHL6RES)\
                    $(SHL6DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL6DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL6DEFAULTRES:b).rc
.IF "$(SHL6ICON)" != ""
    @echo 1 ICON $(SHL6ICON) >> $(MISC)/$(SHL6DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL6ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL6DEFAULTRES:b).rc
.ELSE			# "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL6TARGET)$(DLLPOST) >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL6TARGET:b) >> $(MISC)/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL6DEFAULTRES) $(MISC)/$(SHL6DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL6DEFAULTRES:b).rc $(SHL6DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL6DEFAULTRES)"!=""
.IF "$(SHL6ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL6ALLRES) > $(SHL6LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL6LINKRES) $(SHL6LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL6ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL6LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL6STDLIBS) -Wl,--end-group \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL6OBJLIST:=$(STDOBJ) $(SHL6OBJS) $(SHL6LINKRESO) $(shell $(TYPE) /dev/null $(SHL6LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB6NAME)"!=""	# do not have to include objs
    @noop $(assign DEF6OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF6OBJLIST) $(assign ALL6OBJLIST:=$(ALL6OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB6NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL6LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL6STDLIBS) -Wl,--end-group \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL6USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL6LINKER) @$(mktmp \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL6STACK) $(SHL6BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6OBJS) \
        $(SHL6LIBS) \
        $(SHL6STDLIBS) \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL6LINKER) @$(mktmp	$(SHL6LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        $(SHL6STACK) -out:$(SHL6TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL6IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ) \
        $(SHL6LIBS)                         \
        $(SHL6STDLIBS)                      \
        $(SHL6STDSHL) $(STDSHL6)                           \
        $(SHL6LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL6USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL6LINKER) @$(mktmp	$(SHL6LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        $(SHL6STACK) -out:$(SHL6TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_6IMPLIB) \
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ))   \
        $(SHL6LIBS) \
        @$(mktmp $(SHL6STDLIBS)                      \
        $(SHL6STDSHL) $(STDSHL6)                           \
        $(SHL6LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL6USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL6TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL6TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL6BASEX) \
        $(SHL6STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL6IMPLIB).exp \
        $(STDOBJ) \
        $(SHL6OBJS) \
        $(SHL6STDLIBS) \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRES) \
        ) >> $(MISC)/$(SHL6TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL6TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL6TARGET).lnk
        $(COMMAND_ECHO)$(SHL6LINKER) @$(MISC)/$(SHL6TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_6.cmd
    @echo $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) \
    `cat /dev/null $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL6LINKER) $(SHL6LINKFLAGS) $(SHL6VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(SHL6STDSHL) $(STDSHL6) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_6.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_6.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_6.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_6.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_6.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL6RPATH) $@
.IF "$(SHL6CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL6CREATEJNILIB)"!=""
.IF "$(SHL6NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS6) $(SHL6TARGETN)
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB6FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL6OBJS)) $(shell cat /dev/null $(LIB6TARGET) $(SHL6LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_6.cmd
    @echo $(SHL6LINKER) $(SHL6LINKFLAGS) $(SHL6SONAME) $(LINKFLAGSSHL) $(SHL6VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL6LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(SHL6STDSHL) $(STDSHL6) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_6.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_6.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_6.cmd
.IF "$(SHL6NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL6TARGETN) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS6) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS6) -- -s $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL6TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL6TARGETN:f) $(SHL6TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL6TARGETN)"!=""

# unroll begin

.IF "$(SHL7TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL7STDLIBS=
STDSHL=
.ELSE
SHL7ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL7CODETYPE)"=="C"
SHL7LINKER=$(LINKC)
SHL7STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL7LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL7CODETYPE)"=="C"
SHL7LINKER=$(LINK)
SHL7STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL7LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL7CODETYPE)"=="C"

SHL7RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL7RPATH)*=/ERROR:/Bad_SHL7RPATH_value
SHL7LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL7RPATH))

.IF "$(SHL7USE_EXPORTS)"==""
SHL7DEF*=$(MISC)/$(SHL7TARGET).def
.ENDIF			# "$(SHL7USE_EXPORTS)"==""

EXTRALIBPATHS7=$(EXTRALIBPATHS)
.IF "$(SHL7NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS7+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL7NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL7VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL7VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL7VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL7VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL7VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL7VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL7VERSIONOBJ:b).h/ > $@

.INIT : $(SHL7VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL7IMPLIB)" == ""
SHL7IMPLIB=i$(TARGET)_t7
.ENDIF			# "$(SHL7IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_7IMPLIB=-implib:$(LB)/$(SHL7IMPLIB).lib
SHL7IMPLIBN=$(LB)/$(SHL7IMPLIB).lib
.ELSE
USE_7IMPLIB=-Wl,--out-implib=$(SHL7IMPLIBN)
SHL7IMPLIBN=$(LB)/lib$(SHL7IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL7IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_7IMPLIB_DEPS=$(LB)/$(SHL7IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL7DEF=$(SHL7DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL7DEF=
SHL7DEPN+:=$(SHL7DEPNU)

.IF "$(SHL7VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL7VERSIONMAP=$(MISC)/$(SHL7TARGET).vmap
.ENDIF			# "$(SHL7VERSIONMAP)"==""

.IF "$(USE_SHL7VERSIONMAP)"!=""

.IF "$(SHL7FILTERFILE)"!=""
.IF "$(SHL7VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL7VERSIONMAP=$(MISC)/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF

$(USE_SHL7VERSIONMAP): \
                    $(SHL7OBJS)\
                    $(SHL7LIBS)\
                    $(SHL7FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL7OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL7OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL7OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL7OBJS)!"=""
    @$(TYPE) /dev/null $(SHL7LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL7INTERFACE) -f $(SHL7FILTERFILE) -m $@

.ELSE			# "$(SHL7FILTERFILE)"!=""
USE_SHL7VERSIONMAP=$(MISC)/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL7FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL7FILTERFILE)"!=""
.ELSE			# "$(USE_SHL7VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL7VERSIONMAP)"!=""
USE_SHL7VERSIONMAP=$(MISC)/$(SHL7VERSIONMAP:b)_$(SHL7TARGET)$(SHL7VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL7VERSIONMAP): $(SHL7OBJS) $(SHL7LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL7VERSIONMAP) .ERRREMOVE: $(SHL7VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL7VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL7VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL7VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL7OBJS)"!=""
    -echo $(foreach,i,$(SHL7OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL7LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL7LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL7VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL7VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL7SONAME=\"$(SONAME_SWITCH)$(SHL7TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL7RES)"!=""
SHL7ALLRES+=$(SHL7RES)
SHL7LINKRES*=$(MISC)/$(SHL7TARGET).res
SHL7LINKRESO*=$(MISC)/$(SHL7TARGET)_res.o
.ENDIF			# "$(SHL7RES)"!=""

.IF "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""
SHL7DEFAULTRES*=$(MISC)/$(SHL7TARGET)_def.res
SHL7ALLRES+=$(SHL7DEFAULTRES)
SHL7LINKRES*=$(MISC)/$(SHL7TARGET).res
SHL7LINKRESO*=$(MISC)/$(SHL7TARGET)_res.o
.ENDIF			# "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL7TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL7LIBS)"!=""
$(MISC)/$(SHL7TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL7LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL7LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL7TARGETN:b)_linkinc.ls
$(SHL7TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL7IMPLIBN)" != ""
$(SHL7IMPLIBN) : $(SHL7TARGETN)
.ENDIF

$(SHL7TARGETN) : \
                    $(SHL7OBJS)\
                    $(SHL7LIBS)\
                    $(USE_7IMPLIB_DEPS)\
                    $(USE_SHL7DEF)\
                    $(USE_SHL7VERSIONMAP)\
                    $(SHL7RES)\
                    $(SHL7DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL7DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL7DEFAULTRES:b).rc
.IF "$(SHL7ICON)" != ""
    @echo 1 ICON $(SHL7ICON) >> $(MISC)/$(SHL7DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL7ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL7DEFAULTRES:b).rc
.ELSE			# "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL7TARGET)$(DLLPOST) >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL7TARGET:b) >> $(MISC)/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL7DEFAULTRES) $(MISC)/$(SHL7DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL7DEFAULTRES:b).rc $(SHL7DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL7DEFAULTRES)"!=""
.IF "$(SHL7ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL7ALLRES) > $(SHL7LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL7LINKRES) $(SHL7LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL7ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL7LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL7STDLIBS) -Wl,--end-group \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL7OBJLIST:=$(STDOBJ) $(SHL7OBJS) $(SHL7LINKRESO) $(shell $(TYPE) /dev/null $(SHL7LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB7NAME)"!=""	# do not have to include objs
    @noop $(assign DEF7OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF7OBJLIST) $(assign ALL7OBJLIST:=$(ALL7OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB7NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL7LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL7STDLIBS) -Wl,--end-group \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL7USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL7LINKER) @$(mktmp \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL7STACK) $(SHL7BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7OBJS) \
        $(SHL7LIBS) \
        $(SHL7STDLIBS) \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL7LINKER) @$(mktmp	$(SHL7LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        $(SHL7STACK) -out:$(SHL7TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL7IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ) \
        $(SHL7LIBS)                         \
        $(SHL7STDLIBS)                      \
        $(SHL7STDSHL) $(STDSHL7)                           \
        $(SHL7LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL7USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL7LINKER) @$(mktmp	$(SHL7LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        $(SHL7STACK) -out:$(SHL7TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_7IMPLIB) \
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ))   \
        $(SHL7LIBS) \
        @$(mktmp $(SHL7STDLIBS)                      \
        $(SHL7STDSHL) $(STDSHL7)                           \
        $(SHL7LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL7USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL7TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL7TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL7BASEX) \
        $(SHL7STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL7IMPLIB).exp \
        $(STDOBJ) \
        $(SHL7OBJS) \
        $(SHL7STDLIBS) \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRES) \
        ) >> $(MISC)/$(SHL7TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL7TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL7TARGET).lnk
        $(COMMAND_ECHO)$(SHL7LINKER) @$(MISC)/$(SHL7TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_7.cmd
    @echo $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) \
    `cat /dev/null $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL7LINKER) $(SHL7LINKFLAGS) $(SHL7VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(SHL7STDSHL) $(STDSHL7) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_7.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_7.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_7.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_7.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_7.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL7RPATH) $@
.IF "$(SHL7CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL7CREATEJNILIB)"!=""
.IF "$(SHL7NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS7) $(SHL7TARGETN)
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB7FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL7OBJS)) $(shell cat /dev/null $(LIB7TARGET) $(SHL7LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_7.cmd
    @echo $(SHL7LINKER) $(SHL7LINKFLAGS) $(SHL7SONAME) $(LINKFLAGSSHL) $(SHL7VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL7LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(SHL7STDSHL) $(STDSHL7) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_7.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_7.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_7.cmd
.IF "$(SHL7NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL7TARGETN) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS7) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS7) -- -s $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL7TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL7TARGETN:f) $(SHL7TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL7TARGETN)"!=""

# unroll begin

.IF "$(SHL8TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL8STDLIBS=
STDSHL=
.ELSE
SHL8ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL8CODETYPE)"=="C"
SHL8LINKER=$(LINKC)
SHL8STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL8LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL8CODETYPE)"=="C"
SHL8LINKER=$(LINK)
SHL8STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL8LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL8CODETYPE)"=="C"

SHL8RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL8RPATH)*=/ERROR:/Bad_SHL8RPATH_value
SHL8LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL8RPATH))

.IF "$(SHL8USE_EXPORTS)"==""
SHL8DEF*=$(MISC)/$(SHL8TARGET).def
.ENDIF			# "$(SHL8USE_EXPORTS)"==""

EXTRALIBPATHS8=$(EXTRALIBPATHS)
.IF "$(SHL8NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS8+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL8NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL8VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL8VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL8VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL8VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL8VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL8VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL8VERSIONOBJ:b).h/ > $@

.INIT : $(SHL8VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL8IMPLIB)" == ""
SHL8IMPLIB=i$(TARGET)_t8
.ENDIF			# "$(SHL8IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_8IMPLIB=-implib:$(LB)/$(SHL8IMPLIB).lib
SHL8IMPLIBN=$(LB)/$(SHL8IMPLIB).lib
.ELSE
USE_8IMPLIB=-Wl,--out-implib=$(SHL8IMPLIBN)
SHL8IMPLIBN=$(LB)/lib$(SHL8IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL8IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_8IMPLIB_DEPS=$(LB)/$(SHL8IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL8DEF=$(SHL8DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL8DEF=
SHL8DEPN+:=$(SHL8DEPNU)

.IF "$(SHL8VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL8VERSIONMAP=$(MISC)/$(SHL8TARGET).vmap
.ENDIF			# "$(SHL8VERSIONMAP)"==""

.IF "$(USE_SHL8VERSIONMAP)"!=""

.IF "$(SHL8FILTERFILE)"!=""
.IF "$(SHL8VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL8VERSIONMAP=$(MISC)/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF

$(USE_SHL8VERSIONMAP): \
                    $(SHL8OBJS)\
                    $(SHL8LIBS)\
                    $(SHL8FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL8OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL8OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL8OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL8OBJS)!"=""
    @$(TYPE) /dev/null $(SHL8LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL8INTERFACE) -f $(SHL8FILTERFILE) -m $@

.ELSE			# "$(SHL8FILTERFILE)"!=""
USE_SHL8VERSIONMAP=$(MISC)/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL8FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL8FILTERFILE)"!=""
.ELSE			# "$(USE_SHL8VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL8VERSIONMAP)"!=""
USE_SHL8VERSIONMAP=$(MISC)/$(SHL8VERSIONMAP:b)_$(SHL8TARGET)$(SHL8VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL8VERSIONMAP): $(SHL8OBJS) $(SHL8LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL8VERSIONMAP) .ERRREMOVE: $(SHL8VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL8VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL8VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL8VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL8OBJS)"!=""
    -echo $(foreach,i,$(SHL8OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL8LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL8LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL8VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL8VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL8SONAME=\"$(SONAME_SWITCH)$(SHL8TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL8RES)"!=""
SHL8ALLRES+=$(SHL8RES)
SHL8LINKRES*=$(MISC)/$(SHL8TARGET).res
SHL8LINKRESO*=$(MISC)/$(SHL8TARGET)_res.o
.ENDIF			# "$(SHL8RES)"!=""

.IF "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""
SHL8DEFAULTRES*=$(MISC)/$(SHL8TARGET)_def.res
SHL8ALLRES+=$(SHL8DEFAULTRES)
SHL8LINKRES*=$(MISC)/$(SHL8TARGET).res
SHL8LINKRESO*=$(MISC)/$(SHL8TARGET)_res.o
.ENDIF			# "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL8TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL8LIBS)"!=""
$(MISC)/$(SHL8TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL8LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL8LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL8TARGETN:b)_linkinc.ls
$(SHL8TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL8IMPLIBN)" != ""
$(SHL8IMPLIBN) : $(SHL8TARGETN)
.ENDIF

$(SHL8TARGETN) : \
                    $(SHL8OBJS)\
                    $(SHL8LIBS)\
                    $(USE_8IMPLIB_DEPS)\
                    $(USE_SHL8DEF)\
                    $(USE_SHL8VERSIONMAP)\
                    $(SHL8RES)\
                    $(SHL8DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL8DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL8DEFAULTRES:b).rc
.IF "$(SHL8ICON)" != ""
    @echo 1 ICON $(SHL8ICON) >> $(MISC)/$(SHL8DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL8ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL8DEFAULTRES:b).rc
.ELSE			# "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL8TARGET)$(DLLPOST) >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL8TARGET:b) >> $(MISC)/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL8DEFAULTRES) $(MISC)/$(SHL8DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL8DEFAULTRES:b).rc $(SHL8DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL8DEFAULTRES)"!=""
.IF "$(SHL8ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL8ALLRES) > $(SHL8LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL8LINKRES) $(SHL8LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL8ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL8LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL8STDLIBS) -Wl,--end-group \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL8OBJLIST:=$(STDOBJ) $(SHL8OBJS) $(SHL8LINKRESO) $(shell $(TYPE) /dev/null $(SHL8LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB8NAME)"!=""	# do not have to include objs
    @noop $(assign DEF8OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF8OBJLIST) $(assign ALL8OBJLIST:=$(ALL8OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB8NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL8LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL8STDLIBS) -Wl,--end-group \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL8USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL8LINKER) @$(mktmp \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL8STACK) $(SHL8BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8OBJS) \
        $(SHL8LIBS) \
        $(SHL8STDLIBS) \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL8LINKER) @$(mktmp	$(SHL8LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        $(SHL8STACK) -out:$(SHL8TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL8IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ) \
        $(SHL8LIBS)                         \
        $(SHL8STDLIBS)                      \
        $(SHL8STDSHL) $(STDSHL8)                           \
        $(SHL8LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL8USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL8LINKER) @$(mktmp	$(SHL8LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        $(SHL8STACK) -out:$(SHL8TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_8IMPLIB) \
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ))   \
        $(SHL8LIBS) \
        @$(mktmp $(SHL8STDLIBS)                      \
        $(SHL8STDSHL) $(STDSHL8)                           \
        $(SHL8LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL8USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL8TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL8TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL8BASEX) \
        $(SHL8STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL8IMPLIB).exp \
        $(STDOBJ) \
        $(SHL8OBJS) \
        $(SHL8STDLIBS) \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRES) \
        ) >> $(MISC)/$(SHL8TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL8TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL8TARGET).lnk
        $(COMMAND_ECHO)$(SHL8LINKER) @$(MISC)/$(SHL8TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_8.cmd
    @echo $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) \
    `cat /dev/null $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL8LINKER) $(SHL8LINKFLAGS) $(SHL8VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(SHL8STDSHL) $(STDSHL8) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_8.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_8.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_8.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_8.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_8.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL8RPATH) $@
.IF "$(SHL8CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL8CREATEJNILIB)"!=""
.IF "$(SHL8NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS8) $(SHL8TARGETN)
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB8FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL8OBJS)) $(shell cat /dev/null $(LIB8TARGET) $(SHL8LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_8.cmd
    @echo $(SHL8LINKER) $(SHL8LINKFLAGS) $(SHL8SONAME) $(LINKFLAGSSHL) $(SHL8VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL8LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(SHL8STDSHL) $(STDSHL8) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_8.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_8.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_8.cmd
.IF "$(SHL8NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL8TARGETN) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS8) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS8) -- -s $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL8TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL8TARGETN:f) $(SHL8TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL8TARGETN)"!=""

# unroll begin

.IF "$(SHL9TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL9STDLIBS=
STDSHL=
.ELSE
SHL9ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL9CODETYPE)"=="C"
SHL9LINKER=$(LINKC)
SHL9STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL9LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL9CODETYPE)"=="C"
SHL9LINKER=$(LINK)
SHL9STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL9LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL9CODETYPE)"=="C"

SHL9RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL9RPATH)*=/ERROR:/Bad_SHL9RPATH_value
SHL9LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL9RPATH))

.IF "$(SHL9USE_EXPORTS)"==""
SHL9DEF*=$(MISC)/$(SHL9TARGET).def
.ENDIF			# "$(SHL9USE_EXPORTS)"==""

EXTRALIBPATHS9=$(EXTRALIBPATHS)
.IF "$(SHL9NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS9+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL9NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL9VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL9VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL9VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL9VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL9VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL9VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL9VERSIONOBJ:b).h/ > $@

.INIT : $(SHL9VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL9IMPLIB)" == ""
SHL9IMPLIB=i$(TARGET)_t9
.ENDIF			# "$(SHL9IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_9IMPLIB=-implib:$(LB)/$(SHL9IMPLIB).lib
SHL9IMPLIBN=$(LB)/$(SHL9IMPLIB).lib
.ELSE
USE_9IMPLIB=-Wl,--out-implib=$(SHL9IMPLIBN)
SHL9IMPLIBN=$(LB)/lib$(SHL9IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL9IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_9IMPLIB_DEPS=$(LB)/$(SHL9IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL9DEF=$(SHL9DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL9DEF=
SHL9DEPN+:=$(SHL9DEPNU)

.IF "$(SHL9VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL9VERSIONMAP=$(MISC)/$(SHL9TARGET).vmap
.ENDIF			# "$(SHL9VERSIONMAP)"==""

.IF "$(USE_SHL9VERSIONMAP)"!=""

.IF "$(SHL9FILTERFILE)"!=""
.IF "$(SHL9VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL9VERSIONMAP=$(MISC)/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF

$(USE_SHL9VERSIONMAP): \
                    $(SHL9OBJS)\
                    $(SHL9LIBS)\
                    $(SHL9FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL9OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL9OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL9OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL9OBJS)!"=""
    @$(TYPE) /dev/null $(SHL9LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL9INTERFACE) -f $(SHL9FILTERFILE) -m $@

.ELSE			# "$(SHL9FILTERFILE)"!=""
USE_SHL9VERSIONMAP=$(MISC)/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL9FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL9FILTERFILE)"!=""
.ELSE			# "$(USE_SHL9VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL9VERSIONMAP)"!=""
USE_SHL9VERSIONMAP=$(MISC)/$(SHL9VERSIONMAP:b)_$(SHL9TARGET)$(SHL9VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL9VERSIONMAP): $(SHL9OBJS) $(SHL9LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL9VERSIONMAP) .ERRREMOVE: $(SHL9VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL9VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL9VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL9VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL9OBJS)"!=""
    -echo $(foreach,i,$(SHL9OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL9LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL9LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL9VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL9VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL9SONAME=\"$(SONAME_SWITCH)$(SHL9TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL9RES)"!=""
SHL9ALLRES+=$(SHL9RES)
SHL9LINKRES*=$(MISC)/$(SHL9TARGET).res
SHL9LINKRESO*=$(MISC)/$(SHL9TARGET)_res.o
.ENDIF			# "$(SHL9RES)"!=""

.IF "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""
SHL9DEFAULTRES*=$(MISC)/$(SHL9TARGET)_def.res
SHL9ALLRES+=$(SHL9DEFAULTRES)
SHL9LINKRES*=$(MISC)/$(SHL9TARGET).res
SHL9LINKRESO*=$(MISC)/$(SHL9TARGET)_res.o
.ENDIF			# "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL9TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL9LIBS)"!=""
$(MISC)/$(SHL9TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL9LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL9LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL9TARGETN:b)_linkinc.ls
$(SHL9TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL9IMPLIBN)" != ""
$(SHL9IMPLIBN) : $(SHL9TARGETN)
.ENDIF

$(SHL9TARGETN) : \
                    $(SHL9OBJS)\
                    $(SHL9LIBS)\
                    $(USE_9IMPLIB_DEPS)\
                    $(USE_SHL9DEF)\
                    $(USE_SHL9VERSIONMAP)\
                    $(SHL9RES)\
                    $(SHL9DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL9DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL9DEFAULTRES:b).rc
.IF "$(SHL9ICON)" != ""
    @echo 1 ICON $(SHL9ICON) >> $(MISC)/$(SHL9DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL9ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL9DEFAULTRES:b).rc
.ELSE			# "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL9TARGET)$(DLLPOST) >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL9TARGET:b) >> $(MISC)/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL9DEFAULTRES) $(MISC)/$(SHL9DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL9DEFAULTRES:b).rc $(SHL9DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL9DEFAULTRES)"!=""
.IF "$(SHL9ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL9ALLRES) > $(SHL9LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL9LINKRES) $(SHL9LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL9ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL9LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL9STDLIBS) -Wl,--end-group \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL9OBJLIST:=$(STDOBJ) $(SHL9OBJS) $(SHL9LINKRESO) $(shell $(TYPE) /dev/null $(SHL9LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB9NAME)"!=""	# do not have to include objs
    @noop $(assign DEF9OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF9OBJLIST) $(assign ALL9OBJLIST:=$(ALL9OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB9NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL9LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL9STDLIBS) -Wl,--end-group \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL9USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL9LINKER) @$(mktmp \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL9STACK) $(SHL9BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9OBJS) \
        $(SHL9LIBS) \
        $(SHL9STDLIBS) \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL9LINKER) @$(mktmp	$(SHL9LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        $(SHL9STACK) -out:$(SHL9TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL9IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ) \
        $(SHL9LIBS)                         \
        $(SHL9STDLIBS)                      \
        $(SHL9STDSHL) $(STDSHL9)                           \
        $(SHL9LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL9USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL9LINKER) @$(mktmp	$(SHL9LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        $(SHL9STACK) -out:$(SHL9TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_9IMPLIB) \
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ))   \
        $(SHL9LIBS) \
        @$(mktmp $(SHL9STDLIBS)                      \
        $(SHL9STDSHL) $(STDSHL9)                           \
        $(SHL9LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL9USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL9TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL9TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL9BASEX) \
        $(SHL9STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL9IMPLIB).exp \
        $(STDOBJ) \
        $(SHL9OBJS) \
        $(SHL9STDLIBS) \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRES) \
        ) >> $(MISC)/$(SHL9TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL9TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL9TARGET).lnk
        $(COMMAND_ECHO)$(SHL9LINKER) @$(MISC)/$(SHL9TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_9.cmd
    @echo $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) \
    `cat /dev/null $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL9LINKER) $(SHL9LINKFLAGS) $(SHL9VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(SHL9STDSHL) $(STDSHL9) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_9.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_9.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_9.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_9.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_9.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL9RPATH) $@
.IF "$(SHL9CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL9CREATEJNILIB)"!=""
.IF "$(SHL9NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS9) $(SHL9TARGETN)
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB9FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL9OBJS)) $(shell cat /dev/null $(LIB9TARGET) $(SHL9LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_9.cmd
    @echo $(SHL9LINKER) $(SHL9LINKFLAGS) $(SHL9SONAME) $(LINKFLAGSSHL) $(SHL9VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL9LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(SHL9STDSHL) $(STDSHL9) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_9.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_9.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_9.cmd
.IF "$(SHL9NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL9TARGETN) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS9) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS9) -- -s $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL9TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL9TARGETN:f) $(SHL9TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL9TARGETN)"!=""

# unroll begin

.IF "$(SHL10TARGETN)"!=""

.IF "$(SHLLINKARCONLY)" != ""
SHL10STDLIBS=
STDSHL=
.ELSE
SHL10ARCHIVES=
.ENDIF

# decide how to link
.IF "$(SHL10CODETYPE)"=="C"
SHL10LINKER=$(LINKC)
SHL10STDSHL=$(subst,CPPRUNTIME, $(STDSHL))
SHL10LINKFLAGS+=$(LINKCFLAGS)
.ELSE			# "$(SHL10CODETYPE)"=="C"
SHL10LINKER=$(LINK)
SHL10STDSHL=$(subst,CPPRUNTIME,$(STDLIBCPP) $(STDSHL))
SHL10LINKFLAGS+=$(LINKFLAGS)
.ENDIF			# "$(SHL10CODETYPE)"=="C"

SHL10RPATH*=OOO
LINKFLAGSRUNPATH_$(SHL10RPATH)*=/ERROR:/Bad_SHL10RPATH_value
SHL10LINKFLAGS+=$(LINKFLAGSRUNPATH_$(SHL10RPATH))

.IF "$(SHL10USE_EXPORTS)"==""
SHL10DEF*=$(MISC)/$(SHL10TARGET).def
.ENDIF			# "$(SHL10USE_EXPORTS)"==""

EXTRALIBPATHS10=$(EXTRALIBPATHS)
.IF "$(SHL10NOCHECK)"==""
.IF "$(SOLAR_STLLIBPATH)"!=""
EXTRALIBPATHS10+=-L$(SOLAR_STLLIBPATH)
.ENDIF
.ENDIF				# "$(SHL10NOCHECK)"!=""
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(L10N_framework)"==""
.IF "$(VERSIONOBJ)"!=""
SHL10VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)/$(SHL10VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL10VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL10VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)/$(SHL10VERSIONOBJ:b).c : $(SOLARENV)/src/version.c $(INCCOM)/$(SHL10VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)/src/version.c $@
    $(COMMAND_ECHO)$(TYPE) $(SOLARENV)/src/version.c | $(SED) s/_version.h/$(SHL10VERSIONOBJ:b).h/ > $@

.INIT : $(SHL10VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""
.ENDIF

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL10IMPLIB)" == ""
SHL10IMPLIB=i$(TARGET)_t10
.ENDIF			# "$(SHL10IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_10IMPLIB=-implib:$(LB)/$(SHL10IMPLIB).lib
SHL10IMPLIBN=$(LB)/$(SHL10IMPLIB).lib
.ELSE
USE_10IMPLIB=-Wl,--out-implib=$(SHL10IMPLIBN)
SHL10IMPLIBN=$(LB)/lib$(SHL10IMPLIB).dll.a
.ENDIF			# "$(COM)" != "GCC"
ALLTAR : $(SHL10IMPLIBN)

.IF "$(USE_DEFFILE)"==""
.IF "$(COM)" != "GCC"
USE_10IMPLIB_DEPS=$(LB)/$(SHL10IMPLIB).lib
.ENDIF
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL10DEF=$(SHL10DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL10DEF=
SHL10DEPN+:=$(SHL10DEPNU)

.IF "$(SHL10VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL10VERSIONMAP=$(MISC)/$(SHL10TARGET).vmap
.ENDIF			# "$(SHL10VERSIONMAP)"==""

.IF "$(USE_SHL10VERSIONMAP)"!=""

.IF "$(SHL10FILTERFILE)"!=""
.IF "$(SHL10VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL10VERSIONMAP=$(MISC)/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF

$(USE_SHL10VERSIONMAP): \
                    $(SHL10OBJS)\
                    $(SHL10LIBS)\
                    $(SHL10FILTERFILE)
    @$(RM) $@.dump
.IF "$(SHL10OBJS)"!=""
# dump remaining objects on the fly
.IF "$(OS)"=="MACOSX"
    @-nm $(SHL10OBJS:s/.obj/.o/) > $@.dump
.ELSE
    @nm $(SHL10OBJS:s/.obj/.o/) > $@.dump
.ENDIF
.ENDIF			# "$(SHL10OBJS)!"=""
    @$(TYPE) /dev/null $(SHL10LIBS:s/.lib/.dump/) >> $@.dump
    $(COMMAND_ECHO) $(PERL) $(SOLARENV)/bin/mapgen.pl -d $@.dump -s $(SHL10INTERFACE) -f $(SHL10FILTERFILE) -m $@

.ELSE			# "$(SHL10FILTERFILE)"!=""
USE_SHL10VERSIONMAP=$(MISC)/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) :
.IF "$(VERBOSE)" == "TRUE"
    @echo -----------------------------
    @echo SHL10FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
.ELSE
    @$(TOUCH) $@
.ENDIF
#	force_dmake_to_error
.ENDIF			# "$(SHL10FILTERFILE)"!=""
.ELSE			# "$(USE_SHL10VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL10VERSIONMAP)"!=""
USE_SHL10VERSIONMAP=$(MISC)/$(SHL10VERSIONMAP:b)_$(SHL10TARGET)$(SHL10VERSIONMAP:e)
.IF "$(OS)" != "AIX" && "$(OS)" != "ANDROID"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL10VERSIONMAP): $(SHL10OBJS) $(SHL10LIBS)
.ENDIF

# .ERRREMOVE is needed as a recipe line like "$(AWK) ... > $@" would create $@
# even if the $(AWK) part fails:
$(USE_SHL10VERSIONMAP) .ERRREMOVE: $(SHL10VERSIONMAP)
    @@-$(RM) -f $@
# The following files will only be generated and needed on Mac OS X as temporary files
# in order to generate exported symbols list out of Linux/Solaris map files
.IF "$(OS)"=="MACOSX"
    @-$(RM) -f $@.symregexp >& $(NULLDEV)
    @-$(RM) -f $@.expsymlist >& $(NULLDEV)
.ENDIF
# Its questionable if the following condition '.IF "$(COMID)"=="gcc3"' makes sense and what
# happens if somebody will change it in the future
.IF "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL10VERSIONMAP) | $(AWK) -f $(SOLARENV)/bin/addsym.awk > $@
.ELIF "$(COMNAME)"=="sunpro5"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL10VERSIONMAP) | $(GREP) -v $(IGNORE_SYMBOLS) > $@
.ELSE           # "$(COMID)"=="gcc3"
    $(COMMAND_ECHO) tr -d "\015" < $(SHL10VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    @-cat $@ | $(AWK) -f $(SOLARENV)/bin/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL10OBJS)"!=""
    -echo $(foreach,i,$(SHL10OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL10LIBS)"!=""
    $(COMMAND_ECHO)-$(TYPE) $(foreach,j,$(SHL10LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)/bin/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL10VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL10VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="AIX"
.IF "$(GUI)"=="UNX"
SHL10SONAME=\"$(SONAME_SWITCH)$(SHL10TARGETN:f)\"
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL10RES)"!=""
SHL10ALLRES+=$(SHL10RES)
SHL10LINKRES*=$(MISC)/$(SHL10TARGET).res
SHL10LINKRESO*=$(MISC)/$(SHL10TARGET)_res.o
.ENDIF			# "$(SHL10RES)"!=""

.IF "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""
SHL10DEFAULTRES*=$(MISC)/$(SHL10TARGET)_def.res
SHL10ALLRES+=$(SHL10DEFAULTRES)
SHL10LINKRES*=$(MISC)/$(SHL10TARGET).res
SHL10LINKRESO*=$(MISC)/$(SHL10TARGET)_res.o
.ENDIF			# "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL10TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
.IF "$(SHL10LIBS)"!=""
$(MISC)/$(SHL10TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(COMMAND_ECHO)$(SED) -f $(SOLARENV)/bin/chrel.sed $(foreach,i,$(SHL10LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF          # "$(SHL10LIBS)"!=""
.ENDIF

LINKINCTARGETS+=$(MISC)/$(SHL10TARGETN:b)_linkinc.ls
$(SHL10TARGETN) : $(LINKINCTARGETS)

.ELSE

$(MISC)/%linkinc.ls:
    @echo . > $@
.ENDIF          # "$(linkinc)"!=""

.IF "$(COM)" == "GCC" && "$(SHL10IMPLIBN)" != ""
$(SHL10IMPLIBN) : $(SHL10TARGETN)
.ENDIF

$(SHL10TARGETN) : \
                    $(SHL10OBJS)\
                    $(SHL10LIBS)\
                    $(USE_10IMPLIB_DEPS)\
                    $(USE_SHL10DEF)\
                    $(USE_SHL10VERSIONMAP)\
                    $(SHL10RES)\
                    $(SHL10DEPN)
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
.IF "$(SHL10DEFAULTRES)"!=""
    @@-$(RM) $(MISC)/$(SHL10DEFAULTRES:b).rc
.IF "$(SHL10ICON)" != ""
    @echo 1 ICON $(SHL10ICON) >> $(MISC)/$(SHL10DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL10ADD_VERINFO)$(EMQ)" >> $(MISC)/$(SHL10DEFAULTRES:b).rc
.ELSE			# "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL10TARGET)$(DLLPOST) >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define RES_APP_VENDOR $(OOO_VENDOR) >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL10TARGET:b) >> $(MISC)/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
.IF "$(RCFLAGSOUTRES)"!=""
# rc, takes separate flag naming output file, source .rc file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(RCFLAGSOUTRES)$(SHL10DEFAULTRES) $(MISC)/$(SHL10DEFAULTRES:b).rc
.ELSE
# windres, just takes output file last
    $(COMMAND_ECHO)$(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)/$(SHL10DEFAULTRES:b).rc $(SHL10DEFAULTRES)
.ENDIF
.ENDIF			# "$(SHL10DEFAULTRES)"!=""
.IF "$(SHL10ALLRES)"!=""
    $(COMMAND_ECHO)$(TYPE) $(SHL10ALLRES) > $(SHL10LINKRES)
.IF "$(COM)"=="GCC"
    $(WINDRES) $(SHL10LINKRES) $(SHL10LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(SHL10ALLRES)"!=""
.IF "$(COM)"=="GCC"
# GNU ld since 2.17 supports @cmdfile syntax
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL10LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL10STDLIBS) -Wl,--end-group \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRESO) \
    ))
.ELSE
    @noop $(assign ALL10OBJLIST:=$(STDOBJ) $(SHL10OBJS) $(SHL10LINKRESO) $(shell $(TYPE) /dev/null $(SHL10LIBS) | $(SED) s?$(ROUT)?$(PRJ)/$(ROUT)?g))
.IF "$(DEFLIB10NAME)"!=""	# do not have to include objs
    @noop $(assign DEF10OBJLIST:=$(shell $(TYPE) $(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib) | sed s?$(ROUT)?$(PRJ)/$(ROUT)?g))
    @noop $(foreach,i,$(DEF10OBJLIST) $(assign ALL10OBJLIST:=$(ALL10OBJLIST:s?$i??)))
.ENDIF			# "$(DEFLIB10NAME)"!=""
    $(COMMAND_ECHO)$(LINK) @$(mktmp $(strip \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) \
	$(SOLARLIB) \
        $(MINGWSSTDOBJ) \
        -o $@ \
        -Wl,-Map,$(MISC)/$(@:b).map \
        $(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10OBJS) \
        $(subst,$(ROUT),$(PRJ)/$(ROUT) $(shell cat /dev/null $(SHL10LIBS))) \
        -Wl,--exclude-libs,ALL,--start-group $(SHL10STDLIBS) -Wl,--end-group \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRESO) \
    ))
.ENDIF
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL10USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL10LINKER) @$(mktmp \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL10STACK) $(SHL10BASEX)	\
        -out:$@ \
        -map:$(MISC)/$(@:b).map \
        -def:$(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10OBJS) \
        $(SHL10LIBS) \
        $(SHL10STDLIBS) \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(COMMAND_ECHO)$(SHL10LINKER) @$(mktmp	$(SHL10LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        $(SHL10STACK) -out:$(SHL10TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(LB)/$(SHL10IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ) \
        $(SHL10LIBS)                         \
        $(SHL10STDLIBS)                      \
        $(SHL10STDSHL) $(STDSHL10)                           \
        $(SHL10LINKRES) \
    )
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL10USE_EXPORTS)"!="name"
    $(COMMAND_ECHO)$(SHL10LINKER) @$(mktmp	$(SHL10LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        $(SHL10STACK) -out:$(SHL10TARGETN)	\
        -map:$(MISC)/$(@:B).map				\
        $(USE_10IMPLIB) \
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ))   \
        $(SHL10LIBS) \
        @$(mktmp $(SHL10STDLIBS)                      \
        $(SHL10STDSHL) $(STDSHL10)                           \
        $(SHL10LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL10USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        $(COMMAND_ECHO)-$(RM) del $(MISC)/$(SHL10TARGET).lnk
        $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL10TARGET).lst
        $(COMMAND_ECHO)$(TYPE) $(mktmp \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL10BASEX) \
        $(SHL10STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)/$(SHL10IMPLIB).exp \
        $(STDOBJ) \
        $(SHL10OBJS) \
        $(SHL10STDLIBS) \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRES) \
        ) >> $(MISC)/$(SHL10TARGET).lnk
        $(COMMAND_ECHO)$(TYPE) $(MISC)/$(SHL10TARGETN:b)_linkinc.ls  >> $(MISC)/$(SHL10TARGET).lnk
        $(COMMAND_ECHO)$(SHL10LINKER) @$(MISC)/$(SHL10TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(MT) $(MTFLAGS) -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(COMMAND_ECHO)$(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(COMMAND_ECHO)$(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(@:b).list
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_10.cmd
    @echo $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) \
    `cat /dev/null $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)/$(@:b).list
    @/bin/echo -n $(SHL10LINKER) $(SHL10LINKFLAGS) $(SHL10VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) -o $@ \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(SHL10STDSHL) $(STDSHL10) -filelist $(MISC)/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_10.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
        `cat $(MISC)/$(TARGET).$(@:b)_10.cmd` \
        >> $(MISC)/$(TARGET).$(@:b)_10.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_10.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_10.cmd
    @$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl \
        shl $(SHL10RPATH) $@
.IF "$(SHL10CREATEJNILIB)"!=""
    @echo "Making:   " $(@:f).jnilib
    @macosx-create-bundle $@
.ENDIF          # "$(SHL10CREATEJNILIB)"!=""
.IF "$(SHL10NOCHECK)"==""
    $(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS10) $(SHL10TARGETN)
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ELIF "$(DISABLE_DYNLOADING)"=="TRUE"
    $(COMMAND_ECHO)$(AR) $(LIB10FLAGS) $(LIBFLAGS) $@ $(subst,.obj,.o $(SHL10OBJS)) $(shell cat /dev/null $(LIB10TARGET) $(SHL10LIBS) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g)
    $(COMMAND_ECHO)$(RANLIB) $@
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)/$(TARGET).$(@:b)_10.cmd
    @echo $(SHL10LINKER) $(SHL10LINKFLAGS) $(SHL10SONAME) $(LINKFLAGSSHL) $(SHL10VERSIONMAPPARA) -L$(PRJ)/$(ROUT)/lib $(SOLARLIB) $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL10LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)/$(ROUT)\#g` \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(SHL10STDSHL) $(STDSHL10) $(LINKOUTPUT_FILTER) > $(MISC)/$(TARGET).$(@:b)_10.cmd
  .IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(TARGET).$(@:b)_10.cmd
  .ENDIF
    @+source $(MISC)/$(TARGET).$(@:b)_10.cmd
.IF "$(SHL10NOCHECK)"==""
    $(COMMAND_ECHO)-$(RM) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
    $(COMMAND_ECHO)$(RENAME) $(SHL10TARGETN) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
.IF "$(VERBOSE)"=="TRUE"
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS10) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
.ELSE
    $(COMMAND_ECHO)$(SOLARENV)/bin/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS10) -- -s $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
.ENDIF
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!="" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID"
    $(COMMAND_ECHO)$(RM) $(LB)/$(SHL10TARGETN:b)
    $(COMMAND_ECHO)cd $(LB) && ln -s $(SHL10TARGETN:f) $(SHL10TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
.IF "$(VERBOSE)" == "TRUE"
    @ls -l $@
.ENDIF
.ENDIF			# "$(GUI)" == "UNX"

.ENDIF			# "$(SHL10TARGETN)"!=""

# unroll begin

.IF "$(SHL1IMPLIBN)" != ""

USELIB1DEPN+=$(SHL1LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL1TARGET=$(SHL1TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL1IMPLIBN):	\
                    $(SHL1DEF) \
                    $(USE_SHL1TARGET) \
                    $(USELIB1DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL1IMPLIBN):	\
                    $(SHL1LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL1IMPLIBN) \
    -def:$(SHL1DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL1TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL2IMPLIBN)" != ""

USELIB2DEPN+=$(SHL2LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL2TARGET=$(SHL2TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL2IMPLIBN):	\
                    $(SHL2DEF) \
                    $(USE_SHL2TARGET) \
                    $(USELIB2DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL2IMPLIBN):	\
                    $(SHL2LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL2IMPLIBN) \
    -def:$(SHL2DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL2TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL3IMPLIBN)" != ""

USELIB3DEPN+=$(SHL3LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL3TARGET=$(SHL3TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL3IMPLIBN):	\
                    $(SHL3DEF) \
                    $(USE_SHL3TARGET) \
                    $(USELIB3DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL3IMPLIBN):	\
                    $(SHL3LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL3IMPLIBN) \
    -def:$(SHL3DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL3TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL4IMPLIBN)" != ""

USELIB4DEPN+=$(SHL4LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL4TARGET=$(SHL4TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL4IMPLIBN):	\
                    $(SHL4DEF) \
                    $(USE_SHL4TARGET) \
                    $(USELIB4DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL4IMPLIBN):	\
                    $(SHL4LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL4IMPLIBN) \
    -def:$(SHL4DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL4TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL5IMPLIBN)" != ""

USELIB5DEPN+=$(SHL5LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL5TARGET=$(SHL5TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL5IMPLIBN):	\
                    $(SHL5DEF) \
                    $(USE_SHL5TARGET) \
                    $(USELIB5DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL5IMPLIBN):	\
                    $(SHL5LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL5IMPLIBN) \
    -def:$(SHL5DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL5TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL6IMPLIBN)" != ""

USELIB6DEPN+=$(SHL6LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL6TARGET=$(SHL6TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL6IMPLIBN):	\
                    $(SHL6DEF) \
                    $(USE_SHL6TARGET) \
                    $(USELIB6DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL6IMPLIBN):	\
                    $(SHL6LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL6IMPLIBN) \
    -def:$(SHL6DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL6TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL7IMPLIBN)" != ""

USELIB7DEPN+=$(SHL7LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL7TARGET=$(SHL7TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL7IMPLIBN):	\
                    $(SHL7DEF) \
                    $(USE_SHL7TARGET) \
                    $(USELIB7DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL7IMPLIBN):	\
                    $(SHL7LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL7IMPLIBN) \
    -def:$(SHL7DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL7TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL8IMPLIBN)" != ""

USELIB8DEPN+=$(SHL8LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL8TARGET=$(SHL8TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL8IMPLIBN):	\
                    $(SHL8DEF) \
                    $(USE_SHL8TARGET) \
                    $(USELIB8DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL8IMPLIBN):	\
                    $(SHL8LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL8IMPLIBN) \
    -def:$(SHL8DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL8TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL9IMPLIBN)" != ""

USELIB9DEPN+=$(SHL9LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL9TARGET=$(SHL9TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL9IMPLIBN):	\
                    $(SHL9DEF) \
                    $(USE_SHL9TARGET) \
                    $(USELIB9DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL9IMPLIBN):	\
                    $(SHL9LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL9IMPLIBN) \
    -def:$(SHL9DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL9TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL10IMPLIBN)" != ""

USELIB10DEPN+=$(SHL10LIBS)

.IF "$(USE_DEFFILE)"!=""
USE_SHL10TARGET=$(SHL10TARGETN)
.ENDIF

.IF "$(GUI)$(COM)" != "WNTGCC"
.IF "$(GUI)" != "UNX"
$(SHL10IMPLIBN):	\
                    $(SHL10DEF) \
                    $(USE_SHL10TARGET) \
                    $(USELIB10DEPN) \
                    $(USELIBDEPN)
.ELSE
$(SHL10IMPLIBN):	\
                    $(SHL10LIBS)
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)" == "WNT"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL10IMPLIBN) \
    -def:$(SHL10DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL10TARGETN) creates $@
    @$(TOUCH) $@
.ENDIF			# "$(USE_DEFFILE)==""

.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

