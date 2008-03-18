# unroll begin

.IF "$(SHL1TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL1STDLIBS=
.ENDIF

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
SHL1DEF*=$(MISC)$/$(SHL1TARGET).def
.ENDIF			# "$(SHL1USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL1VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL1VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL1VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL1VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL1VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL1VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL1VERSIONOBJ:b).h/ > $@

.INIT : $(SHL1VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL1IMPLIB)" == ""
SHL1IMPLIB=i$(TARGET)_t1
.ENDIF			# "$(SHL1IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_1IMPLIB=-implib:$(LB)$/$(SHL1IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL1IMPLIBN=$(LB)$/$(SHL1IMPLIB).lib
ALLTAR : $(SHL1IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_1IMPLIB_DEPS=$(LB)$/$(SHL1IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL1DEF=$(SHL1DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL1DEF=
SHL1DEPN+:=$(SHL1DEPNU)

.IF "$(SHL1VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
.ENDIF			# "$(SHL1VERSIONMAP)"==""

.IF "$(USE_SHL1VERSIONMAP)"!=""

.IF "$(SHL1FILTERFILE)"!=""
.IF "$(SHL1VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL1INTERFACE) -f $(SHL1FILTERFILE) -m $@

.ELSE			# "$(SHL1FILTERFILE)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) :
    @echo -----------------------------
    @echo SHL1FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL1FILTERFILE)"!=""
.ELSE			# "$(USE_SHL1VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL1VERSIONMAP)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1VERSIONMAP:b)_$(SHL1TARGET)$(SHL1VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL1VERSIONMAP): $(SHL1OBJS) $(SHL1LIBS)
.ENDIF

$(USE_SHL1VERSIONMAP): $(SHL1VERSIONMAP)
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
    tr -d "\015" < $(SHL1VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL1VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL1OBJS)"!=""
    -echo $(foreach,i,$(SHL1OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL1LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL1LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL1VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL1VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL1SONAME=$(SONAME_SWITCH)$(SHL1TARGETN:f)
.ELSE
SHL1SONAME=\"$(SONAME_SWITCH)$(SHL1TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL1RES)"!=""
SHL1ALLRES+=$(SHL1RES)
SHL1LINKRES*=$(MISC)$/$(SHL1TARGET).res
SHL1LINKRESO*=$(MISC)$/$(SHL1TARGET)_res.o
.ENDIF			# "$(SHL1RES)"!=""

.IF "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""
SHL1DEFAULTRES*=$(MISC)$/$(SHL1TARGET)_def.res
SHL1ALLRES+=$(SHL1DEFAULTRES)
SHL1LINKRES*=$(MISC)$/$(SHL1TARGET).res
SHL1LINKRESO*=$(MISC)$/$(SHL1TARGET)_res.o
.ENDIF			# "$(SHL1DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL1TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL1TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL1LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL1TARGETN:b)_linkinc.ls
$(SHL1TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL1USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL1LINKLIST=$(MISC)$/$(SHL1TARGET)_link.lst
$(MISC)$/$(SHL1TARGET)_link.lst : $(SHL1LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL1LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL1USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL1TARGETN) : \
                    $(SHL1OBJS)\
                    $(SHL1LIBS)\
                    $(USE_1IMPLIB_DEPS)\
                    $(USE_SHL1DEF)\
                    $(USE_SHL1VERSIONMAP)\
                    $(SHL1RES)\
                    $(SHL1DEPN) \
                    $(SHL1LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL1TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL1DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL1DEFAULTRES:b).rc
.IF "$(SHL1ICON)" != ""
    @echo 1 ICON $(SHL1ICON) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL1ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ELSE			# "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(SHL1ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL1TARGET)$(DLLPOST) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL1TARGET:b) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(SHL1DEFAULTRES)"!=""
.IF "$(SHL1ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL1ALLRES:s/res /res+/) $(SHL1LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL1ALLRES) > $(SHL1LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL1LINKRES) $(SHL1LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL1ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB1NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL1DEF) \
        --dllname $(SHL1TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_1.cmd
.ELSE			# "$(DEFLIB1NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL1DEF) \
        --dllname $(SHL1TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL1OBJS) $(SHL1LINKRESO) \
        `$(TYPE) /dev/null $(SHL1LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_1.cmd
.ENDIF			# "$(DEFLIB1NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) $(SHL1OBJS) $(SHL1LINKRESO) \
        `$(TYPE) /dev/null $(SHL1LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL1STDLIBS) $(SHL1STDSHL) $(STDSHL1) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_1.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL1USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL1LINKER) @$(mktmp \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL1STACK) $(SHL1BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL1DEF) \
        $(USE_1IMPLIB) \
        $(STDOBJ) \
        $(SHL1VERSIONOBJ) $(SHL1OBJS) \
        $(SHL1LIBS) \
        $(SHL1STDLIBS) \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL1LINKER) @$(mktmp	$(SHL1LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        $(SHL1STACK) -out:$(SHL1TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL1IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ) \
        $(SHL1LIBS)                         \
        $(SHL1STDLIBS)                      \
        $(SHL1STDSHL) $(STDSHL1)                           \
        $(SHL1LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL1USE_EXPORTS)"!="name"
    $(SHL1LINKER) @$(mktmp	$(SHL1LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL1BASEX)		\
        $(SHL1STACK) -out:$(SHL1TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_1IMPLIB) \
        $(STDOBJ)							\
        $(SHL1OBJS) $(SHL1VERSIONOBJ))   \
        @$(MISC)$/$(SHL1TARGET)_link.lst \
        @$(mktmp $(SHL1STDLIBS)                      \
        $(SHL1STDSHL) $(STDSHL1)                           \
        $(SHL1LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL1USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL1TARGET).lnk
        -$(RM) $(MISC)$/$(SHL1TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL1LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL1BASEX) \
        $(SHL1STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL1IMPLIB).exp \
        $(STDOBJ) \
        $(SHL1OBJS) \
        $(SHL1STDLIBS) \
        $(SHL1STDSHL) $(STDSHL1) \
        $(SHL1LINKRES) \
        ) >> $(MISC)$/$(SHL1TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL1TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL1TARGET).lnk
        $(SHL1LINKER) @$(MISC)$/$(SHL1TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @echo $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) \
    `cat /dev/null $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL1LINKER) $(SHL1LINKFLAGS) $(SHL1VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(SHL1STDSHL) $(STDSHL1) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_1.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL1RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL1NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL1TARGETN)
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @echo $(SHL1LINKER) $(SHL1LINKFLAGS) $(SHL1SONAME) $(LINKFLAGSSHL) $(SHL1VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL1LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(SHL1STDSHL) $(STDSHL1) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_1.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_1.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL1NOCHECK)"==""
    -$(RM) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
    $(RENAME) $(SHL1TARGETN) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL1TARGETN:b)
    cd $(LB) && ln -s $(SHL1TARGETN:f) $(SHL1TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL1TARGET)

runtest_$(SHL1TARGET) : $(SHL1TARGETN)
    testshl $(SHL1TARGETN) sce$/$(SHL1TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL1TARGETN)"!=""

# unroll begin

.IF "$(SHL2TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL2STDLIBS=
.ENDIF

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
SHL2DEF*=$(MISC)$/$(SHL2TARGET).def
.ENDIF			# "$(SHL2USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL2VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL2VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL2VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL2VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL2VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL2VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL2VERSIONOBJ:b).h/ > $@

.INIT : $(SHL2VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL2IMPLIB)" == ""
SHL2IMPLIB=i$(TARGET)_t2
.ENDIF			# "$(SHL2IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_2IMPLIB=-implib:$(LB)$/$(SHL2IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL2IMPLIBN=$(LB)$/$(SHL2IMPLIB).lib
ALLTAR : $(SHL2IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_2IMPLIB_DEPS=$(LB)$/$(SHL2IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL2DEF=$(SHL2DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL2DEF=
SHL2DEPN+:=$(SHL2DEPNU)

.IF "$(SHL2VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
.ENDIF			# "$(SHL2VERSIONMAP)"==""

.IF "$(USE_SHL2VERSIONMAP)"!=""

.IF "$(SHL2FILTERFILE)"!=""
.IF "$(SHL2VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL2INTERFACE) -f $(SHL2FILTERFILE) -m $@

.ELSE			# "$(SHL2FILTERFILE)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) :
    @echo -----------------------------
    @echo SHL2FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL2FILTERFILE)"!=""
.ELSE			# "$(USE_SHL2VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL2VERSIONMAP)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2VERSIONMAP:b)_$(SHL2TARGET)$(SHL2VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL2VERSIONMAP): $(SHL2OBJS) $(SHL2LIBS)
.ENDIF

$(USE_SHL2VERSIONMAP): $(SHL2VERSIONMAP)
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
    tr -d "\015" < $(SHL2VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL2VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL2OBJS)"!=""
    -echo $(foreach,i,$(SHL2OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL2LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL2LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL2VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL2VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL2SONAME=$(SONAME_SWITCH)$(SHL2TARGETN:f)
.ELSE
SHL2SONAME=\"$(SONAME_SWITCH)$(SHL2TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL2RES)"!=""
SHL2ALLRES+=$(SHL2RES)
SHL2LINKRES*=$(MISC)$/$(SHL2TARGET).res
SHL2LINKRESO*=$(MISC)$/$(SHL2TARGET)_res.o
.ENDIF			# "$(SHL2RES)"!=""

.IF "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""
SHL2DEFAULTRES*=$(MISC)$/$(SHL2TARGET)_def.res
SHL2ALLRES+=$(SHL2DEFAULTRES)
SHL2LINKRES*=$(MISC)$/$(SHL2TARGET).res
SHL2LINKRESO*=$(MISC)$/$(SHL2TARGET)_res.o
.ENDIF			# "$(SHL2DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL2TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL2TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL2LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL2TARGETN:b)_linkinc.ls
$(SHL2TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL2USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL2LINKLIST=$(MISC)$/$(SHL2TARGET)_link.lst
$(MISC)$/$(SHL2TARGET)_link.lst : $(SHL2LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL2LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL2USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL2TARGETN) : \
                    $(SHL2OBJS)\
                    $(SHL2LIBS)\
                    $(USE_2IMPLIB_DEPS)\
                    $(USE_SHL2DEF)\
                    $(USE_SHL2VERSIONMAP)\
                    $(SHL2RES)\
                    $(SHL2DEPN) \
                    $(SHL2LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL2TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL2DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL2DEFAULTRES:b).rc
.IF "$(SHL2ICON)" != ""
    @echo 1 ICON $(SHL2ICON) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL2ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ELSE			# "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(SHL2ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL2TARGET)$(DLLPOST) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL2TARGET:b) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(SHL2DEFAULTRES)"!=""
.IF "$(SHL2ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL2ALLRES:s/res /res+/) $(SHL2LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL2ALLRES) > $(SHL2LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL2LINKRES) $(SHL2LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL2ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB2NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL2DEF) \
        --dllname $(SHL2TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_2.cmd
.ELSE			# "$(DEFLIB2NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL2DEF) \
        --dllname $(SHL2TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL2OBJS) $(SHL2LINKRESO) \
        `$(TYPE) /dev/null $(SHL2LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_2.cmd
.ENDIF			# "$(DEFLIB2NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) $(SHL2OBJS) $(SHL2LINKRESO) \
        `$(TYPE) /dev/null $(SHL2LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL2STDLIBS) $(SHL2STDSHL) $(STDSHL2) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_2.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL2USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL2LINKER) @$(mktmp \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL2STACK) $(SHL2BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL2DEF) \
        $(USE_2IMPLIB) \
        $(STDOBJ) \
        $(SHL2VERSIONOBJ) $(SHL2OBJS) \
        $(SHL2LIBS) \
        $(SHL2STDLIBS) \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL2LINKER) @$(mktmp	$(SHL2LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        $(SHL2STACK) -out:$(SHL2TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL2IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ) \
        $(SHL2LIBS)                         \
        $(SHL2STDLIBS)                      \
        $(SHL2STDSHL) $(STDSHL2)                           \
        $(SHL2LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL2USE_EXPORTS)"!="name"
    $(SHL2LINKER) @$(mktmp	$(SHL2LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL2BASEX)		\
        $(SHL2STACK) -out:$(SHL2TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_2IMPLIB) \
        $(STDOBJ)							\
        $(SHL2OBJS) $(SHL2VERSIONOBJ))   \
        @$(MISC)$/$(SHL2TARGET)_link.lst \
        @$(mktmp $(SHL2STDLIBS)                      \
        $(SHL2STDSHL) $(STDSHL2)                           \
        $(SHL2LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL2USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL2TARGET).lnk
        -$(RM) $(MISC)$/$(SHL2TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL2LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL2BASEX) \
        $(SHL2STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL2IMPLIB).exp \
        $(STDOBJ) \
        $(SHL2OBJS) \
        $(SHL2STDLIBS) \
        $(SHL2STDSHL) $(STDSHL2) \
        $(SHL2LINKRES) \
        ) >> $(MISC)$/$(SHL2TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL2TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL2TARGET).lnk
        $(SHL2LINKER) @$(MISC)$/$(SHL2TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @echo $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) \
    `cat /dev/null $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL2LINKER) $(SHL2LINKFLAGS) $(SHL2VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(SHL2STDSHL) $(STDSHL2) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_2.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL2RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL2NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL2TARGETN)
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @echo $(SHL2LINKER) $(SHL2LINKFLAGS) $(SHL2SONAME) $(LINKFLAGSSHL) $(SHL2VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL2LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(SHL2STDSHL) $(STDSHL2) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_2.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_2.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL2NOCHECK)"==""
    -$(RM) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
    $(RENAME) $(SHL2TARGETN) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL2TARGETN:b)
    cd $(LB) && ln -s $(SHL2TARGETN:f) $(SHL2TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL2TARGET)

runtest_$(SHL2TARGET) : $(SHL2TARGETN)
    testshl $(SHL2TARGETN) sce$/$(SHL2TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL2TARGETN)"!=""

# unroll begin

.IF "$(SHL3TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL3STDLIBS=
.ENDIF

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
SHL3DEF*=$(MISC)$/$(SHL3TARGET).def
.ENDIF			# "$(SHL3USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL3VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL3VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL3VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL3VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL3VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL3VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL3VERSIONOBJ:b).h/ > $@

.INIT : $(SHL3VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL3IMPLIB)" == ""
SHL3IMPLIB=i$(TARGET)_t3
.ENDIF			# "$(SHL3IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_3IMPLIB=-implib:$(LB)$/$(SHL3IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL3IMPLIBN=$(LB)$/$(SHL3IMPLIB).lib
ALLTAR : $(SHL3IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_3IMPLIB_DEPS=$(LB)$/$(SHL3IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL3DEF=$(SHL3DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL3DEF=
SHL3DEPN+:=$(SHL3DEPNU)

.IF "$(SHL3VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
.ENDIF			# "$(SHL3VERSIONMAP)"==""

.IF "$(USE_SHL3VERSIONMAP)"!=""

.IF "$(SHL3FILTERFILE)"!=""
.IF "$(SHL3VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL3INTERFACE) -f $(SHL3FILTERFILE) -m $@

.ELSE			# "$(SHL3FILTERFILE)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) :
    @echo -----------------------------
    @echo SHL3FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL3FILTERFILE)"!=""
.ELSE			# "$(USE_SHL3VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL3VERSIONMAP)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3VERSIONMAP:b)_$(SHL3TARGET)$(SHL3VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL3VERSIONMAP): $(SHL3OBJS) $(SHL3LIBS)
.ENDIF

$(USE_SHL3VERSIONMAP): $(SHL3VERSIONMAP)
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
    tr -d "\015" < $(SHL3VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL3VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL3OBJS)"!=""
    -echo $(foreach,i,$(SHL3OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL3LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL3LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL3VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL3VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL3SONAME=$(SONAME_SWITCH)$(SHL3TARGETN:f)
.ELSE
SHL3SONAME=\"$(SONAME_SWITCH)$(SHL3TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL3RES)"!=""
SHL3ALLRES+=$(SHL3RES)
SHL3LINKRES*=$(MISC)$/$(SHL3TARGET).res
SHL3LINKRESO*=$(MISC)$/$(SHL3TARGET)_res.o
.ENDIF			# "$(SHL3RES)"!=""

.IF "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""
SHL3DEFAULTRES*=$(MISC)$/$(SHL3TARGET)_def.res
SHL3ALLRES+=$(SHL3DEFAULTRES)
SHL3LINKRES*=$(MISC)$/$(SHL3TARGET).res
SHL3LINKRESO*=$(MISC)$/$(SHL3TARGET)_res.o
.ENDIF			# "$(SHL3DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL3TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL3TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL3LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL3TARGETN:b)_linkinc.ls
$(SHL3TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL3USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL3LINKLIST=$(MISC)$/$(SHL3TARGET)_link.lst
$(MISC)$/$(SHL3TARGET)_link.lst : $(SHL3LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL3LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL3USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL3TARGETN) : \
                    $(SHL3OBJS)\
                    $(SHL3LIBS)\
                    $(USE_3IMPLIB_DEPS)\
                    $(USE_SHL3DEF)\
                    $(USE_SHL3VERSIONMAP)\
                    $(SHL3RES)\
                    $(SHL3DEPN) \
                    $(SHL3LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL3TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL3DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL3DEFAULTRES:b).rc
.IF "$(SHL3ICON)" != ""
    @echo 1 ICON $(SHL3ICON) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL3ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ELSE			# "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(SHL3ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL3TARGET)$(DLLPOST) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL3TARGET:b) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(SHL3DEFAULTRES)"!=""
.IF "$(SHL3ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL3ALLRES:s/res /res+/) $(SHL3LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL3ALLRES) > $(SHL3LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL3LINKRES) $(SHL3LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL3ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB3NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL3DEF) \
        --dllname $(SHL3TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_3.cmd
.ELSE			# "$(DEFLIB3NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL3DEF) \
        --dllname $(SHL3TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL3OBJS) $(SHL3LINKRESO) \
        `$(TYPE) /dev/null $(SHL3LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_3.cmd
.ENDIF			# "$(DEFLIB3NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) $(SHL3OBJS) $(SHL3LINKRESO) \
        `$(TYPE) /dev/null $(SHL3LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL3STDLIBS) $(SHL3STDSHL) $(STDSHL3) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_3.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL3USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL3LINKER) @$(mktmp \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL3STACK) $(SHL3BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL3DEF) \
        $(USE_3IMPLIB) \
        $(STDOBJ) \
        $(SHL3VERSIONOBJ) $(SHL3OBJS) \
        $(SHL3LIBS) \
        $(SHL3STDLIBS) \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL3LINKER) @$(mktmp	$(SHL3LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        $(SHL3STACK) -out:$(SHL3TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL3IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ) \
        $(SHL3LIBS)                         \
        $(SHL3STDLIBS)                      \
        $(SHL3STDSHL) $(STDSHL3)                           \
        $(SHL3LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL3USE_EXPORTS)"!="name"
    $(SHL3LINKER) @$(mktmp	$(SHL3LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL3BASEX)		\
        $(SHL3STACK) -out:$(SHL3TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_3IMPLIB) \
        $(STDOBJ)							\
        $(SHL3OBJS) $(SHL3VERSIONOBJ))   \
        @$(MISC)$/$(SHL3TARGET)_link.lst \
        @$(mktmp $(SHL3STDLIBS)                      \
        $(SHL3STDSHL) $(STDSHL3)                           \
        $(SHL3LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL3USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL3TARGET).lnk
        -$(RM) $(MISC)$/$(SHL3TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL3LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL3BASEX) \
        $(SHL3STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL3IMPLIB).exp \
        $(STDOBJ) \
        $(SHL3OBJS) \
        $(SHL3STDLIBS) \
        $(SHL3STDSHL) $(STDSHL3) \
        $(SHL3LINKRES) \
        ) >> $(MISC)$/$(SHL3TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL3TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL3TARGET).lnk
        $(SHL3LINKER) @$(MISC)$/$(SHL3TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @echo $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) \
    `cat /dev/null $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL3LINKER) $(SHL3LINKFLAGS) $(SHL3VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(SHL3STDSHL) $(STDSHL3) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_3.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL3RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL3NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL3TARGETN)
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @echo $(SHL3LINKER) $(SHL3LINKFLAGS) $(SHL3SONAME) $(LINKFLAGSSHL) $(SHL3VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL3LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(SHL3STDSHL) $(STDSHL3) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_3.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_3.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL3NOCHECK)"==""
    -$(RM) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
    $(RENAME) $(SHL3TARGETN) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL3TARGETN:b)
    cd $(LB) && ln -s $(SHL3TARGETN:f) $(SHL3TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL3TARGET)

runtest_$(SHL3TARGET) : $(SHL3TARGETN)
    testshl $(SHL3TARGETN) sce$/$(SHL3TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL3TARGETN)"!=""

# unroll begin

.IF "$(SHL4TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL4STDLIBS=
.ENDIF

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
SHL4DEF*=$(MISC)$/$(SHL4TARGET).def
.ENDIF			# "$(SHL4USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL4VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL4VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL4VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL4VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL4VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL4VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL4VERSIONOBJ:b).h/ > $@

.INIT : $(SHL4VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL4IMPLIB)" == ""
SHL4IMPLIB=i$(TARGET)_t4
.ENDIF			# "$(SHL4IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_4IMPLIB=-implib:$(LB)$/$(SHL4IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL4IMPLIBN=$(LB)$/$(SHL4IMPLIB).lib
ALLTAR : $(SHL4IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_4IMPLIB_DEPS=$(LB)$/$(SHL4IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL4DEF=$(SHL4DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL4DEF=
SHL4DEPN+:=$(SHL4DEPNU)

.IF "$(SHL4VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
.ENDIF			# "$(SHL4VERSIONMAP)"==""

.IF "$(USE_SHL4VERSIONMAP)"!=""

.IF "$(SHL4FILTERFILE)"!=""
.IF "$(SHL4VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL4INTERFACE) -f $(SHL4FILTERFILE) -m $@

.ELSE			# "$(SHL4FILTERFILE)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) :
    @echo -----------------------------
    @echo SHL4FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL4FILTERFILE)"!=""
.ELSE			# "$(USE_SHL4VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL4VERSIONMAP)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4VERSIONMAP:b)_$(SHL4TARGET)$(SHL4VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL4VERSIONMAP): $(SHL4OBJS) $(SHL4LIBS)
.ENDIF

$(USE_SHL4VERSIONMAP): $(SHL4VERSIONMAP)
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
    tr -d "\015" < $(SHL4VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL4VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL4OBJS)"!=""
    -echo $(foreach,i,$(SHL4OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL4LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL4LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL4VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL4VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL4SONAME=$(SONAME_SWITCH)$(SHL4TARGETN:f)
.ELSE
SHL4SONAME=\"$(SONAME_SWITCH)$(SHL4TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL4RES)"!=""
SHL4ALLRES+=$(SHL4RES)
SHL4LINKRES*=$(MISC)$/$(SHL4TARGET).res
SHL4LINKRESO*=$(MISC)$/$(SHL4TARGET)_res.o
.ENDIF			# "$(SHL4RES)"!=""

.IF "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""
SHL4DEFAULTRES*=$(MISC)$/$(SHL4TARGET)_def.res
SHL4ALLRES+=$(SHL4DEFAULTRES)
SHL4LINKRES*=$(MISC)$/$(SHL4TARGET).res
SHL4LINKRESO*=$(MISC)$/$(SHL4TARGET)_res.o
.ENDIF			# "$(SHL4DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL4TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL4TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL4LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL4TARGETN:b)_linkinc.ls
$(SHL4TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL4USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL4LINKLIST=$(MISC)$/$(SHL4TARGET)_link.lst
$(MISC)$/$(SHL4TARGET)_link.lst : $(SHL4LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL4LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL4USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL4TARGETN) : \
                    $(SHL4OBJS)\
                    $(SHL4LIBS)\
                    $(USE_4IMPLIB_DEPS)\
                    $(USE_SHL4DEF)\
                    $(USE_SHL4VERSIONMAP)\
                    $(SHL4RES)\
                    $(SHL4DEPN) \
                    $(SHL4LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL4TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL4DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL4DEFAULTRES:b).rc
.IF "$(SHL4ICON)" != ""
    @echo 1 ICON $(SHL4ICON) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL4ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ELSE			# "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(SHL4ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL4TARGET)$(DLLPOST) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL4TARGET:b) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(SHL4DEFAULTRES)"!=""
.IF "$(SHL4ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL4ALLRES:s/res /res+/) $(SHL4LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL4ALLRES) > $(SHL4LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL4LINKRES) $(SHL4LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL4ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB4NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL4DEF) \
        --dllname $(SHL4TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_4.cmd
.ELSE			# "$(DEFLIB4NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL4DEF) \
        --dllname $(SHL4TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL4OBJS) $(SHL4LINKRESO) \
        `$(TYPE) /dev/null $(SHL4LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_4.cmd
.ENDIF			# "$(DEFLIB4NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) $(SHL4OBJS) $(SHL4LINKRESO) \
        `$(TYPE) /dev/null $(SHL4LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL4STDLIBS) $(SHL4STDSHL) $(STDSHL4) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_4.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL4USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL4LINKER) @$(mktmp \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL4STACK) $(SHL4BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL4DEF) \
        $(USE_4IMPLIB) \
        $(STDOBJ) \
        $(SHL4VERSIONOBJ) $(SHL4OBJS) \
        $(SHL4LIBS) \
        $(SHL4STDLIBS) \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL4LINKER) @$(mktmp	$(SHL4LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        $(SHL4STACK) -out:$(SHL4TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL4IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ) \
        $(SHL4LIBS)                         \
        $(SHL4STDLIBS)                      \
        $(SHL4STDSHL) $(STDSHL4)                           \
        $(SHL4LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL4USE_EXPORTS)"!="name"
    $(SHL4LINKER) @$(mktmp	$(SHL4LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL4BASEX)		\
        $(SHL4STACK) -out:$(SHL4TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_4IMPLIB) \
        $(STDOBJ)							\
        $(SHL4OBJS) $(SHL4VERSIONOBJ))   \
        @$(MISC)$/$(SHL4TARGET)_link.lst \
        @$(mktmp $(SHL4STDLIBS)                      \
        $(SHL4STDSHL) $(STDSHL4)                           \
        $(SHL4LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL4USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL4TARGET).lnk
        -$(RM) $(MISC)$/$(SHL4TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL4LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL4BASEX) \
        $(SHL4STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL4IMPLIB).exp \
        $(STDOBJ) \
        $(SHL4OBJS) \
        $(SHL4STDLIBS) \
        $(SHL4STDSHL) $(STDSHL4) \
        $(SHL4LINKRES) \
        ) >> $(MISC)$/$(SHL4TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL4TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL4TARGET).lnk
        $(SHL4LINKER) @$(MISC)$/$(SHL4TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @echo $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) \
    `cat /dev/null $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL4LINKER) $(SHL4LINKFLAGS) $(SHL4VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(SHL4STDSHL) $(STDSHL4) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_4.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL4RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL4NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL4TARGETN)
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @echo $(SHL4LINKER) $(SHL4LINKFLAGS) $(SHL4SONAME) $(LINKFLAGSSHL) $(SHL4VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL4LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(SHL4STDSHL) $(STDSHL4) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_4.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_4.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL4NOCHECK)"==""
    -$(RM) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
    $(RENAME) $(SHL4TARGETN) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL4TARGETN:b)
    cd $(LB) && ln -s $(SHL4TARGETN:f) $(SHL4TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL4TARGET)

runtest_$(SHL4TARGET) : $(SHL4TARGETN)
    testshl $(SHL4TARGETN) sce$/$(SHL4TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL4TARGETN)"!=""

# unroll begin

.IF "$(SHL5TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL5STDLIBS=
.ENDIF

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
SHL5DEF*=$(MISC)$/$(SHL5TARGET).def
.ENDIF			# "$(SHL5USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL5VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL5VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL5VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL5VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL5VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL5VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL5VERSIONOBJ:b).h/ > $@

.INIT : $(SHL5VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL5IMPLIB)" == ""
SHL5IMPLIB=i$(TARGET)_t5
.ENDIF			# "$(SHL5IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_5IMPLIB=-implib:$(LB)$/$(SHL5IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL5IMPLIBN=$(LB)$/$(SHL5IMPLIB).lib
ALLTAR : $(SHL5IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_5IMPLIB_DEPS=$(LB)$/$(SHL5IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL5DEF=$(SHL5DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL5DEF=
SHL5DEPN+:=$(SHL5DEPNU)

.IF "$(SHL5VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
.ENDIF			# "$(SHL5VERSIONMAP)"==""

.IF "$(USE_SHL5VERSIONMAP)"!=""

.IF "$(SHL5FILTERFILE)"!=""
.IF "$(SHL5VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL5INTERFACE) -f $(SHL5FILTERFILE) -m $@

.ELSE			# "$(SHL5FILTERFILE)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) :
    @echo -----------------------------
    @echo SHL5FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL5FILTERFILE)"!=""
.ELSE			# "$(USE_SHL5VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL5VERSIONMAP)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5VERSIONMAP:b)_$(SHL5TARGET)$(SHL5VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL5VERSIONMAP): $(SHL5OBJS) $(SHL5LIBS)
.ENDIF

$(USE_SHL5VERSIONMAP): $(SHL5VERSIONMAP)
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
    tr -d "\015" < $(SHL5VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL5VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL5OBJS)"!=""
    -echo $(foreach,i,$(SHL5OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL5LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL5LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL5VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL5VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL5SONAME=$(SONAME_SWITCH)$(SHL5TARGETN:f)
.ELSE
SHL5SONAME=\"$(SONAME_SWITCH)$(SHL5TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL5RES)"!=""
SHL5ALLRES+=$(SHL5RES)
SHL5LINKRES*=$(MISC)$/$(SHL5TARGET).res
SHL5LINKRESO*=$(MISC)$/$(SHL5TARGET)_res.o
.ENDIF			# "$(SHL5RES)"!=""

.IF "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""
SHL5DEFAULTRES*=$(MISC)$/$(SHL5TARGET)_def.res
SHL5ALLRES+=$(SHL5DEFAULTRES)
SHL5LINKRES*=$(MISC)$/$(SHL5TARGET).res
SHL5LINKRESO*=$(MISC)$/$(SHL5TARGET)_res.o
.ENDIF			# "$(SHL5DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL5TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL5TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL5LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL5TARGETN:b)_linkinc.ls
$(SHL5TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL5USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL5LINKLIST=$(MISC)$/$(SHL5TARGET)_link.lst
$(MISC)$/$(SHL5TARGET)_link.lst : $(SHL5LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL5LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL5USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL5TARGETN) : \
                    $(SHL5OBJS)\
                    $(SHL5LIBS)\
                    $(USE_5IMPLIB_DEPS)\
                    $(USE_SHL5DEF)\
                    $(USE_SHL5VERSIONMAP)\
                    $(SHL5RES)\
                    $(SHL5DEPN) \
                    $(SHL5LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL5TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL5DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL5DEFAULTRES:b).rc
.IF "$(SHL5ICON)" != ""
    @echo 1 ICON $(SHL5ICON) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL5ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ELSE			# "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(SHL5ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL5TARGET)$(DLLPOST) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL5TARGET:b) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(SHL5DEFAULTRES)"!=""
.IF "$(SHL5ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL5ALLRES:s/res /res+/) $(SHL5LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL5ALLRES) > $(SHL5LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL5LINKRES) $(SHL5LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL5ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB5NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL5DEF) \
        --dllname $(SHL5TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_5.cmd
.ELSE			# "$(DEFLIB5NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL5DEF) \
        --dllname $(SHL5TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL5OBJS) $(SHL5LINKRESO) \
        `$(TYPE) /dev/null $(SHL5LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_5.cmd
.ENDIF			# "$(DEFLIB5NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) $(SHL5OBJS) $(SHL5LINKRESO) \
        `$(TYPE) /dev/null $(SHL5LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL5STDLIBS) $(SHL5STDSHL) $(STDSHL5) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_5.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL5USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL5LINKER) @$(mktmp \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL5STACK) $(SHL5BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL5DEF) \
        $(USE_5IMPLIB) \
        $(STDOBJ) \
        $(SHL5VERSIONOBJ) $(SHL5OBJS) \
        $(SHL5LIBS) \
        $(SHL5STDLIBS) \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL5LINKER) @$(mktmp	$(SHL5LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        $(SHL5STACK) -out:$(SHL5TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL5IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ) \
        $(SHL5LIBS)                         \
        $(SHL5STDLIBS)                      \
        $(SHL5STDSHL) $(STDSHL5)                           \
        $(SHL5LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL5USE_EXPORTS)"!="name"
    $(SHL5LINKER) @$(mktmp	$(SHL5LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL5BASEX)		\
        $(SHL5STACK) -out:$(SHL5TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_5IMPLIB) \
        $(STDOBJ)							\
        $(SHL5OBJS) $(SHL5VERSIONOBJ))   \
        @$(MISC)$/$(SHL5TARGET)_link.lst \
        @$(mktmp $(SHL5STDLIBS)                      \
        $(SHL5STDSHL) $(STDSHL5)                           \
        $(SHL5LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL5USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL5TARGET).lnk
        -$(RM) $(MISC)$/$(SHL5TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL5LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL5BASEX) \
        $(SHL5STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL5IMPLIB).exp \
        $(STDOBJ) \
        $(SHL5OBJS) \
        $(SHL5STDLIBS) \
        $(SHL5STDSHL) $(STDSHL5) \
        $(SHL5LINKRES) \
        ) >> $(MISC)$/$(SHL5TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL5TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL5TARGET).lnk
        $(SHL5LINKER) @$(MISC)$/$(SHL5TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @echo $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) \
    `cat /dev/null $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL5LINKER) $(SHL5LINKFLAGS) $(SHL5VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(SHL5STDSHL) $(STDSHL5) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_5.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL5RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL5NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL5TARGETN)
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @echo $(SHL5LINKER) $(SHL5LINKFLAGS) $(SHL5SONAME) $(LINKFLAGSSHL) $(SHL5VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL5LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(SHL5STDSHL) $(STDSHL5) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_5.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_5.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL5NOCHECK)"==""
    -$(RM) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
    $(RENAME) $(SHL5TARGETN) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL5TARGETN:b)
    cd $(LB) && ln -s $(SHL5TARGETN:f) $(SHL5TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL5TARGET)

runtest_$(SHL5TARGET) : $(SHL5TARGETN)
    testshl $(SHL5TARGETN) sce$/$(SHL5TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL5TARGETN)"!=""

# unroll begin

.IF "$(SHL6TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL6STDLIBS=
.ENDIF

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
SHL6DEF*=$(MISC)$/$(SHL6TARGET).def
.ENDIF			# "$(SHL6USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL6VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL6VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL6VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL6VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL6VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL6VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL6VERSIONOBJ:b).h/ > $@

.INIT : $(SHL6VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL6IMPLIB)" == ""
SHL6IMPLIB=i$(TARGET)_t6
.ENDIF			# "$(SHL6IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_6IMPLIB=-implib:$(LB)$/$(SHL6IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL6IMPLIBN=$(LB)$/$(SHL6IMPLIB).lib
ALLTAR : $(SHL6IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_6IMPLIB_DEPS=$(LB)$/$(SHL6IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL6DEF=$(SHL6DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL6DEF=
SHL6DEPN+:=$(SHL6DEPNU)

.IF "$(SHL6VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
.ENDIF			# "$(SHL6VERSIONMAP)"==""

.IF "$(USE_SHL6VERSIONMAP)"!=""

.IF "$(SHL6FILTERFILE)"!=""
.IF "$(SHL6VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL6INTERFACE) -f $(SHL6FILTERFILE) -m $@

.ELSE			# "$(SHL6FILTERFILE)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) :
    @echo -----------------------------
    @echo SHL6FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL6FILTERFILE)"!=""
.ELSE			# "$(USE_SHL6VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL6VERSIONMAP)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6VERSIONMAP:b)_$(SHL6TARGET)$(SHL6VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL6VERSIONMAP): $(SHL6OBJS) $(SHL6LIBS)
.ENDIF

$(USE_SHL6VERSIONMAP): $(SHL6VERSIONMAP)
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
    tr -d "\015" < $(SHL6VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL6VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL6OBJS)"!=""
    -echo $(foreach,i,$(SHL6OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL6LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL6LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL6VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL6VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL6SONAME=$(SONAME_SWITCH)$(SHL6TARGETN:f)
.ELSE
SHL6SONAME=\"$(SONAME_SWITCH)$(SHL6TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL6RES)"!=""
SHL6ALLRES+=$(SHL6RES)
SHL6LINKRES*=$(MISC)$/$(SHL6TARGET).res
SHL6LINKRESO*=$(MISC)$/$(SHL6TARGET)_res.o
.ENDIF			# "$(SHL6RES)"!=""

.IF "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""
SHL6DEFAULTRES*=$(MISC)$/$(SHL6TARGET)_def.res
SHL6ALLRES+=$(SHL6DEFAULTRES)
SHL6LINKRES*=$(MISC)$/$(SHL6TARGET).res
SHL6LINKRESO*=$(MISC)$/$(SHL6TARGET)_res.o
.ENDIF			# "$(SHL6DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL6TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL6TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL6LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL6TARGETN:b)_linkinc.ls
$(SHL6TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL6USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL6LINKLIST=$(MISC)$/$(SHL6TARGET)_link.lst
$(MISC)$/$(SHL6TARGET)_link.lst : $(SHL6LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL6LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL6USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL6TARGETN) : \
                    $(SHL6OBJS)\
                    $(SHL6LIBS)\
                    $(USE_6IMPLIB_DEPS)\
                    $(USE_SHL6DEF)\
                    $(USE_SHL6VERSIONMAP)\
                    $(SHL6RES)\
                    $(SHL6DEPN) \
                    $(SHL6LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL6TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL6DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL6DEFAULTRES:b).rc
.IF "$(SHL6ICON)" != ""
    @echo 1 ICON $(SHL6ICON) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL6ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ELSE			# "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(SHL6ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL6TARGET)$(DLLPOST) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL6TARGET:b) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(SHL6DEFAULTRES)"!=""
.IF "$(SHL6ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL6ALLRES:s/res /res+/) $(SHL6LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL6ALLRES) > $(SHL6LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL6LINKRES) $(SHL6LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL6ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB6NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL6DEF) \
        --dllname $(SHL6TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_6.cmd
.ELSE			# "$(DEFLIB6NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL6DEF) \
        --dllname $(SHL6TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL6OBJS) $(SHL6LINKRESO) \
        `$(TYPE) /dev/null $(SHL6LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_6.cmd
.ENDIF			# "$(DEFLIB6NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) $(SHL6OBJS) $(SHL6LINKRESO) \
        `$(TYPE) /dev/null $(SHL6LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL6STDLIBS) $(SHL6STDSHL) $(STDSHL6) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_6.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL6USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL6LINKER) @$(mktmp \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL6STACK) $(SHL6BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL6DEF) \
        $(USE_6IMPLIB) \
        $(STDOBJ) \
        $(SHL6VERSIONOBJ) $(SHL6OBJS) \
        $(SHL6LIBS) \
        $(SHL6STDLIBS) \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL6LINKER) @$(mktmp	$(SHL6LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        $(SHL6STACK) -out:$(SHL6TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL6IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ) \
        $(SHL6LIBS)                         \
        $(SHL6STDLIBS)                      \
        $(SHL6STDSHL) $(STDSHL6)                           \
        $(SHL6LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL6USE_EXPORTS)"!="name"
    $(SHL6LINKER) @$(mktmp	$(SHL6LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL6BASEX)		\
        $(SHL6STACK) -out:$(SHL6TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_6IMPLIB) \
        $(STDOBJ)							\
        $(SHL6OBJS) $(SHL6VERSIONOBJ))   \
        @$(MISC)$/$(SHL6TARGET)_link.lst \
        @$(mktmp $(SHL6STDLIBS)                      \
        $(SHL6STDSHL) $(STDSHL6)                           \
        $(SHL6LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL6USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL6TARGET).lnk
        -$(RM) $(MISC)$/$(SHL6TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL6LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL6BASEX) \
        $(SHL6STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL6IMPLIB).exp \
        $(STDOBJ) \
        $(SHL6OBJS) \
        $(SHL6STDLIBS) \
        $(SHL6STDSHL) $(STDSHL6) \
        $(SHL6LINKRES) \
        ) >> $(MISC)$/$(SHL6TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL6TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL6TARGET).lnk
        $(SHL6LINKER) @$(MISC)$/$(SHL6TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @echo $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) \
    `cat /dev/null $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL6LINKER) $(SHL6LINKFLAGS) $(SHL6VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(SHL6STDSHL) $(STDSHL6) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_6.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL6RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL6NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL6TARGETN)
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @echo $(SHL6LINKER) $(SHL6LINKFLAGS) $(SHL6SONAME) $(LINKFLAGSSHL) $(SHL6VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL6LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(SHL6STDSHL) $(STDSHL6) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_6.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_6.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL6NOCHECK)"==""
    -$(RM) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
    $(RENAME) $(SHL6TARGETN) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL6TARGETN:b)
    cd $(LB) && ln -s $(SHL6TARGETN:f) $(SHL6TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL6TARGET)

runtest_$(SHL6TARGET) : $(SHL6TARGETN)
    testshl $(SHL6TARGETN) sce$/$(SHL6TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL6TARGETN)"!=""

# unroll begin

.IF "$(SHL7TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL7STDLIBS=
.ENDIF

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
SHL7DEF*=$(MISC)$/$(SHL7TARGET).def
.ENDIF			# "$(SHL7USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL7VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL7VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL7VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL7VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL7VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL7VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL7VERSIONOBJ:b).h/ > $@

.INIT : $(SHL7VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL7IMPLIB)" == ""
SHL7IMPLIB=i$(TARGET)_t7
.ENDIF			# "$(SHL7IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_7IMPLIB=-implib:$(LB)$/$(SHL7IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL7IMPLIBN=$(LB)$/$(SHL7IMPLIB).lib
ALLTAR : $(SHL7IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_7IMPLIB_DEPS=$(LB)$/$(SHL7IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL7DEF=$(SHL7DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL7DEF=
SHL7DEPN+:=$(SHL7DEPNU)

.IF "$(SHL7VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
.ENDIF			# "$(SHL7VERSIONMAP)"==""

.IF "$(USE_SHL7VERSIONMAP)"!=""

.IF "$(SHL7FILTERFILE)"!=""
.IF "$(SHL7VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL7INTERFACE) -f $(SHL7FILTERFILE) -m $@

.ELSE			# "$(SHL7FILTERFILE)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) :
    @echo -----------------------------
    @echo SHL7FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL7FILTERFILE)"!=""
.ELSE			# "$(USE_SHL7VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL7VERSIONMAP)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7VERSIONMAP:b)_$(SHL7TARGET)$(SHL7VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL7VERSIONMAP): $(SHL7OBJS) $(SHL7LIBS)
.ENDIF

$(USE_SHL7VERSIONMAP): $(SHL7VERSIONMAP)
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
    tr -d "\015" < $(SHL7VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL7VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL7OBJS)"!=""
    -echo $(foreach,i,$(SHL7OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL7LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL7LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL7VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL7VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL7SONAME=$(SONAME_SWITCH)$(SHL7TARGETN:f)
.ELSE
SHL7SONAME=\"$(SONAME_SWITCH)$(SHL7TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL7RES)"!=""
SHL7ALLRES+=$(SHL7RES)
SHL7LINKRES*=$(MISC)$/$(SHL7TARGET).res
SHL7LINKRESO*=$(MISC)$/$(SHL7TARGET)_res.o
.ENDIF			# "$(SHL7RES)"!=""

.IF "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""
SHL7DEFAULTRES*=$(MISC)$/$(SHL7TARGET)_def.res
SHL7ALLRES+=$(SHL7DEFAULTRES)
SHL7LINKRES*=$(MISC)$/$(SHL7TARGET).res
SHL7LINKRESO*=$(MISC)$/$(SHL7TARGET)_res.o
.ENDIF			# "$(SHL7DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL7TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL7TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL7LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL7TARGETN:b)_linkinc.ls
$(SHL7TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL7USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL7LINKLIST=$(MISC)$/$(SHL7TARGET)_link.lst
$(MISC)$/$(SHL7TARGET)_link.lst : $(SHL7LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL7LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL7USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL7TARGETN) : \
                    $(SHL7OBJS)\
                    $(SHL7LIBS)\
                    $(USE_7IMPLIB_DEPS)\
                    $(USE_SHL7DEF)\
                    $(USE_SHL7VERSIONMAP)\
                    $(SHL7RES)\
                    $(SHL7DEPN) \
                    $(SHL7LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL7TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL7DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL7DEFAULTRES:b).rc
.IF "$(SHL7ICON)" != ""
    @echo 1 ICON $(SHL7ICON) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL7ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ELSE			# "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(SHL7ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL7TARGET)$(DLLPOST) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL7TARGET:b) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(SHL7DEFAULTRES)"!=""
.IF "$(SHL7ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL7ALLRES:s/res /res+/) $(SHL7LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL7ALLRES) > $(SHL7LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL7LINKRES) $(SHL7LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL7ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB7NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL7DEF) \
        --dllname $(SHL7TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_7.cmd
.ELSE			# "$(DEFLIB7NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL7DEF) \
        --dllname $(SHL7TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL7OBJS) $(SHL7LINKRESO) \
        `$(TYPE) /dev/null $(SHL7LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_7.cmd
.ENDIF			# "$(DEFLIB7NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) $(SHL7OBJS) $(SHL7LINKRESO) \
        `$(TYPE) /dev/null $(SHL7LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL7STDLIBS) $(SHL7STDSHL) $(STDSHL7) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_7.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL7USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL7LINKER) @$(mktmp \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL7STACK) $(SHL7BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL7DEF) \
        $(USE_7IMPLIB) \
        $(STDOBJ) \
        $(SHL7VERSIONOBJ) $(SHL7OBJS) \
        $(SHL7LIBS) \
        $(SHL7STDLIBS) \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL7LINKER) @$(mktmp	$(SHL7LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        $(SHL7STACK) -out:$(SHL7TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL7IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ) \
        $(SHL7LIBS)                         \
        $(SHL7STDLIBS)                      \
        $(SHL7STDSHL) $(STDSHL7)                           \
        $(SHL7LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL7USE_EXPORTS)"!="name"
    $(SHL7LINKER) @$(mktmp	$(SHL7LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL7BASEX)		\
        $(SHL7STACK) -out:$(SHL7TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_7IMPLIB) \
        $(STDOBJ)							\
        $(SHL7OBJS) $(SHL7VERSIONOBJ))   \
        @$(MISC)$/$(SHL7TARGET)_link.lst \
        @$(mktmp $(SHL7STDLIBS)                      \
        $(SHL7STDSHL) $(STDSHL7)                           \
        $(SHL7LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL7USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL7TARGET).lnk
        -$(RM) $(MISC)$/$(SHL7TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL7LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL7BASEX) \
        $(SHL7STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL7IMPLIB).exp \
        $(STDOBJ) \
        $(SHL7OBJS) \
        $(SHL7STDLIBS) \
        $(SHL7STDSHL) $(STDSHL7) \
        $(SHL7LINKRES) \
        ) >> $(MISC)$/$(SHL7TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL7TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL7TARGET).lnk
        $(SHL7LINKER) @$(MISC)$/$(SHL7TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @echo $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) \
    `cat /dev/null $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL7LINKER) $(SHL7LINKFLAGS) $(SHL7VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(SHL7STDSHL) $(STDSHL7) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_7.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL7RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL7NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL7TARGETN)
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @echo $(SHL7LINKER) $(SHL7LINKFLAGS) $(SHL7SONAME) $(LINKFLAGSSHL) $(SHL7VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL7LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(SHL7STDSHL) $(STDSHL7) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_7.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_7.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL7NOCHECK)"==""
    -$(RM) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
    $(RENAME) $(SHL7TARGETN) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL7TARGETN:b)
    cd $(LB) && ln -s $(SHL7TARGETN:f) $(SHL7TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL7TARGET)

runtest_$(SHL7TARGET) : $(SHL7TARGETN)
    testshl $(SHL7TARGETN) sce$/$(SHL7TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL7TARGETN)"!=""

# unroll begin

.IF "$(SHL8TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL8STDLIBS=
.ENDIF

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
SHL8DEF*=$(MISC)$/$(SHL8TARGET).def
.ENDIF			# "$(SHL8USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL8VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL8VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL8VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL8VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL8VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL8VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL8VERSIONOBJ:b).h/ > $@

.INIT : $(SHL8VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL8IMPLIB)" == ""
SHL8IMPLIB=i$(TARGET)_t8
.ENDIF			# "$(SHL8IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_8IMPLIB=-implib:$(LB)$/$(SHL8IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL8IMPLIBN=$(LB)$/$(SHL8IMPLIB).lib
ALLTAR : $(SHL8IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_8IMPLIB_DEPS=$(LB)$/$(SHL8IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL8DEF=$(SHL8DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL8DEF=
SHL8DEPN+:=$(SHL8DEPNU)

.IF "$(SHL8VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
.ENDIF			# "$(SHL8VERSIONMAP)"==""

.IF "$(USE_SHL8VERSIONMAP)"!=""

.IF "$(SHL8FILTERFILE)"!=""
.IF "$(SHL8VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL8INTERFACE) -f $(SHL8FILTERFILE) -m $@

.ELSE			# "$(SHL8FILTERFILE)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) :
    @echo -----------------------------
    @echo SHL8FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL8FILTERFILE)"!=""
.ELSE			# "$(USE_SHL8VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL8VERSIONMAP)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8VERSIONMAP:b)_$(SHL8TARGET)$(SHL8VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL8VERSIONMAP): $(SHL8OBJS) $(SHL8LIBS)
.ENDIF

$(USE_SHL8VERSIONMAP): $(SHL8VERSIONMAP)
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
    tr -d "\015" < $(SHL8VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL8VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL8OBJS)"!=""
    -echo $(foreach,i,$(SHL8OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL8LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL8LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL8VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL8VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL8SONAME=$(SONAME_SWITCH)$(SHL8TARGETN:f)
.ELSE
SHL8SONAME=\"$(SONAME_SWITCH)$(SHL8TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL8RES)"!=""
SHL8ALLRES+=$(SHL8RES)
SHL8LINKRES*=$(MISC)$/$(SHL8TARGET).res
SHL8LINKRESO*=$(MISC)$/$(SHL8TARGET)_res.o
.ENDIF			# "$(SHL8RES)"!=""

.IF "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""
SHL8DEFAULTRES*=$(MISC)$/$(SHL8TARGET)_def.res
SHL8ALLRES+=$(SHL8DEFAULTRES)
SHL8LINKRES*=$(MISC)$/$(SHL8TARGET).res
SHL8LINKRESO*=$(MISC)$/$(SHL8TARGET)_res.o
.ENDIF			# "$(SHL8DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL8TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL8TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL8LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL8TARGETN:b)_linkinc.ls
$(SHL8TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL8USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL8LINKLIST=$(MISC)$/$(SHL8TARGET)_link.lst
$(MISC)$/$(SHL8TARGET)_link.lst : $(SHL8LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL8LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL8USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL8TARGETN) : \
                    $(SHL8OBJS)\
                    $(SHL8LIBS)\
                    $(USE_8IMPLIB_DEPS)\
                    $(USE_SHL8DEF)\
                    $(USE_SHL8VERSIONMAP)\
                    $(SHL8RES)\
                    $(SHL8DEPN) \
                    $(SHL8LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL8TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL8DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL8DEFAULTRES:b).rc
.IF "$(SHL8ICON)" != ""
    @echo 1 ICON $(SHL8ICON) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL8ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ELSE			# "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(SHL8ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL8TARGET)$(DLLPOST) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL8TARGET:b) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(SHL8DEFAULTRES)"!=""
.IF "$(SHL8ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL8ALLRES:s/res /res+/) $(SHL8LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL8ALLRES) > $(SHL8LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL8LINKRES) $(SHL8LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL8ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB8NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL8DEF) \
        --dllname $(SHL8TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_8.cmd
.ELSE			# "$(DEFLIB8NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL8DEF) \
        --dllname $(SHL8TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL8OBJS) $(SHL8LINKRESO) \
        `$(TYPE) /dev/null $(SHL8LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_8.cmd
.ENDIF			# "$(DEFLIB8NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) $(SHL8OBJS) $(SHL8LINKRESO) \
        `$(TYPE) /dev/null $(SHL8LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL8STDLIBS) $(SHL8STDSHL) $(STDSHL8) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_8.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL8USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL8LINKER) @$(mktmp \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL8STACK) $(SHL8BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL8DEF) \
        $(USE_8IMPLIB) \
        $(STDOBJ) \
        $(SHL8VERSIONOBJ) $(SHL8OBJS) \
        $(SHL8LIBS) \
        $(SHL8STDLIBS) \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL8LINKER) @$(mktmp	$(SHL8LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        $(SHL8STACK) -out:$(SHL8TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL8IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ) \
        $(SHL8LIBS)                         \
        $(SHL8STDLIBS)                      \
        $(SHL8STDSHL) $(STDSHL8)                           \
        $(SHL8LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL8USE_EXPORTS)"!="name"
    $(SHL8LINKER) @$(mktmp	$(SHL8LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL8BASEX)		\
        $(SHL8STACK) -out:$(SHL8TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_8IMPLIB) \
        $(STDOBJ)							\
        $(SHL8OBJS) $(SHL8VERSIONOBJ))   \
        @$(MISC)$/$(SHL8TARGET)_link.lst \
        @$(mktmp $(SHL8STDLIBS)                      \
        $(SHL8STDSHL) $(STDSHL8)                           \
        $(SHL8LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL8USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL8TARGET).lnk
        -$(RM) $(MISC)$/$(SHL8TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL8LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL8BASEX) \
        $(SHL8STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL8IMPLIB).exp \
        $(STDOBJ) \
        $(SHL8OBJS) \
        $(SHL8STDLIBS) \
        $(SHL8STDSHL) $(STDSHL8) \
        $(SHL8LINKRES) \
        ) >> $(MISC)$/$(SHL8TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL8TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL8TARGET).lnk
        $(SHL8LINKER) @$(MISC)$/$(SHL8TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @echo $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) \
    `cat /dev/null $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL8LINKER) $(SHL8LINKFLAGS) $(SHL8VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(SHL8STDSHL) $(STDSHL8) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_8.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL8RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL8NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL8TARGETN)
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @echo $(SHL8LINKER) $(SHL8LINKFLAGS) $(SHL8SONAME) $(LINKFLAGSSHL) $(SHL8VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL8LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(SHL8STDSHL) $(STDSHL8) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_8.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_8.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL8NOCHECK)"==""
    -$(RM) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
    $(RENAME) $(SHL8TARGETN) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL8TARGETN:b)
    cd $(LB) && ln -s $(SHL8TARGETN:f) $(SHL8TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL8TARGET)

runtest_$(SHL8TARGET) : $(SHL8TARGETN)
    testshl $(SHL8TARGETN) sce$/$(SHL8TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL8TARGETN)"!=""

# unroll begin

.IF "$(SHL9TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL9STDLIBS=
.ENDIF

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
SHL9DEF*=$(MISC)$/$(SHL9TARGET).def
.ENDIF			# "$(SHL9USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL9VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL9VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL9VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL9VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL9VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL9VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL9VERSIONOBJ:b).h/ > $@

.INIT : $(SHL9VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL9IMPLIB)" == ""
SHL9IMPLIB=i$(TARGET)_t9
.ENDIF			# "$(SHL9IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_9IMPLIB=-implib:$(LB)$/$(SHL9IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL9IMPLIBN=$(LB)$/$(SHL9IMPLIB).lib
ALLTAR : $(SHL9IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_9IMPLIB_DEPS=$(LB)$/$(SHL9IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL9DEF=$(SHL9DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL9DEF=
SHL9DEPN+:=$(SHL9DEPNU)

.IF "$(SHL9VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
.ENDIF			# "$(SHL9VERSIONMAP)"==""

.IF "$(USE_SHL9VERSIONMAP)"!=""

.IF "$(SHL9FILTERFILE)"!=""
.IF "$(SHL9VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL9INTERFACE) -f $(SHL9FILTERFILE) -m $@

.ELSE			# "$(SHL9FILTERFILE)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) :
    @echo -----------------------------
    @echo SHL9FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL9FILTERFILE)"!=""
.ELSE			# "$(USE_SHL9VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL9VERSIONMAP)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9VERSIONMAP:b)_$(SHL9TARGET)$(SHL9VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL9VERSIONMAP): $(SHL9OBJS) $(SHL9LIBS)
.ENDIF

$(USE_SHL9VERSIONMAP): $(SHL9VERSIONMAP)
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
    tr -d "\015" < $(SHL9VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL9VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL9OBJS)"!=""
    -echo $(foreach,i,$(SHL9OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL9LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL9LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL9VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL9VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL9SONAME=$(SONAME_SWITCH)$(SHL9TARGETN:f)
.ELSE
SHL9SONAME=\"$(SONAME_SWITCH)$(SHL9TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL9RES)"!=""
SHL9ALLRES+=$(SHL9RES)
SHL9LINKRES*=$(MISC)$/$(SHL9TARGET).res
SHL9LINKRESO*=$(MISC)$/$(SHL9TARGET)_res.o
.ENDIF			# "$(SHL9RES)"!=""

.IF "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""
SHL9DEFAULTRES*=$(MISC)$/$(SHL9TARGET)_def.res
SHL9ALLRES+=$(SHL9DEFAULTRES)
SHL9LINKRES*=$(MISC)$/$(SHL9TARGET).res
SHL9LINKRESO*=$(MISC)$/$(SHL9TARGET)_res.o
.ENDIF			# "$(SHL9DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL9TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL9TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL9LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL9TARGETN:b)_linkinc.ls
$(SHL9TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL9USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL9LINKLIST=$(MISC)$/$(SHL9TARGET)_link.lst
$(MISC)$/$(SHL9TARGET)_link.lst : $(SHL9LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL9LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL9USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL9TARGETN) : \
                    $(SHL9OBJS)\
                    $(SHL9LIBS)\
                    $(USE_9IMPLIB_DEPS)\
                    $(USE_SHL9DEF)\
                    $(USE_SHL9VERSIONMAP)\
                    $(SHL9RES)\
                    $(SHL9DEPN) \
                    $(SHL9LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL9TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL9DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL9DEFAULTRES:b).rc
.IF "$(SHL9ICON)" != ""
    @echo 1 ICON $(SHL9ICON) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL9ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ELSE			# "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(SHL9ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL9TARGET)$(DLLPOST) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL9TARGET:b) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(SHL9DEFAULTRES)"!=""
.IF "$(SHL9ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL9ALLRES:s/res /res+/) $(SHL9LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL9ALLRES) > $(SHL9LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL9LINKRES) $(SHL9LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL9ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB9NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL9DEF) \
        --dllname $(SHL9TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_9.cmd
.ELSE			# "$(DEFLIB9NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL9DEF) \
        --dllname $(SHL9TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL9OBJS) $(SHL9LINKRESO) \
        `$(TYPE) /dev/null $(SHL9LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_9.cmd
.ENDIF			# "$(DEFLIB9NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) $(SHL9OBJS) $(SHL9LINKRESO) \
        `$(TYPE) /dev/null $(SHL9LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL9STDLIBS) $(SHL9STDSHL) $(STDSHL9) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_9.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL9USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL9LINKER) @$(mktmp \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL9STACK) $(SHL9BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL9DEF) \
        $(USE_9IMPLIB) \
        $(STDOBJ) \
        $(SHL9VERSIONOBJ) $(SHL9OBJS) \
        $(SHL9LIBS) \
        $(SHL9STDLIBS) \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL9LINKER) @$(mktmp	$(SHL9LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        $(SHL9STACK) -out:$(SHL9TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL9IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ) \
        $(SHL9LIBS)                         \
        $(SHL9STDLIBS)                      \
        $(SHL9STDSHL) $(STDSHL9)                           \
        $(SHL9LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL9USE_EXPORTS)"!="name"
    $(SHL9LINKER) @$(mktmp	$(SHL9LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL9BASEX)		\
        $(SHL9STACK) -out:$(SHL9TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_9IMPLIB) \
        $(STDOBJ)							\
        $(SHL9OBJS) $(SHL9VERSIONOBJ))   \
        @$(MISC)$/$(SHL9TARGET)_link.lst \
        @$(mktmp $(SHL9STDLIBS)                      \
        $(SHL9STDSHL) $(STDSHL9)                           \
        $(SHL9LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL9USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL9TARGET).lnk
        -$(RM) $(MISC)$/$(SHL9TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL9LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL9BASEX) \
        $(SHL9STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL9IMPLIB).exp \
        $(STDOBJ) \
        $(SHL9OBJS) \
        $(SHL9STDLIBS) \
        $(SHL9STDSHL) $(STDSHL9) \
        $(SHL9LINKRES) \
        ) >> $(MISC)$/$(SHL9TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL9TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL9TARGET).lnk
        $(SHL9LINKER) @$(MISC)$/$(SHL9TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @echo $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) \
    `cat /dev/null $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL9LINKER) $(SHL9LINKFLAGS) $(SHL9VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(SHL9STDSHL) $(STDSHL9) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_9.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL9RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL9NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL9TARGETN)
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @echo $(SHL9LINKER) $(SHL9LINKFLAGS) $(SHL9SONAME) $(LINKFLAGSSHL) $(SHL9VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL9LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(SHL9STDSHL) $(STDSHL9) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_9.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_9.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL9NOCHECK)"==""
    -$(RM) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
    $(RENAME) $(SHL9TARGETN) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL9TARGETN:b)
    cd $(LB) && ln -s $(SHL9TARGETN:f) $(SHL9TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL9TARGET)

runtest_$(SHL9TARGET) : $(SHL9TARGETN)
    testshl $(SHL9TARGETN) sce$/$(SHL9TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL9TARGETN)"!=""

# unroll begin

.IF "$(SHL10TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL10STDLIBS=
.ENDIF

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
SHL10DEF*=$(MISC)$/$(SHL10TARGET).def
.ENDIF			# "$(SHL10USE_EXPORTS)"==""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL10VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
USE_VERSIONH:=$(INCCOM)$/$(SHL10VERSIONOBJ:b).h
.IF "$(GUI)" == "UNX"
SHL10VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL10VERSIONOBJDEP:=$(VERSIONOBJ:d){$(subst,$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL10VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/$(SHL10VERSIONOBJ:b).h
#    $(COPY) $(SOLARENV)$/src$/version.c $@
    $(TYPE) $(SOLARENV)$/src$/version.c | $(SED) s/_version.h/$(SHL10VERSIONOBJ:b).h/ > $@

.INIT : $(SHL10VERSIONOBJDEP)
.ENDIF			# "$(VERSIONOBJ)"!=""

.IF "$(GUI)" != "UNX"
.IF "$(GUI)" == "WNT"
.IF "$(SHL10IMPLIB)" == ""
SHL10IMPLIB=i$(TARGET)_t10
.ENDIF			# "$(SHL10IMPLIB)" == ""
.IF "$(COM)" != "GCC"
USE_10IMPLIB=-implib:$(LB)$/$(SHL10IMPLIB).lib
.ENDIF			# "$(COM)" != "GCC"
SHL10IMPLIBN=$(LB)$/$(SHL10IMPLIB).lib
ALLTAR : $(SHL10IMPLIBN)

.IF "$(USE_DEFFILE)"==""
USE_10IMPLIB_DEPS=$(LB)$/$(SHL10IMPLIB).lib
.ENDIF			# "$(USE_DEFFILE)"==""
.ENDIF			# "$(GUI)" == "WNT"
USE_SHL10DEF=$(SHL10DEF)
.ELSE			# "$(GUI)" != "UNX"
USE_SHL10DEF=
SHL10DEPN+:=$(SHL10DEPNU)

.IF "$(SHL10VERSIONMAP)"==""
# to activate vmaps remove "#"
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
.ENDIF			# "$(SHL10VERSIONMAP)"==""

.IF "$(USE_SHL10VERSIONMAP)"!=""

.IF "$(SHL10FILTERFILE)"!=""
.IF "$(SHL10VERSIONMAP)"!=""
#eine von beiden ist zuviel
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) .PHONY:
    @echo -----------------------------
    @echo you should only use versionmap OR exportfile
    @echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(OS)"!="IRIX"
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
    $(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL10INTERFACE) -f $(SHL10FILTERFILE) -m $@

.ELSE			# "$(SHL10FILTERFILE)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) :
    @echo -----------------------------
    @echo SHL10FILTERFILE not set!
    @echo -----------------------------
    @$(TOUCH) $@
    @echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL10FILTERFILE)"!=""
.ELSE			# "$(USE_SHL10VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL10VERSIONMAP)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10VERSIONMAP:b)_$(SHL10TARGET)$(SHL10VERSIONMAP:e)
.IF "$(OS)"!="IRIX"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF

.IF "$(OS)"=="MACOSX"
$(USE_SHL10VERSIONMAP): $(SHL10OBJS) $(SHL10LIBS)
.ENDIF

$(USE_SHL10VERSIONMAP): $(SHL10VERSIONMAP)
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
    tr -d "\015" < $(SHL10VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    tr -d "\015" < $(SHL10VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @chmod a+w $@
# Mac OS X post-processing generate an exported symbols list from the generated map file
# for details on exported symbols list see man ld on Mac OS X
.IF "$(OS)"=="MACOSX"
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep -v "\*\|?" > $@.exported-symbols
    -cat $@ | $(AWK) -f $(SOLARENV)$/bin$/unxmap-to-macosx-explist.awk | grep "\*\|?" > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL10OBJS)"!=""
    -echo $(foreach,i,$(SHL10OBJS:s/.obj/.o/) $i) | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL10LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL10LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gx | $(SOLARENV)$/bin$/addsym-macosx.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(OS)"=="MACOSX"
.ENDIF			# "$(SHL10VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL10VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL10SONAME=$(SONAME_SWITCH)$(SHL10TARGETN:f)
.ELSE
SHL10SONAME=\"$(SONAME_SWITCH)$(SHL10TARGETN:f)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

.IF "$(SHL10RES)"!=""
SHL10ALLRES+=$(SHL10RES)
SHL10LINKRES*=$(MISC)$/$(SHL10TARGET).res
SHL10LINKRESO*=$(MISC)$/$(SHL10TARGET)_res.o
.ENDIF			# "$(SHL10RES)"!=""

.IF "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""
SHL10DEFAULTRES*=$(MISC)$/$(SHL10TARGET)_def.res
SHL10ALLRES+=$(SHL10DEFAULTRES)
SHL10LINKRES*=$(MISC)$/$(SHL10TARGET).res
SHL10LINKRESO*=$(MISC)$/$(SHL10TARGET)_res.o
.ENDIF			# "$(SHL10DEFAULTRES)$(use_shl_versions)"!=""

#.IF "$(SHL10TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL10TARGET)_linkinc.ls .PHONY:
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL10LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL10TARGETN:b)_linkinc.ls
$(SHL10TARGETN) : $(LINKINCTARGETS)

.ELSE
.IF "$(SHL10USE_EXPORTS)"=="name"
.IF "$(GUI)"=="WNT"
SHL10LINKLIST=$(MISC)$/$(SHL10TARGET)_link.lst
$(MISC)$/$(SHL10TARGET)_link.lst : $(SHL10LIBS)
    @@-$(RM) $@
    $(SED) -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL10LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF
.ENDIF			# "$(SHL10USE_EXPORTS)"=="name"

$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

$(SHL10TARGETN) : \
                    $(SHL10OBJS)\
                    $(SHL10LIBS)\
                    $(USE_10IMPLIB_DEPS)\
                    $(USE_SHL10DEF)\
                    $(USE_SHL10VERSIONMAP)\
                    $(SHL10RES)\
                    $(SHL10DEPN) \
                    $(SHL10LINKLIST)
    @echo ------------------------------
    @echo Making: $(SHL10TARGETN)
.IF "$(GUI)" == "WNT"
.IF "$(SHL10DEFAULTRES)"!=""
    @@-$(RM) $(MISC)$/$(SHL10DEFAULTRES:b).rc
.IF "$(SHL10ICON)" != ""
    @echo 1 ICON $(SHL10ICON) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#include $(EMQ)"$(SHL10ADD_VERINFO)$(EMQ)" >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ELSE			# "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#define ADDITIONAL_VERINFO1 >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO2 >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ADDITIONAL_VERINFO3 >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(SHL10ADD_VERINFO)"!=""
    @echo $(EMQ)#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define ORG_NAME	$(SHL10TARGET)$(DLLPOST) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#define INTERNAL_NAME $(SHL10TARGET:b) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @echo $(EMQ)#include $(EMQ)"shlinfo.rc$(EMQ)" >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(SHL10DEFAULTRES)"!=""
.IF "$(SHL10ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /b $(SHL10ALLRES:s/res /res+/) $(SHL10LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(TYPE) $(SHL10ALLRES) > $(SHL10LINKRES)
.IF "$(COM)"=="GCC"
    windres $(SHL10LINKRES) $(SHL10LINKRESO)
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL10ALLRES)"!=""
.IF "$(COM)"=="GCC"	# always have to call dlltool explicitly as ld cannot handle # comment in .def
.IF "$(DEFLIB10NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL10DEF) \
        --dllname $(SHL10TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o > $(MISC)$/$(TARGET).$(@:b)_10.cmd
.ELSE			# "$(DEFLIB10NAME)"!=""	# do not have to include objs
    @echo dlltool --input-def $(SHL10DEF) \
        --dllname $(SHL10TARGET)$(DLLPOST) \
        --kill-at \
        --output-exp $(MISC)$/$(@:b)_exp.o \
        $(STDOBJ) $(SHL10OBJS) $(SHL10LINKRESO) \
        `$(TYPE) /dev/null $(SHL10LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g`  > $(MISC)$/$(TARGET).$(@:b)_10.cmd
.ENDIF			# "$(DEFLIB10NAME)"!=""
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) $(SHL10OBJS) $(SHL10LINKRESO) \
        `$(TYPE) /dev/null $(SHL10LIBS) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        -Wl,--exclude-libs,ALL $(SHL10STDLIBS) $(SHL10STDSHL) $(STDSHL10) \
        $(MISC)$/$(@:b)_exp.o \
        -Wl,-Map,$(MISC)$/$(@:b).map >> $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @$(TYPE)  $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_10.cmd
.ELSE
.IF "$(linkinc)"==""
.IF "$(SHL10USE_EXPORTS)"!="name"
.IF "$(USE_DEFFILE)"!=""
    $(SHL10LINKER) @$(mktmp \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) \
        $(SHL10STACK) $(SHL10BASEX)	\
        -out:$@ \
        -map:$(MISC)$/$(@:b).map \
        -def:$(SHL10DEF) \
        $(USE_10IMPLIB) \
        $(STDOBJ) \
        $(SHL10VERSIONOBJ) $(SHL10OBJS) \
        $(SHL10LIBS) \
        $(SHL10STDLIBS) \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ELSE			# "$(USE_DEFFILE)"!=""
    $(SHL10LINKER) @$(mktmp	$(SHL10LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        $(SHL10STACK) -out:$(SHL10TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(LB)$/$(SHL10IMPLIB).exp				\
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ) \
        $(SHL10LIBS)                         \
        $(SHL10STDLIBS)                      \
        $(SHL10STDSHL) $(STDSHL10)                           \
        $(SHL10LINKRES) \
    ) $(LINKOUTPUTFILTER)
# double check if target was really written... still making sense?
    @@$(LS) $@
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(SHL10USE_EXPORTS)"!="name"
    $(SHL10LINKER) @$(mktmp	$(SHL10LINKFLAGS)			\
        $(LINKFLAGSSHL) $(SHL10BASEX)		\
        $(SHL10STACK) -out:$(SHL10TARGETN)	\
        -map:$(MISC)$/$(@:B).map				\
        $(USE_10IMPLIB) \
        $(STDOBJ)							\
        $(SHL10OBJS) $(SHL10VERSIONOBJ))   \
        @$(MISC)$/$(SHL10TARGET)_link.lst \
        @$(mktmp $(SHL10STDLIBS)                      \
        $(SHL10STDSHL) $(STDSHL10)                           \
        $(SHL10LINKRES) \
    )
    @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
    $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
    $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
    $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(SHL10USE_EXPORTS)"!="name"
.ELSE			# "$(linkinc)"==""
        -$(RM) del $(MISC)$/$(SHL10TARGET).lnk
        -$(RM) $(MISC)$/$(SHL10TARGET).lst
        $(TYPE) $(mktmp \
        $(SHL10LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL10BASEX) \
        $(SHL10STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL10IMPLIB).exp \
        $(STDOBJ) \
        $(SHL10OBJS) \
        $(SHL10STDLIBS) \
        $(SHL10STDSHL) $(STDSHL10) \
        $(SHL10LINKRES) \
        ) >> $(MISC)$/$(SHL10TARGET).lnk
        $(TYPE) $(MISC)$/$(SHL10TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL10TARGET).lnk
        $(SHL10LINKER) @$(MISC)$/$(SHL10TARGET).lnk
        @echo linking $@.manifest ...
.IF "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -manifest $(TRUSTED_MANIFEST_LOCATION)$/trustedinfo.manifest -out:$@.tmanifest$(EMQ) $(FI)
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.tmanifest -outputresource:$@$(EMQ);2 $(FI)
.ELSE
        $(IFEXIST) $@.manifest $(THEN) mt.exe -manifest $@.manifest -outputresource:$@$(EMQ);2 $(FI)
.ENDIF # "$(VISTA_MANIFEST)"!=""
        $(IFEXIST) $@.manifest $(THEN) $(RM:s/+//) $@.manifest $(FI)
        $(IFEXIST) $@.tmanifest $(THEN) $(RM:s/+//) $@.tmanifest $(FI)
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(@:b).list
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @echo $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) \
    `cat /dev/null $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @echo -n $(SHL10LINKER) $(SHL10LINKFLAGS) $(SHL10VERSIONMAPPARA) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(SHL10STDSHL) $(STDSHL10) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-dylib-link-list.pl \
        `cat $(MISC)$/$(TARGET).$(@:b)_10.cmd` \
        >> $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @$(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl \
        shl $(SHL10RPATH) $@
    @echo "Making: $@.jnilib"
    @macosx-create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL10NOCHECK)"==""
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL10TARGETN)
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @-$(RM) $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @echo $(SHL10LINKER) $(SHL10LINKFLAGS) $(SHL10SONAME) $(LINKFLAGSSHL) $(SHL10VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) -o $@ \
    `cat /dev/null $(SHL10LIBS) | tr -s " " "\n" | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(SHL10STDSHL) $(STDSHL10) $(LINKOUTPUT_FILTER) > $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @cat $(MISC)$/$(TARGET).$(@:b)_10.cmd
    @+source $(MISC)$/$(TARGET).$(@:b)_10.cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL10NOCHECK)"==""
    -$(RM) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
    $(RENAME) $(SHL10TARGETN) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
    $(SOLARENV)$/bin$/checkdll.sh -L$(LB) -L$(SOLARLIBDIR) $(EXTRALIBPATHS) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    $(RM) $(LB)$/$(SHL10TARGETN:b)
    cd $(LB) && ln -s $(SHL10TARGETN:f) $(SHL10TARGETN:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL10TARGET)

runtest_$(SHL10TARGET) : $(SHL10TARGETN)
    testshl $(SHL10TARGETN) sce$/$(SHL10TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL10TARGETN)"!=""

# unroll begin

.IF "$(SHL1IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB1DEPN+=$(SHL1LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL1TARGET=$(SHL1TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL1IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL1IMPLIBN) \
    -def:$(SHL1DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL1TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL2IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB2DEPN+=$(SHL2LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL2TARGET=$(SHL2TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL2IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL2IMPLIBN) \
    -def:$(SHL2DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL2TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL3IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB3DEPN+=$(SHL3LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL3TARGET=$(SHL3TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL3IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL3IMPLIBN) \
    -def:$(SHL3DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL3TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL4IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB4DEPN+=$(SHL4LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL4TARGET=$(SHL4TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL4IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL4IMPLIBN) \
    -def:$(SHL4DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL4TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL5IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB5DEPN+=$(SHL5LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL5TARGET=$(SHL5TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL5IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL5IMPLIBN) \
    -def:$(SHL5DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL5TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL6IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB6DEPN+=$(SHL6LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL6TARGET=$(SHL6TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL6IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL6IMPLIBN) \
    -def:$(SHL6DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL6TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL7IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB7DEPN+=$(SHL7LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL7TARGET=$(SHL7TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL7IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL7IMPLIBN) \
    -def:$(SHL7DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL7TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL8IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB8DEPN+=$(SHL8LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL8TARGET=$(SHL8TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL8IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL8IMPLIBN) \
    -def:$(SHL8DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL8TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL9IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB9DEPN+=$(SHL9LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL9TARGET=$(SHL9TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL9IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL9IMPLIBN) \
    -def:$(SHL9DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL9TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

# unroll begin

.IF "$(SHL10IMPLIBN)" != ""

.IF "$(UPDATER)"=="YES"
USELIB10DEPN+=$(SHL10LIBS)
.ELSE
.ENDIF

.IF "$(USE_DEFFILE)"!=""
USE_SHL10TARGET=$(SHL10TARGETN)
.ENDIF

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
    @echo ------------------------------
    @echo Making: $(SHL10IMPLIBN)
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    @echo no ImportLibs on mingw
    @-$(RM) $@
    @$(TOUCH) $@
.ELSE			# "$(COM)=="GCC"
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL10IMPLIBN) \
    -def:$(SHL10DEF) )
.ELSE			# "$(USE_DEFFILE)==""
    @echo build of $(SHL10TARGETN) creates $@
.ENDIF			# "$(USE_DEFFILE)==""
.ENDIF			# "$(COM)"=="GCC"
.ELSE
    @echo no ImportLibs on Mac and *ix
    @-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF

