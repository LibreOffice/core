##*******************************************************************
##*
##*    $Workfile:   rules.mk  $
##*
##*    Beschreibung      Compiler und Systemabhaengige
##*                      Entwicklungtools-Definitionen
##*                      Regeln
##*
##*    Ersterstellung    MH 9.2.96
##*    Letzte Aenderung  $Author: mh $ $Date: 2000-10-30 06:00:04 $
##*    $Revision: 1.4 $
##*
##*    $Logfile:   T:/solar/inc/rules.mkv  $
##*
##*    Copyright (c) 1990-1996, STAR DIVISION
##*
##*******************************************************************

MKFILENAME:=RULES.MK

.IF	"$(debug)"!=""
SEMADEBUG=echo $(VCSID) hat das File $@ mit debug=$(debug) uebersetzt >$(MISC)\$*.!d!
.ELSE
SEMADEBUG=@+rem dummy
.ENDIF

$(OBJ)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.cxx)
.ELSE
.IF "$(COM)"=="WTC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj )  $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx 
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(SLO)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(SPEW)"!=""
    @+-spew x:\er\stardiv.spw
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) -E  $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.cxx)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
.IF "$(COM)"=="WTC"
    @+if exist $@ $(RM) /q $@ >& nul

    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj )  $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(SLO)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) -E  $(CFLAGSINCXX)$(PWD)$/$*.cpp
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.cpp)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
.IF "$(COM)"=="WTC"
    @+if exist $@ $(RM) /q $@ >& nul

    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj )  $(CFLAGSINCXX)$(PWD)$/$*.cpp
