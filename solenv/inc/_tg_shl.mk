# unroll begin

.IF "$(SHL1TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL1STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL1STDLIBS+=$(STATICLIB)
# Allow certain libraries to not link to libstatic*.dylib. This is only used
# by libraries that cannot be linked to other libraries.
.IF "$(NOSHAREDSTATICLIB)"==""
SHL1STDLIBS+=$(STATICLIB)
.ENDIF
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL1STDLIBS=
STDSHL=
.ELSE
SHL1ARCHIVES=
.ENDIF

SHL1DEF*=$(MISC)$/$(SHL1TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL1TARGET)"!=""
.IF "$(COMP1TYPELIST)"==""

#fallback
LOCAL1DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL1TARGET)))}.xml"))
.IF "$(LOCAL1DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC1)"==""
SHL1DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL1DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC1)"==""

.ENDIF          # "$(COMP1TYPELIST)"==""
.ENDIF			# "$(SHL1TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL1VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL1DEPN+=$(SHL1VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL1DEPN+=$(SHL1VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL1VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL1VERSIONMAP): \
                    $(SHL1OBJS)\
                    $(SHL1LIBS)\
                    $(SHL1FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL1INTERFACE) -f $(SHL1FILTERFILE) -m $@

.ELSE			# "$(SHL1FILTERFILE)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1TARGET).vmap
$(USE_SHL1VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL1FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL1FILTERFILE)"!=""
.ELSE			# "$(USE_SHL1VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL1VERSIONMAP)"!=""
USE_SHL1VERSIONMAP=$(MISC)$/$(SHL1VERSIONMAP:b)_$(SHL1TARGET)$(SHL1VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL1VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL1VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL1VERSIONMAP): $(SHL1VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL1VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL1VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL1VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL1VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL1SONAME=$(SONAME_SWITCH)$(SHL1TARGETN:b:b)
.ELSE
SHL1SONAME=\"$(SONAME_SWITCH)$(SHL1TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL1TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL1TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL1LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL1TARGETN:b)_linkinc.ls
$(SHL1TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL1DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL1DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL1ICON)" != ""
    @-+echo 1 ICON $(SHL1ICON) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL1TARGET)$(DLLPOST) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL1TARGET:b) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL1TARGET)$(DLLPOST) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL1TARGET:b) >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL1DEFAULTRES:b).rc
.ENDIF			# "$(SHL1DEFAULTRES)"!=""
.IF "$(SHL1ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL1ALLRES:s/res /res+/) $(SHL1LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL1ALLRES) > $(SHL1LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL1ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL1STDLIBS) $(STDSHL) $(STDSHL1) $(SHL1RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL1) \
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
        $(STDSHL) $(STDSHL1)                           \
        $(SHL1LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL1TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL1TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL1BASEX) \
        $(SHL1STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL1IMPLIB).exp \
        $(STDOBJ) \
        $(SHL1OBJS) \
        $(SHL1STDLIBS) \
        $(STDSHL) $(STDSHL1) \
        $(SHL1LINKRES) \
        ) >> $(MISC)$/$(SHL1TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL1TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL1TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL1TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL1TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL1STDLIBS)` \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(STDSHL) $(STDSHL1) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL1VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL1VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL1NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL1TARGETN)
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL1SONAME) $(LINKFLAGSSHL) $(SHL1VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL1OBJS:s/.obj/.o/) \
    $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL1LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL1STDLIBS) $(SHL1ARCHIVES) $(STDSHL) $(STDSHL1) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL1NOCHECK)"==""
    +-$(RM) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
    +$(RENAME) $(SHL1TARGETN) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL1TARGETN:d)check_$(SHL1TARGETN:f)
.ENDIF				# "$(SHL1NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL1TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL1TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL1TARGETN:f) $(SHL1TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL1TARGETN:f:b:b) $(SHL1TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL1OBJS) `cat /dev/null $(SHL1LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL1VERSIONOBJ) $(SHL1DESCRIPTIONOBJ)) $(SHL1STDLIBS) $(SHL1ARCHIVES) $(STDSHL) $(STDSHL1) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL1TARGET)

runtest_$(SHL1TARGET) : $(SHL1TARGETN)
    testshl $(SHL1TARGETN) sce$/$(SHL1TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL1TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL2TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL2STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL2STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL2STDLIBS=
STDSHL=
.ELSE
SHL2ARCHIVES=
.ENDIF

SHL2DEF*=$(MISC)$/$(SHL2TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL2TARGET)"!=""
.IF "$(COMP2TYPELIST)"==""

#fallback
LOCAL2DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL2TARGET)))}.xml"))
.IF "$(LOCAL2DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC2)"==""
SHL2DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL2DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC2)"==""

.ENDIF          # "$(COMP2TYPELIST)"==""
.ENDIF			# "$(SHL2TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL2VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL2DEPN+=$(SHL2VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL2DEPN+=$(SHL2VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL2VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL2VERSIONMAP): \
                    $(SHL2OBJS)\
                    $(SHL2LIBS)\
                    $(SHL2FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL2INTERFACE) -f $(SHL2FILTERFILE) -m $@

.ELSE			# "$(SHL2FILTERFILE)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2TARGET).vmap
$(USE_SHL2VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL2FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL2FILTERFILE)"!=""
.ELSE			# "$(USE_SHL2VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL2VERSIONMAP)"!=""
USE_SHL2VERSIONMAP=$(MISC)$/$(SHL2VERSIONMAP:b)_$(SHL2TARGET)$(SHL2VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL2VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL2VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL2VERSIONMAP): $(SHL2VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL2VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL2VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL2VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL2VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL2SONAME=$(SONAME_SWITCH)$(SHL2TARGETN:b:b)
.ELSE
SHL2SONAME=\"$(SONAME_SWITCH)$(SHL2TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL2TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL2TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL2LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL2TARGETN:b)_linkinc.ls
$(SHL2TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL2DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL2DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL2ICON)" != ""
    @-+echo 1 ICON $(SHL2ICON) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL2TARGET)$(DLLPOST) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL2TARGET:b) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL2TARGET)$(DLLPOST) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL2TARGET:b) >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL2DEFAULTRES:b).rc
