#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: pstrules.mk,v $
#
#   $Revision: 1.40 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:56:33 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	svxlight rules			+++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SVXLIGHT)"=="TRUE"
.IF "$(SVXLIGHTOBJFILES)"!=""

SVXLIGHTDEFS=-DSVX_LIGHT

$(OBJ)$/sxl_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/sxl_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
     @+$(IFEXIST) $@ $(THEN) $(RM) $@ >& $(NULLDEV) $(FI)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF

.ENDIF			# "$(SVXLIGHTOBJFILES)"!=""

.IF "$(SVXLIGHTSLOFILES)"!=""

SVXLIGHTDEFS=-DSVX_LIGHT

$(SLO)$/sxl_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/sxl_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
     @+$(IFEXIST) $@ $(THEN) $(RM) $@ >& $(NULLDEV) $(FI)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(SVXLIGHTDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/sxl_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF

.ENDIF			# "$(SVXLIGHTSLOFILES)"!=""
.ENDIF			# "$(SVXLIGHT)"=="TRUE"

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	second_build rules			+++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""

#$(SECOND_BUILD)CDEFS=-DSVX_LIGHT

$(OBJ)$/$(SECOND_BUILD)_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
     @+$(IFEXIST) $@ $(THEN) $(RM) $@ >& $(NULLDEV) $(FI)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF

$(OBJ)$/$(SECOND_BUILD)_%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$(SECOND_BUILD)_$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
    @+echo.
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)_OBJFILES)"!=""

.IF "$($(SECOND_BUILD)_SLOFILES)"!=""

$(SLO)$/$(SECOND_BUILD)_%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @+-$(RM) $@
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ENDIF

$(SLO)$/$(SECOND_BUILD)_%.obj :  %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$(SECOND_BUILD)_$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @+-$(RM) $@
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c 
.ELSE
       +$(TYPE) $(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c )
       $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)_SLOFILES)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	srcipting rules		++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(PARFILES)"!=""
ULFPARFILES=$(foreach,i,$(ULFFILES) $(foreach,j,$(SCP_PRODUCT_TYPE) $(PAR)$/$j$/$(i:b).par))
MOREPARFILES=$(foreach,i,$(PARFILES) $(foreach,j,$(SCP_PRODUCT_TYPE) $(PAR)$/$j$/$i ))
ALLPARFILES=$(uniq $(ULFPARFILES) $(MOREPARFILES))

SCP_PRODUCT_TYPE*=FAT

.IF "$(GUI)" == "WNT"
LANGFILEEXT=mlf
.ELSE
LANGFILEEXT=jlf
.ENDIF

$(PAR)$/%.par :
    @echo ------------------------------
    @echo Making: $@
    @+-$(MKDIR) $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))} >& $(NULLDEV)
.IF "$(GUI)"=="WNT"
    $(CPPLCC) -+ -P $(INCLUDE) $(CDEFS) $(SCPDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="UNX"
    cpp.lcc -+ -P $(CDEFS) $(SCPDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.IF "$(common_build_srs)"!=""
    +$(SCPCOMP) -l {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$(@:b).$(LANGFILEEXT)} -s $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre -o $@
.ELSE          # "$(common_build_srs)"!=""
    +$(SCPCOMP) -l {$(MISC)$/$(TARGET)$/$(@:b).$(LANGFILEEXT)} -s $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre -o $@
.ENDIF          # "$(common_build_srs)"!=""
# hacked version    
#    +$(SCPCOMP) -l $(@:b).$(LANGFILEEXT) -s $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre -o $@

.ENDIF			# "$(PARFILES)"!=""

.IF "$(TESTDIR)"!=""

# workaround for strange dmake bug:
# if the previous block was a rule or a target, "\#" isn't recognized
# as an escaped "#". if it was an assignment, escaping works...
some_unique_variable_name:=1

.IF "$(OS)" == "WNT"
REGEXP:="s/^[\#].*$$//"
.ELSE
REGEXP:='s/^[\#].*$$//'
.ENDIF

$(MISC)$/%.exp : sce$/%.sce
    @+-$(RM) $@ >& $(NULLDEV)
    @+-$(RM) $(@:d)$(@:b).tst >& $(NULLDEV)
    +$(TYPE) $< | sed $(REGEXP) | sed "s/^/test_/" > $(@:d)$(@:b).tst
    +$(TYPE) $(@:d)$(@:b).tst | sed "/test_./ w $@"

.ENDIF			# "$(TESTDIR)"!=""
