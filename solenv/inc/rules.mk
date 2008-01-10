#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: rules.mk,v $
#
#   $Revision: 1.93 $
#
#   last change: $Author: obo $ $Date: 2008-01-10 13:02:50 $
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

MKFILENAME:=RULES.MK

$(OBJ)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
.IF "$(noadjust)"==""   
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(noadjust)"==""   
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.cc
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cc
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @+-$(RM) $@ >& $(NULLDEV)
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cc )
    @+$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cc 
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cc )
.ENDIF
.ENDIF

.IF "$(ENABLE_PCH)"!=""
$(SLO)$/precompiled.% .PHONY:
    -$(MKDIRHIER) $(SLO)$/pch
.IF "$(COM)"=="MSC"
    $(CXX) @$(mktmp -Fp$(SLO)$/pch$/precompiled_$(PRJNAME).hxx$(PCHPOST) $(CFLAGS_CREATE_PCH) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGS_NO_EXCEPTIONS) -DEXCEPTIONS_OFF $(CFLAGSAPPEND) $(INCPCH)$/precompiled_$(PRJNAME).cxx)
.ELIF "$(COM)"=="GCC" && "$(CCNUMVER)">="000300040000"
    $(CXX) -o$(SLO)$/pch$/precompiled_$(PRJNAME).hxx$(PCHPOST) $(CFLAGS_CREATE_PCH) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGS_NO_EXCEPTIONS) -DEXCEPTIONS_OFF $(CFLAGSAPPEND) $(INCPCH)/precompiled_$(PRJNAME).hxx
    @echo "#error Tried to use wrong precompiled header" > $(SLO)$/pch$/precompiled_$(PRJNAME).hxx
.ELSE			# "$(COM)"=="MSC"
    $(TOUCH) $(SLO)$/pch$/precompiled_$(PRJNAME).hxx$(PCHPOST)
.ENDIF			# "$(COM)"=="MSC"
    @echo USED_PCHFLAGS=$(CFLAGS)$(CFLAGSCXX)$(CFLAGSCXXSLO)$(CFLAGSSLO)$(CDEFS)$(CDEFSSLO)$(CDEFSMT)$(CFLAGS_NO_EXCEPTIONS) -DEXCEPTIONS_OFF$(CFLAGSAPPEND)> $(INCCOM)$/pch.mk

$(SLO)$/precompiled_ex.% .PHONY:
    -$(MKDIRHIER) $(SLO)$/pch_ex
.IF "$(COM)"=="MSC"
    $(CXX) @$(mktmp -Fp$(SLO)$/pch_ex$/precompiled_$(PRJNAME).hxx$(PCHPOST) $(CFLAGS_CREATE_PCH:s/pchname/pchname_ex/) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSEXCEPTIONS) -DEXCEPTIONS_ON $(CFLAGSAPPEND) $(INCPCH)$/precompiled_$(PRJNAME).cxx)
.ELIF "$(COM)"=="GCC" && "$(CCNUMVER)">="000300040000"
    $(CXX) -o$(SLO)$/pch_ex$/precompiled_$(PRJNAME).hxx$(PCHPOST) $(CFLAGS_CREATE_PCH) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSEXCEPTIONS) -DEXCEPTIONS_ON $(CFLAGSAPPEND) $(INCPCH)/precompiled_$(PRJNAME).hxx
    @echo "#error Tried to use wrong precompiled header" > $(SLO)$/pch_ex$/precompiled_$(PRJNAME).hxx
.ELSE			# "$(COM)"=="MSC"
    $(TOUCH) $(SLO)$/pch_ex$/precompiled_$(PRJNAME).hxx$(PCHPOST)
.ENDIF			# "$(COM)"=="MSC"
    @echo USED_EXCEPTIONS_PCHFLAGS=$(CFLAGS)$(CFLAGSCXX)$(CFLAGSCXXSLO)$(CFLAGSSLO)$(CDEFS)$(CDEFSSLO)$(CDEFSMT)$(CFLAGSEXCEPTIONS) -DEXCEPTIONS_ON$(CFLAGSAPPEND)> $(INCCOM)$/pch_ex.mk

.INCLUDE .IGNORE : $(INCCOM)$/pch.mk
.INCLUDE .IGNORE : $(INCCOM)$/pch_ex.mk
.ENDIF			# "$(ENABLE_PCH)"!=""