.ENDIF			# "$(SHL2DEFAULTRES)"!=""
.IF "$(SHL2ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL2ALLRES:s/res /res+/) $(SHL2LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL2ALLRES) > $(SHL2LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL2ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL2STDLIBS) $(STDSHL) $(STDSHL2) $(SHL2RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL2) \
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
        $(STDSHL) $(STDSHL2)                           \
        $(SHL2LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL2TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL2TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL2BASEX) \
        $(SHL2STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL2IMPLIB).exp \
        $(STDOBJ) \
        $(SHL2OBJS) \
        $(SHL2STDLIBS) \
        $(STDSHL) $(STDSHL2) \
        $(SHL2LINKRES) \
        ) >> $(MISC)$/$(SHL2TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL2TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL2TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL2TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL2TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL2STDLIBS)` \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(STDSHL) $(STDSHL2) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL2VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL2VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL2NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL2TARGETN)
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL2SONAME) $(LINKFLAGSSHL) $(SHL2VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL2OBJS:s/.obj/.o/) \
    $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL2LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL2STDLIBS) $(SHL2ARCHIVES) $(STDSHL) $(STDSHL2) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL2NOCHECK)"==""
    +-$(RM) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
    +$(RENAME) $(SHL2TARGETN) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL2TARGETN:d)check_$(SHL2TARGETN:f)
.ENDIF				# "$(SHL2NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL2TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL2TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL2TARGETN:f) $(SHL2TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL2TARGETN:f:b:b) $(SHL2TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL2OBJS) `cat /dev/null $(SHL2LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL2VERSIONOBJ) $(SHL2DESCRIPTIONOBJ)) $(SHL2STDLIBS) $(SHL2ARCHIVES) $(STDSHL) $(STDSHL2) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL2TARGET)

runtest_$(SHL2TARGET) : $(SHL2TARGETN)
    testshl $(SHL2TARGETN) sce$/$(SHL2TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL2TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL3TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL3STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL3STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL3STDLIBS=
STDSHL=
.ELSE
SHL3ARCHIVES=
.ENDIF

SHL3DEF*=$(MISC)$/$(SHL3TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL3TARGET)"!=""
.IF "$(COMP3TYPELIST)"==""

#fallback
LOCAL3DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL3TARGET)))}.xml"))
.IF "$(LOCAL3DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC3)"==""
SHL3DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL3DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC3)"==""

.ENDIF          # "$(COMP3TYPELIST)"==""
.ENDIF			# "$(SHL3TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL3VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL3DEPN+=$(SHL3VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL3DEPN+=$(SHL3VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL3VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL3VERSIONMAP): \
                    $(SHL3OBJS)\
                    $(SHL3LIBS)\
                    $(SHL3FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL3INTERFACE) -f $(SHL3FILTERFILE) -m $@

.ELSE			# "$(SHL3FILTERFILE)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3TARGET).vmap
$(USE_SHL3VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL3FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL3FILTERFILE)"!=""
.ELSE			# "$(USE_SHL3VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL3VERSIONMAP)"!=""
USE_SHL3VERSIONMAP=$(MISC)$/$(SHL3VERSIONMAP:b)_$(SHL3TARGET)$(SHL3VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL3VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL3VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL3VERSIONMAP): $(SHL3VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL3VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL3VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL3VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL3VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL3SONAME=$(SONAME_SWITCH)$(SHL3TARGETN:b:b)
.ELSE
SHL3SONAME=\"$(SONAME_SWITCH)$(SHL3TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL3TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL3TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL3LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL3TARGETN:b)_linkinc.ls
$(SHL3TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL3DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL3DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL3ICON)" != ""
    @-+echo 1 ICON $(SHL3ICON) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL3TARGET)$(DLLPOST) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL3TARGET:b) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL3TARGET)$(DLLPOST) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL3TARGET:b) >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL3DEFAULTRES:b).rc
.ENDIF			# "$(SHL3DEFAULTRES)"!=""
.IF "$(SHL3ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL3ALLRES:s/res /res+/) $(SHL3LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL3ALLRES) > $(SHL3LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL3ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL3STDLIBS) $(STDSHL) $(STDSHL3) $(SHL3RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL3) \
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
        $(STDSHL) $(STDSHL3)                           \
        $(SHL3LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL3TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL3TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL3BASEX) \
        $(SHL3STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL3IMPLIB).exp \
        $(STDOBJ) \
        $(SHL3OBJS) \
        $(SHL3STDLIBS) \
        $(STDSHL) $(STDSHL3) \
        $(SHL3LINKRES) \
        ) >> $(MISC)$/$(SHL3TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL3TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL3TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL3TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL3TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL3STDLIBS)` \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(STDSHL) $(STDSHL3) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL3VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL3VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL3NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL3TARGETN)
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL3SONAME) $(LINKFLAGSSHL) $(SHL3VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL3OBJS:s/.obj/.o/) \
    $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL3LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL3STDLIBS) $(SHL3ARCHIVES) $(STDSHL) $(STDSHL3) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL3NOCHECK)"==""
    +-$(RM) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
    +$(RENAME) $(SHL3TARGETN) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL3TARGETN:d)check_$(SHL3TARGETN:f)
.ENDIF				# "$(SHL3NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL3TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL3TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL3TARGETN:f) $(SHL3TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL3TARGETN:f:b:b) $(SHL3TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL3OBJS) `cat /dev/null $(SHL3LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL3VERSIONOBJ) $(SHL3DESCRIPTIONOBJ)) $(SHL3STDLIBS) $(SHL3ARCHIVES) $(STDSHL) $(STDSHL3) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL3TARGET)

runtest_$(SHL3TARGET) : $(SHL3TARGETN)
    testshl $(SHL3TARGETN) sce$/$(SHL3TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL3TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL4TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL4STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL4STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL4STDLIBS=
STDSHL=
.ELSE
SHL4ARCHIVES=
.ENDIF

SHL4DEF*=$(MISC)$/$(SHL4TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL4TARGET)"!=""
.IF "$(COMP4TYPELIST)"==""

#fallback
LOCAL4DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL4TARGET)))}.xml"))
.IF "$(LOCAL4DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC4)"==""
SHL4DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL4DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC4)"==""

.ENDIF          # "$(COMP4TYPELIST)"==""
.ENDIF			# "$(SHL4TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL4VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL4DEPN+=$(SHL4VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL4DEPN+=$(SHL4VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL4VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL4VERSIONMAP): \
                    $(SHL4OBJS)\
                    $(SHL4LIBS)\
                    $(SHL4FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL4INTERFACE) -f $(SHL4FILTERFILE) -m $@