.ELSE
.IF "$(COM)"=="BLC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $(MISC)$/*.cxx)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="WTC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU)) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
.IF "$(COM)"=="BLC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx)
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(OBJ)$/$(IDLPACKAGE)$/%.obj : $(OUTCXX)$/$(IDLPACKAGE)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
    @+-$(MKDIRHIER) $(OBJ)$/$(IDLPACKAGE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) -E  $(CFLAGSINCXX)$(OUTCXX)$(IDLPACKAGE)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ) $(OBJ)$/$(IDLPACKAGE)$/$*.o $(CFLAGSINCXX)$(OUTCXX)$/$(IDLPACKAGE)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $(OUTCXX)$/$(IDLPACKAGE)$/*.cxx)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="WTC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU)) $(CFLAGSOUTOBJ)$(OBJ)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$/$(IDLPACKAGE)$/$*.cxx
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$/$(IDLPACKAGE)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$/$(IDLPACKAGE)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF

$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/%.obj : $(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
    @+-$(MKDIRHIER) $(SLO)$(SMARTPRE)$/$(IDLPACKAGE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) -E  $(CFLAGSINCXX)$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.o $(CFLAGSINCXX)$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="WTC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU)) $(CFLAGSOUTOBJ)$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.obj $(CFLAGSINCXX)$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF


$(OBJ)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHOBJFLAGSU) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $(MISC)$/$*.cxx)
.ELSE
.IF "$(MDB)" != ""
    @echo $@
    @echo "#$*#"
    @echo "#$/#"
    @echo $(&:+"\n"))
.ENDIF
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="WTC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU)) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
.IF "$(COM)"=="BLC"
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx)
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.c)
.ELSE
.IF "$(COM)"=="WTC"
    echo $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c
    $(cc) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ)) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c
.ELSE
    +-@echo Cflags: $(CFLAGS)
    $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF
.ENDIF
.ENDIF

$(OBJ)$/%.obj : $(MISCX)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISCX)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISCX)$/$*.c )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISCX)$/$*.c )
    $(SEMADEBUG)
.ENDIF

$(SLO)$/%.obj : $(MISCX)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISCX)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c 
.ELSE
    $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF

$(SLO)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
.IF "$(COM)"=="WTC"
    @+if exist $@ $(RM) /q $@ >& nul
    $(cc) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT)) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c
    +IF exist $*.err @del $*.err
.ELSE
.IF "$(GUI)"=="MAC"
    @$(RM) $@
    $(cc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(shell $(UNIX2MACPATH) $@ $*.c)
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c 
.ELSE
       $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c )
.ENDIF
.ENDIF
.ENDIF
.ENDIF

# Objective-C files
$(OBJ)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(objc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(OBJ)$/%.obj : $(MISCX)$/%.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISCX)$/$*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)$/%.obj : $(MISCX)$/%.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISCX)$/$*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl//) $(CFLAGSCC) $(OBJCFLAGS) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

.IF "$(MAKEFILERC)"==""
$(MISC)$/%.dpc : $(CFILES) $(CXXFILES) $(RCFILES) $(UNOIDLTARGETS) $(SLOFILES) $(OBJFILES)
    @echo ------------------------------
    @echo Making: $@
    dmake subdmake=true $(MFLAGS) $(CALLMACROS) depend=t ALLDPC

$(MISC)$/%.dpw : $(CFILES) $(CXXFILES) $(RCFILES)
    @echo ------------------------------
    @echo Making: $@
    dmake $(MFLAGS) $(CALLMACROS) depend=t ALLDPC

$(MISC)$/%.dpr : $(SRCFILES) $(SRC1FILES) $(SRC2FILES) $(SRC3FILES)
    @echo ------------------------------ 
    @echo Making: $@
.IF "$(GUI)"=="WNT"  || "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
    @+-$(RM) $(SRS)$/$(PWD:f).*.dpr >& $(NULLDEV)
    dmake $(MFLAGS) $(CALLMACROS) make_srs_deps=true SRCALLTARGET -u
    +-$(TYPE) $(SRS)$/$(PWD:f).*.dpr >> $@
.ELSE
    @+echo "#" > $@
.ENDIF
.ELSE		# MAKEFILERC == ""
$/.dpc : 
    @echo ------------------------------
    @echo Ignoring: $@
.ENDIF

$(MISC)$/%.dpz :
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"  || "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
    @+-$(RM) $(MISC)$/$(TARGET).$(PWD:f).*.dpzz >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
    dmake $(MFLAGS) $(CALLMACROS) make_zip_deps=true ZIPALLTARGET -u
    $(TYPE) $(MISC)$/$(TARGET).$(PWD:f).*.dpzz >> $@
    @+-$(RM) $(MISC)$/$(TARGET).$(PWD:f).*.dpzz >& $(NULLDEV)
.ELSE
    @+echo "#" > $@
.ENDIF

# Dependencies fuer unoidl - Files

$(MISC)$/%.dp2 : makefile.mk
.IF "$(nodep)"==""
    @+echo Making dp2...
.IF "$(NOSMARTUNO)"==""
    $(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLDEPFLAGS) -Bs2u $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUT)$/cxx$(SMARTPRE)$/$(IDLPACKAGE) $(DEPIDLFILES)) > $(MISC)$/$(TARGET).dp2
.ENDIF   #NOSMARTUNO
.ELSE
    @+echo not making dp2...
    @+echo "#" > $@
.ENDIF

$(MISC)$/%.dp1 : makefile.mk
.IF "$(nodep)"==""
    @+echo Making dp1...
.IF "$(NOSMARTUNO)"==""
    +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLDEPFLAGS) -Bsmart $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(SMARTPRE)$/$(IDLPACKAGE) -OI$(OUT)$/cxx$(SMARTPRE)$/$(IDLPACKAGE) $(DEPIDLFILES)) > $(MISC)$/$(TARGET).dp1
.ELSE   #NOSMARTUNO
.IF "$(PACKAGE)"==""
    $(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLDEPFLAGS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUT)$/cxx $(DEPIDLFILES)) > $(MISC)$/$(TARGET).dp1
.ELSE
    +echo $(UNOIDL) $(UNOIDLDEFS) $(UNOIDLDEPFLAGS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -I$(SOLARIDLDIR)$/$(PACKAGE) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUT)$/cxx$/$(IDLPACKAGE) $(DEPIDLFILES) 
    $(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLDEPFLAGS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -I$(SOLARIDLDIR)$/$(PACKAGE) -I$(SOLARIDLDIR) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUT)$/cxx$/$(IDLPACKAGE) $(DEPIDLFILES)) > $(MISC)$/$(TARGET).dp1
.ENDIF
.ENDIF   #NOSMARTUNO
.ELSE
    @+echo not making dp1...
    @+echo "#" > $@
.ENDIF

$(MISC)$/%.dp3 : makefile.mk
.IF "$(nodep)"==""
    @+echo Making dp3.....
    +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) $(UNOIDLDEPFLAGS) -Bjava -P$(PRJNAME) -OH$(MISC)$/java $(DEPIDLFILES)) > $(MISC)$/$(TARGET).dp3
.ELSE
    @+echo not making dp3...
    @+echo "#" > $@
.ENDIF

$(MISC)$/%.dpj : makefile.mk
    @+echo Making dpj...
.IF "$(nodep)"!=""
.IF "$(GUI)"=="UNX"
    @+echo > $@
.ELSE
    @+echo. > $@
.ENDIF
.ELSE 			# "$(ndep)"==""
.IF "$(GUI)"=="UNX"
    @$(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/:/ /) -i $i) $(JAVACLASSFILES))
.ELSE
    $(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/;/ /) -i $i) $(JAVACLASSFILES))
.ENDIF
.ENDIF			# "$(nodep)"==""

$(SLO)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="WTC"
       $(ASM) $(AFLAGS) -D$(COM) $*.asm -fo=$(SLO)\$*.obj
       +-if exist $*.err @del $*.err
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
        $(ASM) $(AFLAGS) $*.asm,$(SLO)\$*.obj;
.ENDIF
       @$(SEMADEBUG)

$(OBJ)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="WTC"
       $(ASM) $(AFLAGS) $*.asm -fo=$(OBJ)\$*.obj
       +-if exist $*.err @del $*.err
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
        $(ASM) $(AFLAGS) $*.asm,$(OBJ)\$*.obj;
.ENDIF
       @$(SEMADEBUG)
#
# keine rule fuer *.java - abh. innerhalb eines packages!
#
#$(CLASSDIR)$/$(PACKAGE)$/%.class : %.java
#.IF "$(use_jdep)"!=""
#	$(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $<
#.ELSE
#	$(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $<
#.ENDIF

.SOURCE.idl : . $(SOLARVER)$/$(INPATH)$/idl$/remote $(SOLARVER)$/$(INPATH)$/idl $(SOLARVER)$/$(INPATH)$/idl$(UPDMINOREXT)$/$(PACKAGE)

.IF "$(TF_PACKAGES)"==""
.IF "$(NOSMARTUNO)"==""
# smart uno mapping files fuer den unoidl generieren !
$(MISC)$/%.smr : %.idl
    @+echo Making for nix $@
.IF "$(GUI)"=="UNX"
        @+-echo mapping > $@
        @+-echo \{ >> $@
.IF "$(IDLMAP)"!=""
        @+-echo "#import <$(IDLMAP)>" >> $@
        @+-echo "#import <$*.idl>" >> $@
.ELSE
        @+-echo "#import <$(PRJNAME)map.idl>" >> $@
        @+-echo "#import <$(PRJNAME)$/$*.idl>" >> $@
.ENDIF
        @+-echo "};" >> $@
.ELSE
        @+-echo mapping > $@
        @+-echo { >> $@
.IF "$(IDLMAP)"!=""
        @+-echo `#import <$(IDLMAP)>` >> $@
        @+-echo `#import <$*.idl>` >> $@
.ELSE
        @+-echo `#import <$(PRJNAME)map.idl>` >> $@
        @+-echo `#import <$(PRJNAME)$/$*.idl>` >> $@
.ENDIF
        @+-echo }; >> $@
.ENDIF

# call unoidl 
$(MISC)$/%.cxx : $(MISC)$/%.smr
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGENAME) -OH$(INCCOM)$/$(IDLPACKAGE) -OI$(MISC) $<
        +-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -P$(PRJNAME) -OH$(OUT)$/ucr $(DEPIDLFILES)
.IF "$(UPDATER)$(GUI)"=="YESWNT"
.IF "$(NOUNODOC)"==""
        +-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bdoc -P..$/$(PRJNAME)$/$(PACKAGE) -OH$(PRJ)$/..$/unodoc $(@:b).idl
.ENDIF
.ENDIF
$(INCCOM)$/$(IDLPACKAGE)$/%.hxx : $(MISC)$/%.smr
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) -P$(IDLPACKAGENAME) -OH$(INCCOM)$/$(IDLPACKAGE) -OI$(MISC) $<

.ELSE		#  "$(NOSMARTUNO)"==""

$(MISC)$/%.cxx : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(PRJNAME) -OH$(INCCOM)$/$(PRJNAME) -OI$(MISC) $<
        +-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -P$(PRJNAME) -OH$(OUT)$/ucr $<
$(INCCOM)$/$(PRJNAME)$/%.hxx : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) -P$(PRJNAME) -OH$(INCCOM)$/$(PRJNAME) -OI$(MISC) $<

.ENDIF  			# "$(NOSMARTUNO)"==""

.ELSE		# "$(TF_PACKAGES)"==""

.IF "$(NOSMARTUNO)"==""
# smart uno mapping files fuer den unoidl generieren !
$(MISC)$(SMARTPRE)$/$(IDLPACKAGE)$/%.smr : %.idl
    @+echo Making for TF_PACKAGES $@
    @+-$(MKDIRHIER) $(MISC)$(SMARTPRE)$/$(IDLPACKAGE)
.IF "$(GUI)"=="UNX"
        @+-echo mapping > $@
        @+-echo \{ >> $@
.IF "$(IDLMAP)"!=""
        @+-echo "#import <$(IDLMAP)>" >> $@
.IF "$(UCE)"==""
        @+-echo "#import <$*.idl>" >> $@
.ELSE
        @+-echo "#import <$(IDLPACKAGE)$/$*.idl>" >> $@
.ENDIF
.ELSE			# "$(IDLMAP)"!=""
        @+-echo "#import <$(PRJNAME)map.idl>" >> $@
        @+-echo "#import <$(PRJNAME)$/$*.idl>" >> $@
.ENDIF			# "$(IDLMAP)"!=""
        @+-echo "};" >> $@
.ELSE			# "$(GUI)"=="UNX"
        @+-echo mapping > $(@)
        @+-echo { >>  $(@)
.IF "$(IDLMAP)"!=""
        @+-echo `#import <$(IDLMAP)>` >>  $(@)
.IF "$(UCE)"==""
        @+-echo `#import <$*.idl>` >> $(@)
.ELSE
        @+-echo `#import <$(IDLPACKAGE)$/$*.idl>` >> $(@)
.ENDIF
.ELSE
        @+-echo `#import <$(PRJNAME)map.idl>` >>  $(@)
        @+-echo `#import <$(PRJNAME)$/$*.idl>` >> $(@)
.ENDIF
        @+-echo }; >> $(@)
.ENDIF			# "$(GUI)"=="UNX"

# call unoidl 
$(INCCOM)$(SMARTPRE)$/$(IDLPACKAGE)$/%.hxx $(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/%.cxx .UPDATEALL : $(MISC)$(SMARTPRE)$/$(IDLPACKAGE)$/%.smr
        +$(UNOIDL) $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(SMARTPRE)$/$(IDLPACKAGE) -OI$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE) $(<)
.IF "$(UPDATER)$(GUI)"=="YESWNT"
.IF "$(NOUNODOC)"==""
        +-$(UNOIDL) $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bdoc -P..$/$(PRJNAME)$/$(IDLPACKAGE) -OH$(PRJ)$/..$/unodoc $(@:b).idl
.ENDIF
.ENDIF

$(OUT)$/ucr$/$(IDLPACKAGE)$/%.urd : %.idl
        +$(UNOIDL) -Wb,c $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucr$/$(IDLPACKAGE) $<

$(OUT)$/ucrdoc$/$(IDLPACKAGE)$/%.urd : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucrdoc$/$(IDLPACKAGE) $<

$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/s2u_%.cxx : $(MISC)$(SMARTPRE)$/$(IDLPACKAGE)$/%.smr
    +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) $(TF_PACKAGES_DEF) -Bs2u -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE) $(<)

$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE)$/%.h : %.idl
    +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) $(TF_PACKAGES_DEF) -Wb,c -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) $<

.ELSE		#  "$(NOSMARTUNO)"==""

.IF "$(PACKAGE)"==""
$(MISC)$/%.cxx : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$/$(IDLPACKAGE) -OI$(MISC) $<
$(INCCOM)$/$(PRJNAME)$/%.hxx : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$/$(IDLPACKAGE) -OI$(MISC) $<
.ELSE
$(OUTCXX)$/$(IDLPACKAGE)$/%.cxx : %.idl
        +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUTCXX)$/$(IDLPACKAGE) $< )
$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE)$/%.hxx : %.idl
        +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINC) -P$(IDLPACKAGE) -OH$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE) -OI$(OUTCXX)$/$(IDLPACKAGE) $< )
.ENDIF

$(OUT)$/ucr$/$(IDLPACKAGE)$/%.urd : %.idl
        +$(UNOIDL) $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucr$/$(IDLPACKAGE) $<

.ENDIF  			# "$(NOSMARTUNO)"==""

.IF "$(javauno)"!=""
.IF "$(IDLFILES)"!=""
# idl to java...
$(MISC)$/java$/%.java :
    +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bjava -P$(PRJNAME) -OH$(MISC)$/java $(DEPIDLFILES)
.ENDIF
.ENDIF

.ENDIF

.IF "$(make_uno_ext)"!=""
$(MISC)$/%.uno : $(MISC)$/%.smr
        +-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Buno -prefixswitch -P$(PRJNAME) -OI$(MISC) -OH$(OUT)$/inc $(@:b).idl
        $(TOUCH) $(MISC)$/$(@:b).uno
.ENDIF
    
.IF "$(GUI)"=="WNTtest"
$(MISC)$/%.hid : %.src
    +echo Making hids...
    +mhids $< $(SRS) $(PRJNAME)
.ENDIF

#generate typelists from xml
$(MISC)$/%.mk $(MISC)$/%_description.cxx : %.xml 
    +xml2cmp -func $(MISC)$/$*_description.cxx -types $(MISC)$/$*.mk $*.xml

#generate private rdb
$(BIN)$/%.rdb: $(SOLARBINDIR)$/applicat.rdb $(MISC)$/%.mk
    +rdbmaker -BUCR -O$(BIN)$/$*.rdb $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB)

#strip dos lineends
$(MISC)$/%.sh : %.sh
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $< > $@