$(SLO)$/%.obj : %.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(ENABLE_PCH)"!=""
# just a helper var	
    @noop $(assign used_exc_switches=$(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)))
# cleanup first
    @noop $(assign ACT_PCH_SWITCHES:=$(NULL))
# eq: first string is a copy of the compile line,
# second generated by pch creation
# use pch with exception support
    @noop $(assign ACT_PCH_SWITCHES+=$(eq,$(strip $(USED_EXCEPTIONS_PCHFLAGS)),$(strip $(CFLAGS)$(CFLAGSCXX)$(CFLAGSCXXSLO)$(CFLAGSSLO)$(CDEFS:s/\//)$(CDEFSSLO)$(CDEFSMT)$(used_exc_switches)$(CFLAGSAPPEND)) $(CFLAGS_USE_EXCEPTIONS_PCH) $(NULL)))
# use pch without exception support
    @noop $(assign ACT_PCH_SWITCHES+=$(eq,$(strip $(USED_PCHFLAGS)),$(strip $(CFLAGS)$(CFLAGSCXX)$(CFLAGSCXXSLO)$(CFLAGSSLO)$(CDEFS:s/\//)$(CDEFSSLO)$(CDEFSMT)$(used_exc_switches)$(CFLAGSAPPEND)) $(CFLAGS_USE_PCH) $(NULL)))
.ENDIF			# "$(ENABLE_PCH)"!=""
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) -E  $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CAPTURE_COMMAND) $(CXX) $(ACT_PCH_SWITCHES) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cxx $(CAPTURE_OUTPUT)
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"=="" && "$(CAPTURE_COMMAND)"==""
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
    @@-$(RM) $@ >& $(NULLDEV)
    @$(TYPE) $(mktmp $(CXX) $(ACT_PCH_SWITCHES) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx ) $(CAPTURE_OUTPUT)
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(ACT_PCH_SWITCHES) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx
.ELSE
    $(CXX) @$(mktmp $(ACT_PCH_SWITCHES) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cxx )
.ENDIF		    # "$(COM)"=="GCC"	
.ENDIF			# "$(GUI)"=="UNX"

$(SLO)$/%.obj : %.cpp
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(PWD)$/$*.cpp
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(PWD)$/$*.cpp
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(PWD)$/$*.cpp )
.ENDIF
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.cc
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)$/$*.cc
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cc
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    +$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @+$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cc )
    @+$(ECHONL)
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cc )
.ENDIF

$(OBJ)$/%.obj : $(MISC)$/%.cxx
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(CFLAGSINCXX)$(MISC)$/$*.cxx
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @$(TYPE) $(mktmp $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx
.ELSE
    $(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(CFLAGSINCXX)$(MISC)$/$*.cxx )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @$(TYPE) $(mktmp $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $*.c )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $*.c
.ELSE
    -@echo Cflags: $(CFLAGS) $(INCLUDE)
    $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $*.c )
.ENDIF
.ENDIF

$(OBJ)$/%.obj : $(MISC)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISC)$/$*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(MISC)$/$*.c )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(MISC)$/$*.c
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)$/$*.obj $(MISC)$/$*.c )
.ENDIF
.ENDIF

$(SLO)$/%.obj : $(MISC)$/%.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISC)$/$*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(MISC)$/$*.c )
    @$(ECHONL)
.IF "$(COM)"=="GCC"
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(MISC)$/$*.c 
.ELSE
    $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $(MISC)$/$*.c )
.ENDIF
.ENDIF