.ELSE			# "$(SHL4FILTERFILE)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4TARGET).vmap
$(USE_SHL4VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL4FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL4FILTERFILE)"!=""
.ELSE			# "$(USE_SHL4VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL4VERSIONMAP)"!=""
USE_SHL4VERSIONMAP=$(MISC)$/$(SHL4VERSIONMAP:b)_$(SHL4TARGET)$(SHL4VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL4VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL4VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL4VERSIONMAP): $(SHL4VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL4VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL4VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL4VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL4VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL4SONAME=$(SONAME_SWITCH)$(SHL4TARGETN:b:b)
.ELSE
SHL4SONAME=\"$(SONAME_SWITCH)$(SHL4TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL4TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL4TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL4LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL4TARGETN:b)_linkinc.ls
$(SHL4TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL4DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL4DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL4ICON)" != ""
    @-+echo 1 ICON $(SHL4ICON) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL4TARGET)$(DLLPOST) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL4TARGET:b) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL4TARGET)$(DLLPOST) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL4TARGET:b) >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL4DEFAULTRES:b).rc
.ENDIF			# "$(SHL4DEFAULTRES)"!=""
.IF "$(SHL4ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL4ALLRES:s/res /res+/) $(SHL4LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL4ALLRES) > $(SHL4LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL4ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL4STDLIBS) $(STDSHL) $(STDSHL4) $(SHL4RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL4) \
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
        $(STDSHL) $(STDSHL4)                           \
        $(SHL4LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL4TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL4TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL4BASEX) \
        $(SHL4STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL4IMPLIB).exp \
        $(STDOBJ) \
        $(SHL4OBJS) \
        $(SHL4STDLIBS) \
        $(STDSHL) $(STDSHL4) \
        $(SHL4LINKRES) \
        ) >> $(MISC)$/$(SHL4TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL4TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL4TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL4TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL4TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL4STDLIBS)` \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(STDSHL) $(STDSHL4) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL4VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL4VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL4NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL4TARGETN)
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL4SONAME) $(LINKFLAGSSHL) $(SHL4VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL4OBJS:s/.obj/.o/) \
    $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL4LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL4STDLIBS) $(SHL4ARCHIVES) $(STDSHL) $(STDSHL4) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL4NOCHECK)"==""
    +-$(RM) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
    +$(RENAME) $(SHL4TARGETN) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL4TARGETN:d)check_$(SHL4TARGETN:f)
.ENDIF				# "$(SHL4NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL4TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL4TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL4TARGETN:f) $(SHL4TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL4TARGETN:f:b:b) $(SHL4TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL4OBJS) `cat /dev/null $(SHL4LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL4VERSIONOBJ) $(SHL4DESCRIPTIONOBJ)) $(SHL4STDLIBS) $(SHL4ARCHIVES) $(STDSHL) $(STDSHL4) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL4TARGET)

runtest_$(SHL4TARGET) : $(SHL4TARGETN)
    testshl $(SHL4TARGETN) sce$/$(SHL4TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL4TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL5TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL5STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL5STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL5STDLIBS=
STDSHL=
.ELSE
SHL5ARCHIVES=
.ENDIF

SHL5DEF*=$(MISC)$/$(SHL5TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL5TARGET)"!=""
.IF "$(COMP5TYPELIST)"==""

#fallback
LOCAL5DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL5TARGET)))}.xml"))
.IF "$(LOCAL5DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC5)"==""
SHL5DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL5DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC5)"==""

.ENDIF          # "$(COMP5TYPELIST)"==""
.ENDIF			# "$(SHL5TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL5VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL5DEPN+=$(SHL5VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL5DEPN+=$(SHL5VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL5VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL5VERSIONMAP): \
                    $(SHL5OBJS)\
                    $(SHL5LIBS)\
                    $(SHL5FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL5INTERFACE) -f $(SHL5FILTERFILE) -m $@

.ELSE			# "$(SHL5FILTERFILE)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5TARGET).vmap
$(USE_SHL5VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL5FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL5FILTERFILE)"!=""
.ELSE			# "$(USE_SHL5VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL5VERSIONMAP)"!=""
USE_SHL5VERSIONMAP=$(MISC)$/$(SHL5VERSIONMAP:b)_$(SHL5TARGET)$(SHL5VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL5VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL5VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL5VERSIONMAP): $(SHL5VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL5VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL5VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL5VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL5VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL5SONAME=$(SONAME_SWITCH)$(SHL5TARGETN:b:b)
.ELSE
SHL5SONAME=\"$(SONAME_SWITCH)$(SHL5TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL5TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL5TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL5LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL5TARGETN:b)_linkinc.ls
$(SHL5TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL5DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL5DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL5ICON)" != ""
    @-+echo 1 ICON $(SHL5ICON) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL5TARGET)$(DLLPOST) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL5TARGET:b) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL5TARGET)$(DLLPOST) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL5TARGET:b) >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL5DEFAULTRES:b).rc
.ENDIF			# "$(SHL5DEFAULTRES)"!=""
.IF "$(SHL5ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL5ALLRES:s/res /res+/) $(SHL5LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL5ALLRES) > $(SHL5LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL5ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL5STDLIBS) $(STDSHL) $(STDSHL5) $(SHL5RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL5) \
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
        $(STDSHL) $(STDSHL5)                           \
        $(SHL5LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL5TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL5TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL5BASEX) \
        $(SHL5STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL5IMPLIB).exp \
        $(STDOBJ) \
        $(SHL5OBJS) \
        $(SHL5STDLIBS) \
        $(STDSHL) $(STDSHL5) \
        $(SHL5LINKRES) \
        ) >> $(MISC)$/$(SHL5TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL5TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL5TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL5TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL5TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL5STDLIBS)` \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(STDSHL) $(STDSHL5) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL5VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL5VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL5NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL5TARGETN)
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL5SONAME) $(LINKFLAGSSHL) $(SHL5VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL5OBJS:s/.obj/.o/) \
    $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL5LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL5STDLIBS) $(SHL5ARCHIVES) $(STDSHL) $(STDSHL5) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL5NOCHECK)"==""
    +-$(RM) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
    +$(RENAME) $(SHL5TARGETN) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL5TARGETN:d)check_$(SHL5TARGETN:f)
.ENDIF				# "$(SHL5NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL5TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL5TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL5TARGETN:f) $(SHL5TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL5TARGETN:f:b:b) $(SHL5TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL5OBJS) `cat /dev/null $(SHL5LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL5VERSIONOBJ) $(SHL5DESCRIPTIONOBJ)) $(SHL5STDLIBS) $(SHL5ARCHIVES) $(STDSHL) $(STDSHL5) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL5TARGET)

