
#*******************************************************************
#*/*****************************************************************/
#*
#*    $Workfile:   pstrules.mk  $
#*
#*    Beschreibung      rules which use variables defined
#*                      in makefile.mk
#*
#*    Ersterstellung    HJS 20.07.99
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:16 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/pstrules.mkv  $
#*
#*    Copyright (c) 1990-1999, STAR DIVISION
#*
#*******************************************************************

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	svxlight rules			+++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SVXLIGHTOBJFILES)"!=""

SVXLIGHTDEFS=-DSVX_LIGHT

$(OBJ)$/sxl_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/sxl_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF
.ENDIF

.ENDIF			# "$(SVXLIGHTOBJFILES)"!=""

.IF "$(SVXLIGHTSLOFILES)"!=""

$(SLO)$/sxl_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$/sxl_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF
.ENDIF

.ENDIF			# "$(SVXLIGHTSLOFILES)"!=""

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	second_build rules			+++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)OBJFILES)"!=""

#$(SECOND_BUILD)CDEFS=-DSVX_LIGHT

$(OBJ)$/$(SECOND_BUILD)_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF
.ENDIF

$(OBJ)$/$(SECOND_BUILD)_%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) -E $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.c)
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c
.ELSE
    $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)OBJFILES)"!=""

.IF "$($(SECOND_BUILD)SLOFILES)"!=""

$(SLO)$/$(SECOND_BUILD)_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF
.ENDIF

$(SLO)$/$(SECOND_BUILD)_%.obj :  %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(SLO)$/$(SECOND_BUILD)_$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.c)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c 
.ELSE
       +$(TYPE) $(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c )
       $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
.ENDIF
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)SLOFILES)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	srcipting rules		++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(PARFILES)"!=""
ALLPARFILES=$(foreach,i,$(PARFILES) $(foreach,j,$(SCP_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

SCP_PRODUCT_TYPE*=FAT

$(PAR)$/%.par : 
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="OS2"
    $(CC) /P+ /Pe+ /Pd+ $(INCLUDE) -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) $(*:b).scp > $(MISC)$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="WNT"
    $(CC) /EP $(INCLUDE) -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) $(*:b).scp > $(MISC)$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
    cpp.lcc -+ -P -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).scp > $(MISC)$/$(*:b).pre
.ELSE
    $(CC) -E -P -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -x c++ $(*:b).scp > $(MISC)$/$(*:b).pre
.ENDIF
.ENDIF
    +scpcomp -s $(MISC)$/$(*:b).pre -o $@

.ENDIF			# "$(PARFILES)"!=""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	cppumaker rules		++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

$(UNOUCROUT)$/%.hpp : $(UNOUCRDEP)
    +cppumaker $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) -T{$(subst,$/,. $(subst,$(UNOUCROUT)$/, $(@:db)))}  $(UNOUCRRDB)

$(UNOUCROUT)$/%.hdl : $(UNOUCRDEP)
    +cppumaker $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) -T{$(subst,$/,. $(subst,$(UNOUCROUT)$/, $(@:db)))}  $(UNOUCRRDB)

