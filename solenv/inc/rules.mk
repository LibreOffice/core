#*************************************************************************
#
#   $RCSfile: rules.mk,v $
#
#   $Revision: 1.33 $
#
#   last change: $Author: hjs $ $Date: 2001-08-14 09:15:43 $
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
    $(cc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c
.ELSE
    +-@echo Cflags: $(CFLAGS)
    $(CC) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF

$(OBJ)$/%.obj : $(MISCX)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISCX)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISCX)$/$*.c )
    @+echo.
    $(CC) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISCX)$/$*.c )
    $(SEMADEBUG)
.ENDIF

$(SLO)$/%.obj : $(MISCX)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISCX)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c 
.ELSE
    $(CC) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISCX)$/$*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF

$(SLO)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(cc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+if exist $@ $(RM) /q $@ >& nul
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c 
.ELSE
       $(CC) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c )
.ENDIF
.ENDIF

# Objective-C files
$(OBJ)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
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
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(PCHOBJFLAGSU) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISCX)$/$*.m
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
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(PCHSLOFLAGSU) $(CDEFS) $(CDEFSSLO) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISCX)$/$*.m
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
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(PCHSLOFLAGSU) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.m
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/o_%.dpcc : %.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/s_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @makedepend -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @makedepend -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
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
    +dmake $(MFLAGS) $(CALLMACROS) make_zip_deps=true ZIPALLTARGET -u
    $(TYPE) $(MISC)$/$(TARGET).$(PWD:f).*.dpzz >> $@
    @+-$(RM) $(MISC)$/$(TARGET).$(PWD:f).*.dpzz >& $(NULLDEV)
.ELSE
    @+echo "#" > $@
.ENDIF

# Dependencies fuer java - Files

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

$(OUT)$/ucr$/$(IDLPACKAGE)$/%.urd : %.idl
        +idlc @$(mktmp $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -O$(OUT)$/ucr$/$(IDLPACKAGE) $< )

$(OUT)$/ucrdoc$/$(IDLPACKAGE)$/%.urd : %.idl
        +idlc @$(mktmp $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -C -O$(OUT)$/ucrdoc$/$(IDLPACKAGE) $< )		

.IF "$(GUI)"=="WNTtest"
$(MISC)$/%.hid : %.src
    +echo Making hids...
    +mhids $< $(SRS) $(PRJNAME)
.ENDIF

# make *.xml descriptions available in $(MISC)
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml : %.xml
    +$(COPY) $< $@

##generate typelists from xml
$(MISC)$/%.mk : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml 
    +xml2cmp -types $(MISC)$/$*.mk $*.xml

#generate descriptions from xml
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR)_description.cxx : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml 
    +xml2cmp -func $(MISC)$/$*$($(WINVERSIONNAMES)_MAJOR)_description.cxx $<

#generate private rdb
$(BIN)$/%.rdb: $(COMPRDB) $(MISC)$/%.mk
    +rdbmaker -BUCR -O$(BIN)$/$*.rdb @$(mktmp $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB))

#strip dos lineends
$(MISC)$/%.sh : %.sh
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $< > $@