runtest_$(SHL5TARGET) : $(SHL5TARGETN)
    testshl $(SHL5TARGETN) sce$/$(SHL5TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL5TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL6TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL6STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL6STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL6STDLIBS=
STDSHL=
.ELSE
SHL6ARCHIVES=
.ENDIF

SHL6DEF*=$(MISC)$/$(SHL6TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL6TARGET)"!=""
.IF "$(COMP6TYPELIST)"==""

#fallback
LOCAL6DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL6TARGET)))}.xml"))
.IF "$(LOCAL6DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC6)"==""
SHL6DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL6DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC6)"==""

.ENDIF          # "$(COMP6TYPELIST)"==""
.ENDIF			# "$(SHL6TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL6VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL6DEPN+=$(SHL6VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL6DEPN+=$(SHL6VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL6VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL6VERSIONMAP): \
                    $(SHL6OBJS)\
                    $(SHL6LIBS)\
                    $(SHL6FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL6INTERFACE) -f $(SHL6FILTERFILE) -m $@

.ELSE			# "$(SHL6FILTERFILE)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6TARGET).vmap
$(USE_SHL6VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL6FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL6FILTERFILE)"!=""
.ELSE			# "$(USE_SHL6VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL6VERSIONMAP)"!=""
USE_SHL6VERSIONMAP=$(MISC)$/$(SHL6VERSIONMAP:b)_$(SHL6TARGET)$(SHL6VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL6VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL6VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL6VERSIONMAP): $(SHL6VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL6VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL6VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL6VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL6VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL6SONAME=$(SONAME_SWITCH)$(SHL6TARGETN:b:b)
.ELSE
SHL6SONAME=\"$(SONAME_SWITCH)$(SHL6TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL6TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL6TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL6LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL6TARGETN:b)_linkinc.ls
$(SHL6TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL6DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL6DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL6ICON)" != ""
    @-+echo 1 ICON $(SHL6ICON) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL6TARGET)$(DLLPOST) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL6TARGET:b) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL6TARGET)$(DLLPOST) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL6TARGET:b) >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL6DEFAULTRES:b).rc
.ENDIF			# "$(SHL6DEFAULTRES)"!=""
.IF "$(SHL6ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL6ALLRES:s/res /res+/) $(SHL6LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL6ALLRES) > $(SHL6LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL6ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL6STDLIBS) $(STDSHL) $(STDSHL6) $(SHL6RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL6) \
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
        $(STDSHL) $(STDSHL6)                           \
        $(SHL6LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL6TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL6TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL6BASEX) \
        $(SHL6STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL6IMPLIB).exp \
        $(STDOBJ) \
        $(SHL6OBJS) \
        $(SHL6STDLIBS) \
        $(STDSHL) $(STDSHL6) \
        $(SHL6LINKRES) \
        ) >> $(MISC)$/$(SHL6TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL6TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL6TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL6TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL6TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL6STDLIBS)` \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(STDSHL) $(STDSHL6) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL6VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL6VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL6NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL6TARGETN)
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL6SONAME) $(LINKFLAGSSHL) $(SHL6VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL6OBJS:s/.obj/.o/) \
    $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL6LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL6STDLIBS) $(SHL6ARCHIVES) $(STDSHL) $(STDSHL6) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL6NOCHECK)"==""
    +-$(RM) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
    +$(RENAME) $(SHL6TARGETN) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL6TARGETN:d)check_$(SHL6TARGETN:f)
.ENDIF				# "$(SHL6NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL6TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL6TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL6TARGETN:f) $(SHL6TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL6TARGETN:f:b:b) $(SHL6TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL6OBJS) `cat /dev/null $(SHL6LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL6VERSIONOBJ) $(SHL6DESCRIPTIONOBJ)) $(SHL6STDLIBS) $(SHL6ARCHIVES) $(STDSHL) $(STDSHL6) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL6TARGET)

runtest_$(SHL6TARGET) : $(SHL6TARGETN)
    testshl $(SHL6TARGETN) sce$/$(SHL6TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL6TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL7TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL7STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL7STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL7STDLIBS=
STDSHL=
.ELSE
SHL7ARCHIVES=
.ENDIF

SHL7DEF*=$(MISC)$/$(SHL7TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL7TARGET)"!=""
.IF "$(COMP7TYPELIST)"==""

#fallback
LOCAL7DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL7TARGET)))}.xml"))
.IF "$(LOCAL7DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC7)"==""
SHL7DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL7DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC7)"==""

.ENDIF          # "$(COMP7TYPELIST)"==""
.ENDIF			# "$(SHL7TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL7VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL7DEPN+=$(SHL7VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL7DEPN+=$(SHL7VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL7VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL7VERSIONMAP): \
                    $(SHL7OBJS)\
                    $(SHL7LIBS)\
                    $(SHL7FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL7INTERFACE) -f $(SHL7FILTERFILE) -m $@

.ELSE			# "$(SHL7FILTERFILE)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7TARGET).vmap
$(USE_SHL7VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL7FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL7FILTERFILE)"!=""
.ELSE			# "$(USE_SHL7VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL7VERSIONMAP)"!=""
USE_SHL7VERSIONMAP=$(MISC)$/$(SHL7VERSIONMAP:b)_$(SHL7TARGET)$(SHL7VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL7VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL7VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL7VERSIONMAP): $(SHL7VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL7VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL7VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL7VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL7VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL7SONAME=$(SONAME_SWITCH)$(SHL7TARGETN:b:b)
.ELSE
SHL7SONAME=\"$(SONAME_SWITCH)$(SHL7TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL7TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL7TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL7LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL7TARGETN:b)_linkinc.ls
$(SHL7TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL7DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL7DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL7ICON)" != ""
    @-+echo 1 ICON $(SHL7ICON) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL7TARGET)$(DLLPOST) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL7TARGET:b) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL7TARGET)$(DLLPOST) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL7TARGET:b) >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL7DEFAULTRES:b).rc
