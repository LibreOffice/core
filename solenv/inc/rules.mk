#*************************************************************************
#
#   $RCSfile: rules.mk,v $
#
#   $Revision: 1.55 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 16:54:42 $
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

$(OBJ)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @+$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
.ENDIF
.ENDIF


$(SLO)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) -E  $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF			# "$(GUI)"=="UNX"

$(SLO)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) -E  $(CFLAGSINCXX)$(PWD)$/$*.cpp
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF

$(OBJ)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF

$(OBJ)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
    @+$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c
.ELSE
    +-@echo Cflags: $(CFLAGS)
    $(CC) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $*.c )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : $(MISC)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISC)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISC)$/$*.c )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$*.obj $(MISC)$/$*.c )
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISC)$/$*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISC)$/$*.c )
    @+$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISC)$/$*.c 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)\$*.obj $(MISC)$/$*.c )
.ENDIF
.ENDIF

$(SLO)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.c
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(COM)"=="GCC"
       $(CXX) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c 
.ELSE
       $(CXX) @$(mktmp $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c )
.ENDIF
.ENDIF

# Objective-C files
$(OBJ)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(OBJ)$/%.obj : $(MISC)$/%.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISC)$/$*.m
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)$/%.obj : $(MISC)$/%.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISC)$/$*.m
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
    $(objc) $(CFLAGS:s/stl/dont_use_stl/) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.m
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
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/o_%.dpcc : %.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/s_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.cpp
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISC)$/%.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISC)$/%.c
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISC)$/%.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISC)$/%.cxx
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISC)$/%.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : $(MISC)$/%.m
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
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
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
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
    @$(MAKEDEPEND) -f - -p$(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/ -o.par -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(*:b).scp > $@
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
    @$(MAKEDEPEND) -f - -p$(RES)$/ -o.res $(RCLANGFLAGS_{$(subst,$(@:d:d:d:u), $(@:d:d:u))}:u:s/ //) $(CDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) -I$(SOLARENV)$/inc $(*:b).rc >> $@
.IF "$(LAZY_DEPS)"==""	
#	@+echo $@ : $(RES)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).res  >> $@
    @+echo $@ : $(RES)$/$(*:b).res  >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/%.dpc : $(CFILES) $(CXXFILES) $(RCFILES) $(UNOIDLTARGETS) $(SLOFILES) $(OBJFILES) $(DEPOBJFILES)
    @echo ------------------------------
    @echo Making: $@
    dmake subdmake=true $(MFLAGS) $(MAKEFILE) $(CALLMACROS) depend=t ALLDPC

$(MISC)$/%.dpr :
    @echo ------------------------------ 
.IF "$(nodep)"==""
    @echo Making: $@
    +-$(RM) $@  >& $(NULLDEV)
    dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) NO_HIDS=true make_srs_deps=true $(DEPSRSFILES)
    +-$(TYPE) $(MISC)$/$(PWD:f).*.dprr >> $@
.ENDIF			# "$(nodep)"==""

$(MISC)$/%.dpz :
    @echo ------------------------------
.IF "$(nodep)"==""
    @echo Making: $@
    @+-$(RM) $@  >& $(NULLDEV)
# line too long on 4nt
.IF "$(USE_SHELL)"!="4nt"
    +dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_zip_deps=true $(ZIPDEPFILES)
.ELSE			# "$(USE_SHELL)"!="4nt"
    +$(TYPE) $(mktmp $(ZIPDEPFILES:s/\/\\/)) | xargs -n 20 dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_zip_deps=true
.ENDIF			# "$(USE_SHELL)"!="4nt"
    $(TYPE) $(ZIPDEPFILES) $(mktmp $(NULL)) | grep -v "CVS" >> $@
    +echo zipdep_langs=$(alllangiso) >> $@
    @+-$(RM) $(ZIPDEPFILES) >& $(NULLDEV)
.ENDIF			# "$(nodep)"==""

# Dependencies fuer java - Files

$(MISC)$/%.dpj :
    @+echo Making dpj...
.IF "$(nodep)"!=""
.IF "$(GUI)"=="UNX"
    @+echo > $@
.ELSE
    @+$(ECHONL) > $@
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
        @+-$(RM) $@ >& $(NULLDEV)
        $(ASM) $(AFLAGS) $*.asm,$(SLO)\$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF

$(OBJ)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
       $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)\$*.obj $*.asm
       +-if exist $*.err @del $*.err
.ELSE			# "$(ASM)"=="ml"
        @+if exist $@ $(RM) $@ >& $(NULLDEV)
        $(ASM) $(AFLAGS) $*.asm,$(OBJ)\$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"

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
        +$(IDLC) @$(mktmp $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -O$(OUT)$/ucr$/$(IDLPACKAGE) $< )

$(OUT)$/ucrdoc$/$(IDLPACKAGE)$/%.urd : %.idl
        +$(IDLC) @$(mktmp $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -C -O$(OUT)$/ucrdoc$/$(IDLPACKAGE) $< )		

# generate hid files
$(SRS)$/%.hid : %.src
    @echo ------------------------------
    @echo Making hid $@
        +mhids.bat $*.src $(SRS) $(PRJNAME) $(CDEFS) $(INCLUDE)

# make *.xml descriptions available in $(MISC)
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml : %.xml
    +$(COPY) $< $@

# dummy rule to make sure xml file is in place when used in settings.mk
$(MISC)$/%.mk : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml
    +$(TOUCH) $@
    +echo XML2MK_FILES += $(@:b) >> $@

#generate descriptions from xml
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR)_description.cxx : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml 
    +xml2cmp -func $(MISC)$/$*$($(WINVERSIONNAMES)_MAJOR)_description.cxx $<

#generate private rdb
$(BIN)$/%.rdb: $(COMPRDB) $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml
    +$(RDBMAKER) -BUCR -O$(BIN)$/$*.rdb @$(mktmp $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB))

#strip dos lineends
$(MISC)$/%.sh : %.sh
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $< > $@

$(COMMONMISC)$/$(TARGET)$/%.ulf : %.ulf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(ULFEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(TARGET)$/%.xrb : %.xrb
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(XMLEX) -t xrb -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(TARGET)$/%.xrm : %.xrm
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)

# dirty hack
# if local *.sdf file is missing
%.sdf:
    +echo > $@

