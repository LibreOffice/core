#*************************************************************************
#
#   $RCSfile: rules.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: mh $ $Date: 2001-03-08 16:31:39 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

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
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx 
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.ENDIF
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp 
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
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
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF			# "$(GUI)"=="UNX"

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
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
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
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
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
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(PCHSLOFLAGSU) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
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
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c 
.ELSE
       $(CC) @$(mktmp $(CFLAGS:s/stl//) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c )
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

# dependencies c / c++
.IF "$(verbose)"==""
noout=>& $(NULLDEV)
.ENDIF

$(MISC)$/s_%.dpcc : %.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/o_%.dpcc : %.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/s_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISCX)$/%.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISCX)$/%.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISCX)$/%.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISCX)$/%.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
# dependencies objective-c

$(MISC)$/s_%.dpcc : %.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISCX)$/%.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISCX)$/%.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHOBJFLAGSU:s/-f/-x/) $(CFLAGSOBJ:s/-f/-x/) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# dependency dummy for *.s files

$(MISC)$/s_%.dpcc : %.s
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CFLAGSCC:s/-f/-x/) $(PCHSLOFLAGSU:s/-f/-x/) $(CFLAGSSLO:s/-f/-x/) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# generated source files.

$(MISC)$/o_%.dpcc : 
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @+echo $@ : $(OBJ)$/$(@:b:^"__":s/__o_//).obj > $@
   
$(MISC)$/s_%.dpcc :
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @+echo $@ : $(SLO)$/$(@:b::^"__":s/__s_//).obj > $@

# dependencies script files

$(MISC)$/%.dpsc :
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @+-$(MKDIR) $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))} >& $(NULLDEV)
    @makedepend -f - -p$(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/ -o.par -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).scp > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).par  >> $@
    @+echo $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).par : $(*:b).scp >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# dependencies rc files (native resources for windows)

$(MISC)$/%.dprc : 
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @+-$(MKDIR) $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))} >& $(NULLDEV)
    @makedepend -f - -p$(RES)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/ -o.res $(RCLANGFLAGS_{$(subst,$(@:d:d:d:u), $(@:d:d:u))}:u:s/ //) $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).rc >> $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(RES)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).res  >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

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
    @+echo $(shell +$(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/:/ /) -i $i) $(JAVACLASSFILES)))
.ELSE
    @+echo $(shell +$(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/;/ /) -i $i) $(JAVACLASSFILES)))
.ENDIF
.ENDIF			# "$(nodep)"==""

$(SLO)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="WTC"
       $(ASM) $(AFLAGS) -D$(COM) $*.asm -fo=$(SLO)\$*.obj
       +-if exist $*.err @del $*.err
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
       $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)\$*.obj $*.asm
       +-if exist $*.err @del $*.err
.ELSE			# "$(ASM)"=="ml"
        @+if exist $@ $(RM) /q $@ >& nul
        $(ASM) $(AFLAGS) $*.asm,$(SLO)\$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF
       @$(SEMADEBUG)

$(OBJ)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="WTC"
       $(ASM) $(AFLAGS) $*.asm -fo=$(OBJ)\$*.obj
       +-if exist $*.err @del $*.err
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
       $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)\$*.obj $*.asm
       +-if exist $*.err @del $*.err
.ELSE			# "$(ASM)"=="ml"
        @+if exist $@ $(RM) /q $@ >& nul
        $(ASM) $(AFLAGS) $*.asm,$(OBJ)\$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
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
$(MISC)$/%.mk : %.xml 
    +xml2cmp -types $(MISC)$/$*.mk $*.xml

#generate descriptions from xml
$(MISC)$/%_description.cxx : %.xml 
    +xml2cmp -func $(MISC)$/$*_description.cxx $<

#generate private rdb
$(BIN)$/%.rdb: $(COMPRDB) $(MISC)$/%.mk
    +rdbmaker -BUCR -O$(BIN)$/$*.rdb $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB)

#strip dos lineends
$(MISC)$/%.sh : %.sh
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $< > $@