.ENDIF			# "$(SHL7DEFAULTRES)"!=""
.IF "$(SHL7ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL7ALLRES:s/res /res+/) $(SHL7LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL7ALLRES) > $(SHL7LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL7ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL7STDLIBS) $(STDSHL) $(STDSHL7) $(SHL7RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL7) \
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
        $(STDSHL) $(STDSHL7)                           \
        $(SHL7LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL7TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL7TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL7BASEX) \
        $(SHL7STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL7IMPLIB).exp \
        $(STDOBJ) \
        $(SHL7OBJS) \
        $(SHL7STDLIBS) \
        $(STDSHL) $(STDSHL7) \
        $(SHL7LINKRES) \
        ) >> $(MISC)$/$(SHL7TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL7TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL7TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL7TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL7TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL7STDLIBS)` \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(STDSHL) $(STDSHL7) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL7VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL7VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL7NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL7TARGETN)
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL7SONAME) $(LINKFLAGSSHL) $(SHL7VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL7OBJS:s/.obj/.o/) \
    $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL7LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL7STDLIBS) $(SHL7ARCHIVES) $(STDSHL) $(STDSHL7) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL7NOCHECK)"==""
    +-$(RM) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
    +$(RENAME) $(SHL7TARGETN) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL7TARGETN:d)check_$(SHL7TARGETN:f)
.ENDIF				# "$(SHL7NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL7TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL7TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL7TARGETN:f) $(SHL7TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL7TARGETN:f:b:b) $(SHL7TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL7OBJS) `cat /dev/null $(SHL7LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL7VERSIONOBJ) $(SHL7DESCRIPTIONOBJ)) $(SHL7STDLIBS) $(SHL7ARCHIVES) $(STDSHL) $(STDSHL7) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL7TARGET)

runtest_$(SHL7TARGET) : $(SHL7TARGETN)
    testshl $(SHL7TARGETN) sce$/$(SHL7TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL7TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL8TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL8STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL8STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL8STDLIBS=
STDSHL=
.ELSE
SHL8ARCHIVES=
.ENDIF

SHL8DEF*=$(MISC)$/$(SHL8TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL8TARGET)"!=""
.IF "$(COMP8TYPELIST)"==""

#fallback
LOCAL8DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL8TARGET)))}.xml"))
.IF "$(LOCAL8DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC8)"==""
SHL8DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL8DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC8)"==""

.ENDIF          # "$(COMP8TYPELIST)"==""
.ENDIF			# "$(SHL8TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL8VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL8DEPN+=$(SHL8VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL8DEPN+=$(SHL8VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL8VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL8VERSIONMAP): \
                    $(SHL8OBJS)\
                    $(SHL8LIBS)\
                    $(SHL8FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL8INTERFACE) -f $(SHL8FILTERFILE) -m $@

.ELSE			# "$(SHL8FILTERFILE)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8TARGET).vmap
$(USE_SHL8VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL8FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL8FILTERFILE)"!=""
.ELSE			# "$(USE_SHL8VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL8VERSIONMAP)"!=""
USE_SHL8VERSIONMAP=$(MISC)$/$(SHL8VERSIONMAP:b)_$(SHL8TARGET)$(SHL8VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL8VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL8VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL8VERSIONMAP): $(SHL8VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL8VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL8VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL8VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL8VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL8SONAME=$(SONAME_SWITCH)$(SHL8TARGETN:b:b)
.ELSE
SHL8SONAME=\"$(SONAME_SWITCH)$(SHL8TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL8TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL8TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL8LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL8TARGETN:b)_linkinc.ls
$(SHL8TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL8DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL8DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL8ICON)" != ""
    @-+echo 1 ICON $(SHL8ICON) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL8TARGET)$(DLLPOST) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL8TARGET:b) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL8TARGET)$(DLLPOST) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL8TARGET:b) >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL8DEFAULTRES:b).rc
.ENDIF			# "$(SHL8DEFAULTRES)"!=""
.IF "$(SHL8ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL8ALLRES:s/res /res+/) $(SHL8LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL8ALLRES) > $(SHL8LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL8ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL8STDLIBS) $(STDSHL) $(STDSHL8) $(SHL8RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL8) \
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
        $(STDSHL) $(STDSHL8)                           \
        $(SHL8LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL8TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL8TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL8BASEX) \
        $(SHL8STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL8IMPLIB).exp \
        $(STDOBJ) \
        $(SHL8OBJS) \
        $(SHL8STDLIBS) \
        $(STDSHL) $(STDSHL8) \
        $(SHL8LINKRES) \
        ) >> $(MISC)$/$(SHL8TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL8TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL8TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL8TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL8TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL8STDLIBS)` \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(STDSHL) $(STDSHL8) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL8VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL8VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL8NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL8TARGETN)
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL8SONAME) $(LINKFLAGSSHL) $(SHL8VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL8OBJS:s/.obj/.o/) \
    $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL8LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL8STDLIBS) $(SHL8ARCHIVES) $(STDSHL) $(STDSHL8) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL8NOCHECK)"==""
    +-$(RM) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
    +$(RENAME) $(SHL8TARGETN) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL8TARGETN:d)check_$(SHL8TARGETN:f)
.ENDIF				# "$(SHL8NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL8TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL8TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL8TARGETN:f) $(SHL8TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL8TARGETN:f:b:b) $(SHL8TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL8OBJS) `cat /dev/null $(SHL8LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL8VERSIONOBJ) $(SHL8DESCRIPTIONOBJ)) $(SHL8STDLIBS) $(SHL8ARCHIVES) $(STDSHL) $(STDSHL8) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL8TARGET)

runtest_$(SHL8TARGET) : $(SHL8TARGETN)
    testshl $(SHL8TARGETN) sce$/$(SHL8TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL8TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL9TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL9STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL9STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL9STDLIBS=
STDSHL=
.ELSE
SHL9ARCHIVES=
.ENDIF

SHL9DEF*=$(MISC)$/$(SHL9TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL9TARGET)"!=""
.IF "$(COMP9TYPELIST)"==""

#fallback
LOCAL9DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL9TARGET)))}.xml"))
.IF "$(LOCAL9DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC9)"==""
SHL9DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL9DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC9)"==""

.ENDIF          # "$(COMP9TYPELIST)"==""
.ENDIF			# "$(SHL9TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL9VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL9DEPN+=$(SHL9VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL9DEPN+=$(SHL9VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL9VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL9VERSIONMAP): \
                    $(SHL9OBJS)\
                    $(SHL9LIBS)\
                    $(SHL9FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL9INTERFACE) -f $(SHL9FILTERFILE) -m $@

