#*************************************************************************
#
#   $RCSfile: pstrules.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hjs $ $Date: 2001-08-13 18:28:42 $
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
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""

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

.ENDIF			# "$($(SECOND_BUILD)_OBJFILES)"!=""

.IF "$($(SECOND_BUILD)_SLOFILES)"!=""

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

.ENDIF			# "$($(SECOND_BUILD)_SLOFILES)"!=""
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
    @+-$(MKDIR) $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))} >& $(NULLDEV)
.IF "$(GUI)"=="OS2"
    $(CC) /P+ /Pe+ /Pd+ $(INCLUDE) -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) $(SCPDEFS) -DDLLSUFFIX=$(DLLSUFFIX) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="WNT"
    $(CC) /EP $(INCLUDE) -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) $(SCPDEFS) -DDLLSUFFIX=$(DLLSUFFIX) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
    cpp.lcc -+ -P -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) $(SCPDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ELSE
    $(CC) -E -P -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) $(SCPDEFS) -DDLLSUFFIX=$(DLLSUFFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -x c++ $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.ENDIF
    +scpcomp -s $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre -o $@

.ENDIF			# "$(PARFILES)"!=""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	cppumaker rules		++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

$(UNOUCROUT)$/%.hpp : $(UNOUCRDEP)
    +cppumaker $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) -T{$(subst,$/,. $(subst,$(UNOUCROUT)$/, $(@:db)))}  $(UNOUCRRDB)

$(UNOUCROUT)$/%.hdl : $(UNOUCRDEP)
    +cppumaker $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) -T{$(subst,$/,. $(subst,$(UNOUCROUT)$/, $(@:db)))}  $(UNOUCRRDB)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	description fallbak	++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SHL1TARGET)$(SHL2TARGET)$(SHL3TARGET)$(SHL4TARGET)$(SHL5TARGET)$(SHL6TARGET)$(SHL7TARGET)$(SHL8TARGET)$(SHL9TARGET)"!=""

.IF "$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"==""

#fallback
$(MISC)$/%.xml : $(SOLARENV)$/src$/default_description.xml
    +$(COPY) $< $@
    
.ENDIF          # "$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"==""

.ENDIF			# "$(SHL1TARGET)$(SHL2TARGET)$(SHL3TARGET)$(SHL4TARGET)$(SHL5TARGET)$(SHL6TARGET)$(SHL7TARGET)$(SHL8TARGET)$(SHL9TARGET)"!=""