$(SLO)$/%.obj : %.c
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.c
.IF "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
    $(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""    
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
       $(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c 
.ELSE
       $(CC) @$(mktmp $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)$/$*.obj $*.c )
.ENDIF
.ENDIF

# Objective-C files
$(OBJ)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.m
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
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
    $(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISC)$/$*.m
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)$/%.obj : $(MISC)$/%.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISC)$/$*.m
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)$/%.obj : %.m
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.m
     $(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# dependencies c / c++

not_existing$/s_%.dpcc : %.c;@noop $(assign all_local_slo+:=$<)
not_existing$/o_%.dpcc : %.c;@noop $(assign all_local_obj+:=$<)

# Objective-C++ files
$(OBJ)$/%.obj : %.mm
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(objc) $(CFLAGS) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.mm
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $*.mm
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ENDIF
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C++ files
$(OBJ)$/%.obj : $(MISC)$/%.mm
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)$/$*.o $(MISC)$/$*.mm
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C++ files
$(SLO)$/%.obj : $(MISC)$/%.mm
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $(MISC)$/$*.mm
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C++ files
$(SLO)$/%.obj : %.mm
    @echo ------------------------------
    @echo Making: $@
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(objc) $(CFLAGS) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)$/$*.o $*.mm
    +if ( -e $(@:s/.obj/.o/)) $(TOUCH) $@
.ELSE		"$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# dependencies c / c++
.IF "$(verbose)"==""
noout=>& $(NULLDEV)
.ENDIF

$(MISC)$/s_%.dpcc : %.c
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/%.dpslo :
# faster but unusable with current source (e.g. external include guards)
#	$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(all_local_slo) $(all_misc_slo)) > $@
# slower but with correct output
    @@$(RM) $@
    @$(TOUCH) $@
    @$(foreach,i,$(all_local_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) >> $@ ))
    @$(foreach,i,$(all_misc_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) >> $@ ))
# for both
    $(TYPE) $(mktmp $(foreach,i,$(all_local_slo:b:+".obj") $(@:^"\n") : $(SLO)$/$i) $(foreach,i,$(all_misc_slo:b:+".obj") $(@:^"\n") : $(SLO)$/$i)) >> $@

$(MISC)$/%.dpobj :
# faster but unusable with current source (e.g. external include guards)
#	$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(all_local_obj) $(all_misc_obj)) > $@
# slower but with correct output
    @@$(RM) $@
    @$(TOUCH) $@
    @$(foreach,i,$(all_local_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) >> $@ ))
    @$(foreach,i,$(all_misc_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) >> $@ ))
# for both
    $(TYPE) $(mktmp $(foreach,i,$(all_local_obj:b:+".obj") $(@:^"\n") : $(OBJ)$/$i) $(foreach,i,$(all_misc_obj:b:+".obj") $(@:^"\n") : $(OBJ)$/$i)) >> $@

# see also %.dpslo 
not_existing$/s_%.dpcc : %.cxx;@noop $(assign all_local_slo+:=$<)
not_existing$/o_%.dpcc : %.cxx;@noop $(assign all_local_obj+:=$<)

not_existing$/s_%.dpcc : %.cpp;@noop $(assign all_local_slo+:=$<)
not_existing$/o_%.dpcc : %.cpp;@noop $(assign all_local_obj+:=$<)

not_existing$/s_%.dpcc : $(MISC)$/%.c;@noop $(assign all_misc_slo+:=$<)
not_existing$/o_%.dpcc : $(MISC)$/%.c;@noop $(assign all_misc_obj+:=$<)

not_existing$/s_%.dpcc : $(MISC)$/%.cxx;@noop $(assign all_misc_slo+:=$<)
not_existing$/o_%.dpcc : $(MISC)$/%.cxx;@noop $(assign all_misc_obj+:=$<)

# dependencies objective-c

$(MISC)$/s_%.dpcc : %.m
    @@-$(RM) $@
    @$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/o_%.dpcc : %.m
    @@-$(RM) $@
    @$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/s_%.dpcc : $(MISC)$/%.m
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/o_%.dpcc : $(MISC)$/%.m
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# dependencies objective-c++

$(MISC)$/s_%.dpcc : %.mm
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(SLO)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/o_%.dpcc : %.mm
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @+echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @+echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	
   
$(MISC)$/s_%.dpcc : $(MISC)$/%.mm
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
   
$(MISC)$/o_%.dpcc : $(MISC)$/%.mm
    @echo ------------------------------ $(noout)
    @echo Making: $@ $(noout)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)$/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"	
    @$(MAKEDEPEND) -f - -p$(OBJ)$/ $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"	
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(OBJ)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# dependency dummy for *.s files

$(MISC)$/s_%.dpcc : %.s
    @@-$(RM) $@
    @$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(SLO)$/$(<:b).obj >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# generated source files.