.ELSE			# "$(SHL9FILTERFILE)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9TARGET).vmap
$(USE_SHL9VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL9FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL9FILTERFILE)"!=""
.ELSE			# "$(USE_SHL9VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL9VERSIONMAP)"!=""
USE_SHL9VERSIONMAP=$(MISC)$/$(SHL9VERSIONMAP:b)_$(SHL9TARGET)$(SHL9VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL9VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL9VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL9VERSIONMAP): $(SHL9VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL9VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL9VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL9VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL9VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL9SONAME=$(SONAME_SWITCH)$(SHL9TARGETN:b:b)
.ELSE
SHL9SONAME=\"$(SONAME_SWITCH)$(SHL9TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL9TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL9TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL9LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL9TARGETN:b)_linkinc.ls
$(SHL9TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL9DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL9DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL9ICON)" != ""
    @-+echo 1 ICON $(SHL9ICON) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL9TARGET)$(DLLPOST) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL9TARGET:b) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL9TARGET)$(DLLPOST) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL9TARGET:b) >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL9DEFAULTRES:b).rc
.ENDIF			# "$(SHL9DEFAULTRES)"!=""
.IF "$(SHL9ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL9ALLRES:s/res /res+/) $(SHL9LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL9ALLRES) > $(SHL9LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL9ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL9STDLIBS) $(STDSHL) $(STDSHL9) $(SHL9RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL9) \
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
        $(STDSHL) $(STDSHL9)                           \
        $(SHL9LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL9TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL9TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL9BASEX) \
        $(SHL9STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL9IMPLIB).exp \
        $(STDOBJ) \
        $(SHL9OBJS) \
        $(SHL9STDLIBS) \
        $(STDSHL) $(STDSHL9) \
        $(SHL9LINKRES) \
        ) >> $(MISC)$/$(SHL9TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL9TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL9TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL9TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL9TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL9STDLIBS)` \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(STDSHL) $(STDSHL9) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL9VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL9VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL9NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL9TARGETN)
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL9SONAME) $(LINKFLAGSSHL) $(SHL9VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL9OBJS:s/.obj/.o/) \
    $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL9LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL9STDLIBS) $(SHL9ARCHIVES) $(STDSHL) $(STDSHL9) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL9NOCHECK)"==""
    +-$(RM) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
    +$(RENAME) $(SHL9TARGETN) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL9TARGETN:d)check_$(SHL9TARGETN:f)
.ENDIF				# "$(SHL9NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL9TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL9TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL9TARGETN:f) $(SHL9TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL9TARGETN:f:b:b) $(SHL9TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL9OBJS) `cat /dev/null $(SHL9LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL9VERSIONOBJ) $(SHL9DESCRIPTIONOBJ)) $(SHL9STDLIBS) $(SHL9ARCHIVES) $(STDSHL) $(STDSHL9) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL9TARGET)

runtest_$(SHL9TARGET) : $(SHL9TARGETN)
    testshl $(SHL9TARGETN) sce$/$(SHL9TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL9TARGETN)"!=""

# Anweisungen fuer das Linken
# unroll begin

.IF "$(SHL10TARGETN)"!=""

.IF "$(OS)"=="AIX"
SHL10STDLIBS=
.ENDIF

# Link in static data members for template classes
.IF "$(OS)$(CVER)"=="MACOSXC295"
SHL10STDLIBS+=$(STATICLIB)
.ENDIF

.IF "$(SHLLINKARCONLY)" != ""
SHL10STDLIBS=
STDSHL=
.ELSE
SHL10ARCHIVES=
.ENDIF

SHL10DEF*=$(MISC)$/$(SHL10TARGET).def

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL10TARGET)"!=""
.IF "$(COMP10TYPELIST)"==""

#fallback
LOCAL10DESC:=$(subst,/,$/ $(shell $(FIND) . -name "{$(subst,$($(WINVERSIONNAMES)_MAJOR),* $(subst,$(UPD)$(DLLPOSTFIX), $(SHL10TARGET)))}.xml"))
.IF "$(LOCAL10DESC)"==""
$(MISC)$/%{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
.ELSE           # "$(LOCALDESC10)"==""
SHL10DESCRIPTIONOBJ*=$(SLO)$/$(LOCAL10DESC:b)$($(WINVERSIONNAMES)_MAJOR)_description.obj
.ENDIF          # "$(LOCALDESC10)"==""

.ENDIF          # "$(COMP10TYPELIST)"==""
.ENDIF			# "$(SHL10TARGET)"!="

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++    version object      ++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(VERSIONOBJ)"!=""
SHL10VERSIONOBJ:=$(VERSIONOBJ:d){$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}$(VERSIONOBJ:f)
.IF "$(UPDATER)"=="YES"
USE_VERSIONH:=$(INCCOM)$/_version.h
.ELSE			# "$(UPDATER)"=="YES"
.IF "$(GUI)" == "UNX"
SHL10DEPN+=$(SHL10VERSIONOBJ:s/.o/.obj/)
.ELSE           # "$(GUI)" == "UNX"
SHL10DEPN+=$(SHL10VERSIONOBJ)
.ENDIF          # "$(GUI)" == "UNX"
$(MISC)$/$(SHL10VERSIONOBJ:b).c : $(SOLARENV)$/src$/version.c $(INCCOM)$/_version.h
    +$(COPY) $(SOLARENV)$/src$/version.c $@

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
    @+echo -----------------------------
    @+echo you should only use versionmap OR exportfile
    @+echo -----------------------------
#	force_dmake_to_error

.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL10VERSIONMAP): \
                    $(SHL10OBJS)\
                    $(SHL10LIBS)\
                    $(SHL10FILTERFILE)
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
    +$(PERL) $(SOLARENV)$/bin$/mapgen.pl -d $@.dump -s $(SHL10INTERFACE) -f $(SHL10FILTERFILE) -m $@

.ELSE			# "$(SHL10FILTERFILE)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10TARGET).vmap
$(USE_SHL10VERSIONMAP) : 
    @+echo -----------------------------
    @+echo SHL10FILTERFILE not set!
    @+echo -----------------------------
    @$(TOUCH) $@
    @+echo dummy file to keep the dependencies for later use.
#	force_dmake_to_error
.ENDIF			# "$(SHL10FILTERFILE)"!=""
.ELSE			# "$(USE_SHL10VERSIONMAP)"!=""

#and now for the plain non-generic way...
.IF "$(SHL10VERSIONMAP)"!=""
USE_SHL10VERSIONMAP=$(MISC)$/$(SHL10VERSIONMAP:b)_$(SHL10TARGET)$(SHL10VERSIONMAP:e)
.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
SHL10VERSIONMAPPARA=$(LINKVERSIONMAPFLAG) $(USE_SHL10VERSIONMAP)
.ENDIF
.ENDIF

$(USE_SHL10VERSIONMAP): $(SHL10VERSIONMAP)
    @+-$(RM) -f $@ >& $(NULLDEV)
