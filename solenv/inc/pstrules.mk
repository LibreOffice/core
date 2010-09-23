#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	second_build rules			+++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""

$(OBJ)/$(SECOND_BUILD)_%.obj : %.cxx
    @echo "Making:   " $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @$(IFEXIST) $@ $(THEN) $(RM:s/+//) $@ >& $(NULLDEV) $(FI)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx
.ENDIF

$(OBJ)/$(SECOND_BUILD)_%.obj : %.c
    @echo "Making:   " $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$(SECOND_BUILD)_$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$(SECOND_BUILD)_$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @$(TYPE) $(mktmp $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
    @echo.
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c
.ELSE
    $(COMMAND_ECHO)$(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)\$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
    $(SEMADEBUG)
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)_OBJFILES)"!=""

.IF "$($(SECOND_BUILD)_SLOFILES)"!=""

$(SLO)/$(SECOND_BUILD)_%.obj : %.cxx
    @echo "Making:   " $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$(SECOND_BUILD)_$*.o $(CFLAGSINCXX)$(PWD)/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx
.ELSE
    $(COMMAND_ECHO)$(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$(SECOND_BUILD)_$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx )
.ENDIF
.ENDIF

$(SLO)/$(SECOND_BUILD)_%.obj :  %.c
    @echo "Making:   " $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$(SECOND_BUILD)_$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @-$(RM) $@
.IF "$(COM)"=="GCC"
       $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$(SECOND_BUILD)_$*.obj $*.c 
.ELSE
       $(COMMAND_ECHO)$(TYPE) $(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$(SECOND_BUILD)_$*.obj $*.c )
       $(COMMAND_ECHO)$(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $($(SECOND_BUILD)CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$(SECOND_BUILD)_$*.obj $*.c )
.ENDIF
.ENDIF

.ENDIF			# "$($(SECOND_BUILD)_SLOFILES)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+++++++++++	srcipting rules		++++++++++++++++++++++++++++++++++++++++
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.IF "$(PARFILES)"!=""
ULFPARFILES=$(foreach,i,$(ULFFILES) $(foreach,j,$(SCP_PRODUCT_TYPE) $(PAR)/$j/$(i:b).par))
MOREPARFILES=$(foreach,i,$(PARFILES) $(foreach,j,$(SCP_PRODUCT_TYPE) $(PAR)/$j/$i ))
ALLPARFILES=$(uniq $(ULFPARFILES) $(MOREPARFILES))

SCP_PRODUCT_TYPE*=FAT

.IF "$(GUI)" == "WNT"
LANGFILEEXT=mlf
.ELSE
LANGFILEEXT=jlf
.ENDIF

$(PAR)/%.par :
    @echo "Making:   " $@
    @@-$(MKDIR) $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}
.IF "$(GUI)"=="WNT"
    $(COMMAND_ECHO)$(CPPLCC) -+ -P $(INCLUDE) $(CDEFS) $(SCPDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) $(*:b).scp > $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).pre
.ENDIF
# YD: INCLUDE macro too long, include only few items (scp2 compile)
.IF "$(GUI)"=="OS2"
    $(CPPLCC) -+ -P -I..\..\inc -I..\..\os2gcci.pro\inc $(CDEFS) $(SCPDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) $(*:b).scp > $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).pre
.ENDIF
.IF "$(GUI)"=="UNX"
    $(COMMAND_ECHO)$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cpp.lcc -+ -P $(CDEFS) $(SCPDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) $(*:b).scp > $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).pre
.ENDIF
.IF "$(common_build_srs)"!=""
    $(COMMAND_ECHO)$(SCPCOMP) -l {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))/$(TARGET)/$(@:b).$(LANGFILEEXT)} -s $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).pre -o $@
.ELSE          # "$(common_build_srs)"!=""
    $(COMMAND_ECHO)$(SCPCOMP) -l {$(MISC)/$(TARGET)/$(@:b).$(LANGFILEEXT)} -s $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).pre -o $@
.ENDIF          # "$(common_build_srs)"!=""
# hacked version    
#    $(COMMAND_ECHO)$(SCPCOMP) -l $(@:b).$(LANGFILEEXT) -s $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).pre -o $@

.ENDIF			# "$(PARFILES)"!=""