$(MISC)$/o_%.dpcc : 
    @@-$(RM) $@
    @echo $@ : $(OBJ)$/$(@:b:^"__":s/__o_//).obj > $@

$(MISC)$/s_%.dpcc :
    @@-$(RM) $@
    @echo $@ : $(SLO)$/$(@:b::^"__":s/__s_//).obj > $@

# dependencies script files

$(MISC)$/%.dpsc :
    @@-$(RM) $@
    @@-$(MKDIR) $(MISC)$/{$(subst,$(@:d:d:d), $(@:d:d))}
    @$(MAKEDEPEND) -f - -p$(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))} -o.par -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(*:b).scp > $@
.IF "$(LAZY_DEPS)"==""	
    @echo $@ : $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).par  >> $@
    @echo $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).par : $(*:b).scp >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

# dependencies rc files (native resources for windows)

$(MISC)$/%.dprc : 
    @@-$(RM) $@
    @$(MAKEDEPEND) -f - -p$(RES) -o.res $(RCLANGFLAGS_{$(subst,$(@:d:d:d:u), $(@:d:d:u))}:u:s/ //) $(CDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) -I$(SOLARENV)$/inc $(*:b).rc >> $@
.IF "$(LAZY_DEPS)"==""	
#	@echo $@ : $(RES)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(*:b).res  >> $@
    @echo $@ : $(RES)$/$(*:b).res  >> $@
.ELSE			# "$(LAZY_DEPS)"==""	
    @echo LAZY_DEPS=were_used_to_generate >> $@
.ENDIF			# "$(LAZY_DEPS)"==""	

$(MISC)$/%.dpr :
    @echo ------------------------------ 
.IF "$(nodep)"==""
    @echo Making: $@
    @@-$(RM) $@
    dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) NO_HIDS=true make_srs_deps=true $(DEPSRSFILES)
    -$(TYPE) $(MISC)$/$(PWD:f).*.dprr >> $@
.ENDIF			# "$(nodep)"==""

$(MISC)$/%.dpz :
    @echo ------------------------------
.IF "$(nodep)"==""
    @echo Making: $@
    @@-$(RM) $@
# line too long on 4nt
.IF "$(USE_SHELL)"!="4nt"
    dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_zip_deps=true $(ZIPDEPFILES)
.ELSE			# "$(USE_SHELL)"!="4nt"
    $(TYPE) $(mktmp $(ZIPDEPFILES:s/\/\\/)) | xargs dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_zip_deps=true
.ENDIF			# "$(USE_SHELL)"!="4nt"
    $(TYPE) $(ZIPDEPFILES) $(mktmp $(NULL)) | grep -v "CVS" >> $@
    echo zipdep_langs=$(alllangiso) >> $@
    @@-$(RM) $(ZIPDEPFILES)
.ENDIF			# "$(nodep)"==""

# Dependencies fuer java - Files

$(MISC)$/%.dpj :
    @echo Making dpj...
.IF "$(nodep)"!=""
.IF "$(GUI)"=="UNX"
    @echo > $@
.ELSE
    @$(ECHONL) > $@
.ENDIF
.ELSE 			# "$(ndep)"==""
.IF "$(GUI)"=="UNX"
    @echo $(shell @$(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/:/ /) -i $i) $(JAVACLASSFILES)))
.ELSE
    @echo javadeps
    @echo $(shell @$(STARDEP) @$(mktmp -o $@ -i $(CLASSDIR) $(foreach,i,$(JAVADEPINCLUDES:s/;/ /) -i $i) $(JAVACLASSFILES)))
.ENDIF
.ENDIF			# "$(nodep)"==""

$(SLO)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="GCC"
.IF "$(ASM)"=="ml"
       $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)$/$*.obj $*.asm
       @@-$(RM) $*.err
.ELSE			# "$(ASM)"=="ml"
        @@-$(RM) $@
        $(ASM) $(AFLAGS) $*.asm,$(SLO)$/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ELSE
.IF "$(COM)"=="WTC"
        $(ASM) $(AFLAGS) -D$(COM) $*.asm -fo=$(SLO)$/$*.obj
        -$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
    $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)$/$*.obj $*.asm
    -$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE			# "$(ASM)"=="ml"
    @@-$(RM) $@
    $(ASM) $(AFLAGS) $*.asm,$(SLO)$/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF
.ENDIF

$(OBJ)$/%.obj : %.asm
       @echo ------------------------------
       @echo Making: $@
.IF "$(COM)"=="GCC"
.IF "$(ASM)"=="ml"
       $(ASM) $(AFLAGS) -D$(COM) /Fo$(OBJ)$/$*.obj $*.asm
       @@-$(RM) $*.err
.ELSE			# "$(ASM)"=="ml"
        @@-$(RM) $@
        $(ASM) $(AFLAGS) $*.asm,$(OBJ)$/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
    $(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)$/$*.obj $*.asm
    -$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE			# "$(ASM)"=="ml"
    @$(IFEXIST) $@ $(THEN) $(RM:s/+//) $@ >& $(NULLDEV) $(FI)
    $(ASM) $(AFLAGS) $*.asm,$(OBJ)$/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF

$(OUT)$/ucr$/$(IDLPACKAGE)$/%.urd : %.idl
        @noop $(assign all_outdated_idl+:=$<)

$(OUT)$/ucrdoc$/$(IDLPACKAGE)$/%.urd : %.idl
        @noop $(assign all_outdated_doc_idl+:=$<)

# generate hid files
$(SRS)$/%.hid : %.src
    @echo ------------------------------
    @echo Making hid $@
    $(PERL) $(SOLARENV)$/bin$/mhids.pl $*.src $(SRS) $(PRJNAME) $(CDEFS) $(INCLUDE)

# make *.xml descriptions available in $(MISC)
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml : %.xml
    $(COPY) $< $@

# dummy rule to make sure xml file is in place when used in settings.mk
$(MISC)$/%.mk : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml
    $(TOUCH) $@
    echo XML2MK_FILES += $(@:b) >> $@

#generate descriptions from xml
$(MISC)$/%$($(WINVERSIONNAMES)_MAJOR)_description.cxx : $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml 
    xml2cmp -func $(MISC)$/$*$($(WINVERSIONNAMES)_MAJOR)_description.cxx $<

#generate private rdb
$(BIN)$/%.rdb: $(MISC)$/%$($(WINVERSIONNAMES)_MAJOR).xml
    $(RDBMAKER) -BUCR -O$(BIN)$/$*.rdb @$(mktmp $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB))

#strip dos lineends
$(MISC)$/%.sh : %.sh
    @@-$(RM) -f $@
    @tr -d "\015" < $< > $@

# merge targets
.IF "$(WITH_LANG)"!=""
$(COMMONMISC)$/$(TARGET)$/%.ulf : %.ulf
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(ULFEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(TARGET)$/%.xrb : %.xrb
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(XMLEX) -t xrb -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(MYPATH)$/%.xrm : %.xrm
    -$(MKDIRHIER) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(TARGET)$/%.xrm : %.xrm
    -$(MKDIRHIER) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(COMMONMISC)$/%.xrm : %.xrm
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)
.ENDIF			# "$(WITH_LANG)"!=""

.IF "$(WITH_LANG)"!=""
$(COMMONMISC)$/$(TARGET)$/%.jlf : $$(@:db).ulf
.ELSE			# "$(WITH_LANG)"!=""
$(COMMONMISC)$/$(TARGET)$/%.jlf : $$(@:b).ulf
.ENDIF			# "$(WITH_LANG)"!=""
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(ULFCONV) -o $@.$(INPATH) $<
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.IF "$(WITH_LANG)"!=""
$(COMMONMISC)$/$(TARGET)$/%.mlf : $$(@:db).ulf
.ELSE			# "$(WITH_LANG)"!=""
$(COMMONMISC)$/$(TARGET)$/%.mlf : $$(@:b).ulf
.ENDIF			# "$(WITH_LANG)"!=""
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $@
    @$(WRAPCMD) $(ULFCONV) -o $@.$(INPATH) -t $(SOLARBINDIR)$/msi-encodinglist.txt $<
    @$(RENAME) $@.$(INPATH) $@
    @-$(RM) $@.$(INPATH)

$(COMMONMISC)$/$(TARGET)$/%.xrm : %.xrm
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(WRAPCMD) $(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

# dirty hack
# if local *.sdf file is missing
%.sdf:
    echo > $@