.IF "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL10VERSIONMAP) | $(AWK) -f $(SOLARENV)$/bin$/addsym.awk > $@
.ELSE           # "$(COMID)"=="gcc3"
    +tr -d "\015" < $(SHL10VERSIONMAP) > $@
.ENDIF          # "$(COMID)"=="gcc3"
    @+chmod a+w $@
    
.ENDIF			# "$(SHL10VERSIONMAP)"!=""
.ENDIF			# "$(USE_SHL10VERSIONMAP)"!=""
.ENDIF			# "$(GUI)" != "UNX"

.IF "$(UNIXVERSIONNAMES)"!=""
.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="IRIX"
SHL10SONAME=$(SONAME_SWITCH)$(SHL10TARGETN:b:b)
.ELSE
SHL10SONAME=\"$(SONAME_SWITCH)$(SHL10TARGETN:b:b)\"
.ENDIF
.ENDIF			# "$(GUI)"!="UNX"
.ENDIF			# "$(OS)"!="MACOSX"
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

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

#.IF "$(SHL10TARGETN)"!=""

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/$(SHL10TARGET)_linkinc.ls .PHONY:
    @+-$(RM) $@ >& $(NULLDEV)
    +sed -f $(COMMON_ENV_TOOLS)\chrel.sed $(foreach,i,$(SHL10LIBS) $(i:s/.lib/.lin/)) >> $@
.ENDIF

LINKINCTARGETS+=$(MISC)$/$(SHL10TARGETN:b)_linkinc.ls
$(SHL10TARGETN) : $(LINKINCTARGETS)

.ELSE
$(MISC)$/%linkinc.ls:
    echo . > $@
.ENDIF          # "$(linkinc)"!=""

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
.IF "$(GUI)" == "WNT"
.IF "$(UPDATER)"=="YES"
.IF "$(COM)"=="GCC"
            $(CXX) -c -o$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE
            $(CXX) -c -Fo$(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.obj -DWNT $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(UPDATER)"=="YES"
.IF "$(SHL10DEFAULTRES)"!=""
    @+-$(RM) $(MISC)$/$(SHL10DEFAULTRES:b).rc >& $(NULLDEV)
.IF "$(SHL10ICON)" != ""
    @-+echo 1 ICON $(SHL10ICON) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF
.IF "$(use_shl_versions)" != ""
.IF "$(USE_SHELL)"!="4nt"
    @-+echo \#define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo \#define ORG_NAME	$(SHL10TARGET)$(DLLPOST) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo \#define INTERNAL_NAME $(SHL10TARGET:b) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
     @-+echo \#include \"shlinfo.rc\" >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ELSE			# "$(USE_SHELL)"!="4nt"
    @-+echo #define VERVARIANT	$(BUILD) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo #define ORG_NAME	$(SHL10TARGET)$(DLLPOST) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
    @-+echo #define INTERNAL_NAME $(SHL10TARGET:b) >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
     @-+echo #include "shlinfo.rc" >> $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(use_shl_versions)" != ""
    $(RC) -DWIN32 -I$(SOLARTESDIR) $(INCLUDE) $(RCLINKFLAGS) $(MISC)$/$(SHL10DEFAULTRES:b).rc
.ENDIF			# "$(SHL10DEFAULTRES)"!=""
.IF "$(SHL10ALLRES)"!=""
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /b $(SHL10ALLRES:s/res /res+/) $(SHL10LINKRES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    +$(TYPE) $(SHL10ALLRES) > $(SHL10LINKRES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(SHL10ALLRES)"!=""
.IF "$(linkinc)"==""
.IF "$(USE_DEFFILE)"!=""
.IF "$(COM)"=="GCC"
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -o$@ \
        $(STDOBJ) $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ) | tr -d ï\r\nï > $(MISC)$/$(@:b).cmd
    @+$(TYPE) $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$/$(ROUT)\#g | tr -d ï\r\nï >> $(MISC)$/$(@:b).cmd
    @+echo  $(SHL10STDLIBS) $(STDSHL) $(STDSHL10) $(SHL10RES) >> $(MISC)$/$(@:b).cmd
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
        $(STDSHL) $(STDSHL10) \
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
        $(STDSHL) $(STDSHL10)                           \
        $(SHL10LINKRES) \
    )
.ENDIF			# "$(USE_DEFFILE)"!=""
.ELSE			# "$(linkinc)"==""
        +-$(RM) del $(MISC)$/$(SHL10TARGET).lnk
        +-$(RM) $(MISC)$/$(SHL10TARGET).lst
        +$(TYPE) $(mktmp \
        $(LINKFLAGS) \
        $(LINKFLAGSSHL) $(SHL10BASEX) \
        $(SHL10STACK) $(MAPFILE) \
        -out:$@ \
        $(LB)$/$(SHL10IMPLIB).exp \
        $(STDOBJ) \
        $(SHL10OBJS) \
        $(SHL10STDLIBS) \
        $(STDSHL) $(STDSHL10) \
        $(SHL10LINKRES) \
        ) >> $(MISC)$/$(SHL10TARGET).lnk
        +$(TYPE) $(MISC)$/$(SHL10TARGETN:b)_linkinc.ls  >> $(MISC)$/$(SHL10TARGET).lnk
        $(LINK) @$(MISC)$/$(SHL10TARGET).lnk
.ENDIF			# "$(linkinc)"==""
.ENDIF			# "$(GUI)" == "WNT"
.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
.IF "$(OS)"=="SOLARIS"
.IF "$(COM)"=="GCC"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ELSE		
        $(CC) -c -KPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF		
.ENDIF
.IF "$(OS)"=="MACOSX"
        $(CC) -c -dynamic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
    .IF "$(CVER)"=="C295"
        @echo "------------------------------"
        @echo "Updating static data member initializations"
        @+dmake -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
    .ENDIF
.ENDIF
.IF "$(OS)"=="LINUX" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
        $(CC) -c -fPIC -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
.ENDIF
.IF "$(OS)"=="IRIX"
        $(CC) -c -fpic -o $(SLO)$/{$(subst,$(UPD)$(DLLPOSTFIX),_dflt $(SHL10TARGET))}_version.o -DUNX $(ENVCDEFS) -I$(INCCOM) $(SOLARENV)$/src$/version.c
        @+if ( ! -e $(SOLARLIBDIR) ) mkdir $(SOLARLIBDIR)
        @+if ( ! -e $(SOLARLIBDIR)/so_locations ) touch $(SOLARLIBDIR)/so_locations
.ENDIF			# "$(OS)"=="IRIX"
.ENDIF
.IF "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).list
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ:s/.obj/.o/) \
    `cat /dev/null $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` | tr -s " " "\n" > $(MISC)$/$(@:b).list
    @+echo $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) -o $@ \
    `dylib-link-list $(PRJNAME) $(SOLARVERSION)$/$(INPATH)$/lib $(PRJ)$/$(INPATH)$/lib $(SHL10STDLIBS)` \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(STDSHL) $(STDSHL10) -filelist $(MISC)$/$(@:b).list $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
    .IF "$(CVER)"=="C295"
        # This is a hack as libstatic and libcppuhelper have a circular dependency
        .IF "$(PRJNAME)"=="cppuhelper"
        @echo "------------------------------"
        @echo "Rerunning static data member initializations"
        @+dmake -u -f $(SOLARENV)$/$(OUTPATH)$/inc/makefile.mk $(MFLAGS) $(CALLMACROS) "PRJ=$(PRJ)" "PRJNAME=$(PRJNAME)" "TARGET=$(TARGET)"
        .ENDIF
    .ENDIF
.IF "$(SHL10VERSIONMAP)"!=""
.IF "$(DEBUG)"==""
    @strip -i -r -u -S -s $(SHL10VERSIONMAP) $@
.ENDIF
.ENDIF
    @echo "Making: $@.jnilib"
    @create-bundle $@
.IF "$(UPDATER)"=="YES"
.IF "$(SHL10NOCHECK)"==""
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB) $(SHL10TARGETN)
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @+-$(RM) $(MISC)$/$(@:b).cmd
    @+echo $(LINK) $(LINKFLAGS) $(SHL10SONAME) $(LINKFLAGSSHL) $(SHL10VERSIONMAPPARA) -L$(PRJ)$/$(ROUT)$/lib $(SOLARLIB) $(STDSLO) $(SHL10OBJS:s/.obj/.o/) \
    $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ:s/.obj/.o/) -o $@ \
    `cat /dev/null $(SHL10LIBS) | tr -s " " "\n" | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
    $(SHL10STDLIBS) $(SHL10ARCHIVES) $(STDSHL) $(STDSHL10) $(LINKOUTPUT_FILTER) > $(MISC)$/$(@:b).cmd
    @cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.IF "$(UPDATER)"=="YES"
.IF "$(SHL10NOCHECK)"==""
    +-$(RM) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
    +$(RENAME) $(SHL10TARGETN) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
    +$(SOLARENV)$/bin$/checkdll.sh -L$(LB) $(SOLARLIB:s/2.6//) $(SHL10TARGETN:d)check_$(SHL10TARGETN:f)
.ENDIF				# "$(SHL10NOCHECK)"!=""
.ENDIF			# "$(UPDATER)"=="YES"
.ENDIF			# "$(OS)"=="MACOSX"
.IF "$(UNIXVERSIONNAMES)"!=""
    +$(RM) $(LB)$/$(SHL10TARGETN:b:b:b)
    +$(RM) $(LB)$/$(SHL10TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL10TARGETN:f) $(SHL10TARGETN:b:b)
    +cd $(LB) && ln -s $(SHL10TARGETN:f:b:b) $(SHL10TARGETN:b:b:b)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""
    @ls -l $@
.ENDIF			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="MAC"
    @+-$(RM) $@ $@.xSYM
    $(LINK) $(LINKFLAGS) $(LINKFLAGSSHL) $(foreach,i,$(shell $(UNIX2MACPATH) $(PRJ)$/$(ROUT)$/lib $(SOLARLIB:s/-L//)) -L"$i") $(shell $(UNIX2MACPATH) $(STDSLO) $(SHL10OBJS) `cat /dev/null $(SHL10LIBS) | sed s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` $(SHL10VERSIONOBJ) $(SHL10DESCRIPTIONOBJ)) $(SHL10STDLIBS) $(SHL10ARCHIVES) $(STDSHL) $(STDSHL10) $(LINKOUTPUT_FILTER) -o $(shell $(UNIX2MACPATH) $@)
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(TARGETTHREAD)"!="MT"
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - THREAD WARNING! - this library was linked single threaded 
    @+echo - and must not be used in any office installation!
    @+echo -
    @+echo ----------------------------------------------------------
.ENDIF			# "$(TARGETTHREAD)"!="MT"

.IF "$(TESTDIR)"!=""
.IF "$(NO_TESTS)"==""

ALLTAR : runtest_$(SHL10TARGET)

runtest_$(SHL10TARGET) : $(SHL10TARGETN)
    testshl $(SHL10TARGETN) sce$/$(SHL10TARGET).sce -msg -skip
.ENDIF			# "$(NO_TESTS)"==""
.ENDIF			# "$(TESTDIR)"!=""
.ENDIF			# "$(SHL10TARGETN)"!=""

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL1IMPLIBN) \
    -def:$(SHL1DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL1TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL1DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL1TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL2IMPLIBN) \
    -def:$(SHL2DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL2TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL2DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL2TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL3IMPLIBN) \
    -def:$(SHL3DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL3TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL3DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL3TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL4IMPLIBN) \
    -def:$(SHL4DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL4TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL4DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL4TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL5IMPLIBN) \
    -def:$(SHL5DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL5TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL5DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL5TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL6IMPLIBN) \
    -def:$(SHL6DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL6TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL6DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL6TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL7IMPLIBN) \
    -def:$(SHL7DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL7TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL7DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL7TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL8IMPLIBN) \
    -def:$(SHL8DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL8TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL8DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL8TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL9IMPLIBN) \
    -def:$(SHL9DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL9TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL9DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL9TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
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
# bei use_deffile implib von linker erstellt
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) @$(mktmp -out:$(SHL10IMPLIBN) \
    -def:$(SHL10DEF) )
.ELSE			# "$(GUI)" == "WNT"
    +$(TOUCH) $@
    +@echo build of $(SHL10TARGETN) creates $@
.ENDIF			# "$(GUI)" == "WNT"
.ELSE
.IF "$(GUI)" == "WIN" || "$(GUI)" == "OS2"
.IF "$(USE_DEFFILE)"==""
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL10DEF)
.ELSE
    $(IMPLIB) $(IMPLIBFLAGS) $@ $(SHL10TARGETN)
.ENDIF
.ELSE
    @echo no ImportLibs on Mac and *ix
    @+-$(RM) $@
    @$(TOUCH) $@
.ENDIF
.ENDIF
.ENDIF

# Anweisungen fuer das Linken
