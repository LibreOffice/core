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

MKFILENAME:=RULES.MK

$(OBJ)/%.obj : %.cxx
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(CFLAGSINCXX)$(PWD)/$*.cxx
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
.IF "$(noadjust)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(noadjust)"==""
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx 
.ELSE
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(OBJ)/%.obj : %.cpp
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(CFLAGSINCXX)$(PWD)/$*.cpp
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cpp 
.ELSE
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cpp
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(OBJ)/%.obj : %.cc
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(CFLAGSINCXX)$(PWD)/$*.cc
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @+$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cc 
.ELSE
    $(COMMAND_ECHO)$(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cc )
.ENDIF
.ENDIF

$(SLO)/%.obj : %.cxx
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) -E  $(CFLAGSINCXX)$(PWD)/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CAPTURE_COMMAND) $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(CFLAGSINCXX)$(PWD)/$*.cxx $(CAPTURE_OUTPUT)
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"=="" && "$(CAPTURE_COMMAND)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CAPTURE_COMMAND) $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx $(CAPTURE_OUTPUT)
.ELSE
    $(COMMAND_ECHO)$(CAPTURE_COMMAND) $(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cxx $(CAPTURE_OUTPUT)
.ENDIF		    # "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(nodep)"==""
# fixme: write to file
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(SLO)/%.obj : %.cpp
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(PWD)/$*.cpp
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(CFLAGSINCXX)$(PWD)/$*.cpp
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cpp
.ELSE
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(PWD)/$*.cpp
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(SLO)/%.obj : $(MISC)/%.cxx
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(CFLAGSINCXX)$(MISC)/$*.cxx
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(MISC)/$*.cxx
.ELSE
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(MISC)/$*.cxx
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $@ ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(SLO)/%.obj : $(MISC)/%.cc
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)/$*.cc
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXSLO) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(CFLAGSINCXX)$(MISC)/$*.cc
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @+$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
    $(COMMAND_ECHO)$(CXX) @$(mktmp $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(CFLAGSINCXX)$(MISC)/$*.cc )
.ENDIF

$(OBJ)/%.obj : $(MISC)/%.cxx
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) -E  $(CFLAGSINCXX)$(MISC)/$*.cxx
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSCXXOBJ) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(CFLAGSINCXX)$(MISC)/$*.cxx
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(MISC)/$*.cxx
.ELSE
    $(COMMAND_ECHO)$(CXX) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $(!eq,$(EXCEPTIONSFILES),$(subst,$@, $(EXCEPTIONSFILES)) $(LOCAL_EXCEPTIONS_FLAGS) $(GLOBAL_EXCEPTIONS_FLAGS)) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(CFLAGSINCXX)$(MISC)/$*.cxx
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(OBJ)/%.obj : %.c
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.c
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.c
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
.IF "$(noadjust)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF			# "$(noadjust)"==""
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $*.c
.ELSE
  .IF "$(VERBOSE)" == "TRUE"
    -@echo Cflags: $(CFLAGS) $(INCLUDE)
  .ENDIF
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $*.c
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(OBJ)/%.obj : $(MISC)/%.c
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(MISC)/$*.c
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(MISC)/$*.c
.ELSE
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(OBJ)/$*.obj $(MISC)/$*.c
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

$(SLO)/%.obj : $(MISC)/%.c
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(MISC)/$*.c
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(MISC)/$*.c 
.ELSE
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $(MISC)/$*.c
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
# avoid _version objects (see tg_shl.mk) - quite hacky...
    @noop $(!eq,$@,$(@:s/_version.obj/.obj/) $(NULL) $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/)))
.ENDIF			# "$(nodep)"==""

$(SLO)/%.obj : %.c
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(GUI)"=="UNX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $*.c
.IF "$(OS)$(COM)"=="SOLARISC52" && "$(product)"=="full" && "$(debug)"==""
    $(COMMAND_ECHO)$(ADJUSTVISIBILITY) -p $(@:s/.obj/.o/)
.ENDIF          # "$(OS)"=="SOLARIS" && "$(product)"=="full" && "$(debug)"==""
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE
    @@-$(RM) $@
.IF "$(COM)"=="GCC"
       $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $*.c 
.ELSE
       $(COMMAND_ECHO)$(CC) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ)$(SLO)/$*.obj $*.c
.ENDIF
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""

# Objective-C files
$(OBJ)/%.obj : %.m
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.m
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.m
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(OBJ)/%.obj : $(MISC)/%.m
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(MISC)/$*.m
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)/%.obj : $(MISC)/%.m
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(MISC)/$*.m
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# Objective-C files
$(SLO)/%.obj : %.m
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objc) $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(OBJCFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $*.m
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C files is available for this platform"
.ENDIF		"$(OS)"=="MACOSX"

# dependencies c / c++

not_existing/s_%.dpcc : %.c;@noop $(assign all_local_slo+:=$<)
not_existing/o_%.dpcc : %.c;@noop $(assign all_local_obj+:=$<)

# Objective-C++ files
$(OBJ)/%.obj : %.mm
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
.IF "$(TEST)"!=""
    $(COMMAND_ECHO)$(objcpp) $(CFLAGS) $(INCLUDE) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) -E $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.mm
.ELSE
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objcpp) $(CFLAGS) $(INCLUDE) $(CFLAGSCC) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $*.mm
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.ENDIF
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF			# "$(OS)"=="MACOSX"

# Objective-C++ files
$(OBJ)/%.obj : $(MISC)/%.mm
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objcpp) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(OBJCXXFLAGS) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(OBJ)/$*.o $(MISC)/$*.mm
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< ) > $(MISC)/o_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=o_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF			# "$(OS)"=="MACOSX"

# Objective-C++ files
$(SLO)/%.obj : $(MISC)/%.mm
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(INPATH)/misc/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objcpp) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(OBJCXXFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $(MISC)/$*.mm
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF			# "$(OS)"=="MACOSX"

# Objective-C++ files
$(SLO)/%.obj : %.mm
    @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(OS)"=="MACOSX"
    @$(RM) $@ $(@:s/.obj/.o/)
    $(COMMAND_ECHO)$(objcpp) $(CFLAGS) $(INCLUDE) $(CFLAGSCXX) $(OBJCXXFLAGS) $(CFLAGSSLO) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSAPPEND) $(CFLAGSOUTOBJ) $(SLO)/$*.o $*.mm
    @$(IFEXIST) $(@:s/.obj/.o/) $(THEN) $(TOUCH) $@ $(FI)
.IF "$(nodep)"==""
    $(COMMAND_ECHO)$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< ) > $(MISC)/s_$(@:f:s/.obj/.dpcc/)
    @noop $(assign DEPS_MADE+:=s_$(@:f:s/.obj/.dpcc/))
.ENDIF			# "$(nodep)"==""
.ELSE			# "$(OS)"=="MACOSX"
    @echo "No recipe for compiling Objective-C++ files is available for this platform"
.ENDIF			# "$(OS)"=="MACOSX"

$(MISC)/%.dpslo :
    @echo "Making:   " $(@:f)
    @@$(RM) $@
    @@-$(CDD) $(MISC) && $(!null,$(all_local_slo) $(TYPE:s/+//) echo #) $(foreach,i,$(all_local_slo:b:+".dpcc") s_$(i)) > $(@:f)
    @@-$(CDD) $(MISC) && $(!null,$(all_misc_slo) $(TYPE:s/+//) echo #) $(foreach,i,$(all_misc_slo:b:+".dpcc") s_$(i)) >> $(@:f)
    @$(TYPE) $(mktmp $(foreach,i,$(all_local_slo:b:+".dpcc") $(@:s#\#/#:^"\n") : $(MISC:s#\#/#)/s_$i) $(foreach,i,$(all_misc_slo:b:+".dpcc") $(@:s#\#/#:^"\n") : $(MISC:s#\#/#)/s_$i)) >> $@
    @$(TYPE) $(mktmp $(TARGET)_known_dpcc+=$(all_local_slo:b:+".dpcc":^"s_") $(all_misc_slo:b:+".dpcc":^"s_")) >> $@

$(MISC)/%.dpobj :
    @echo "Making:   " $(@:f)
    @@$(RM) $@
    @@-$(CDD) $(MISC) && $(!null,$(all_local_obj) $(TYPE:s/+//) echo #) $(foreach,i,$(all_local_obj:b:+".dpcc") o_$(i)) > $(@:f)
    @@-$(CDD) $(MISC) && $(!null,$(all_misc_obj) $(TYPE:s/+//) echo #) $(foreach,i,$(all_misc_obj:b:+".dpcc") o_$(i)) >> $(@:f)
    @$(TYPE) $(mktmp $(foreach,i,$(all_local_obj:b:+".dpcc") $(@:s#\#/#:^"\n") : $(MISC:s#\#/#)/o_$i) $(foreach,i,$(all_misc_obj:b:+".dpcc") $(@:s#\#/#:^"\n") : $(MISC:s#\#/#)/o_$i)) >> $@
    @$(TYPE) $(mktmp $(TARGET)_known_dpcc+=$(all_local_obj:b:+".dpcc":^"s_") $(all_misc_obj:b:+".dpcc":^"s_")) >> $@

# see also %.dpslo 
not_existing/s_%.dpcc : %.cxx;@noop $(assign all_local_slo+:=$<)
not_existing/o_%.dpcc : %.cxx;@noop $(assign all_local_obj+:=$<)

not_existing/s_%.dpcc : %.cpp;@noop $(assign all_local_slo+:=$<)
not_existing/o_%.dpcc : %.cpp;@noop $(assign all_local_obj+:=$<)

not_existing/s_%.dpcc : $(MISC)/%.c;@noop $(assign all_misc_slo+:=$<)
not_existing/o_%.dpcc : $(MISC)/%.c;@noop $(assign all_misc_obj+:=$<)

not_existing/s_%.dpcc : $(MISC)/%.cxx;@noop $(assign all_misc_slo+:=$<)
not_existing/o_%.dpcc : $(MISC)/%.cxx;@noop $(assign all_misc_obj+:=$<)

not_existing/s_%.dpcc : %.mm;@noop $(assign all_local_slo+:=$<)
not_existing/o_%.dpcc : %.mm;@noop $(assign all_local_obj+:=$<)

not_existing/s_%.dpcc : $(MISC)/%.mm;@noop $(assign all_misc_slo+:=$<)
not_existing/o_%.dpcc : $(MISC)/%.mm;@noop $(assign all_misc_obj+:=$<)

# dependencies objective-c

$(MISC)/s_%.dpcc : %.m
    @@-$(RM) $@
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
    @echo $@ : $(SLO)/$(<:b).obj >> $@

$(MISC)/o_%.dpcc : %.m
    @@-$(RM) $@
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< > $@
    @echo $@ : $(OBJ)/$(<:b).obj >> $@

$(MISC)/s_%.dpcc : $(MISC)/%.m
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | sed s\#$(MISC)/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"
    @echo $@ : $(SLO)/$(<:b).obj >> $@

$(MISC)/o_%.dpcc : $(MISC)/%.m
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | sed s\#$(MISC)/\#\# > $@
.ELSE			# "$(GUI)"=="UNX"
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $< | $(SED) s/$(MISC:s/\/\\/)\\// > $@
.ENDIF			# "$(GUI)"=="UNX"
    @echo $@ : $(OBJ)/$(<:b).obj >> $@

# dependency dummy for *.s files

$(MISC)/s_%.dpcc : %.s
    @@-$(RM) $@
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $< > $@
    @echo $@ : $(SLO)/$(<:b).obj >> $@

$(MISC)/s_%.dpcc : $(SLO)/%.obj
    @noop $(assign DEPS_MISSING+:=$(@:f))

$(MISC)/o_%.dpcc : $(OBJ)/%.obj
    @noop $(assign DEPS_MISSING+:=$(@:f))

# dependencies script files

$(MISC)/%.dpsc :
    @@-$(RM) $@
    @@-$(MKDIR) $(MISC)/{$(subst,$(@:d:d:d), $(@:d:d))}
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(PAR)/{$(subst,$(@:d:d:d), $(@:d:d))} -o.par -D{$(subst,$(@:d:d:d:u), $(@:d:d:u))}_PRODUCT $(CDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(*:b).scp > $@
    @echo $@ : $(PAR)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).par  >> $@
    @echo $(PAR)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).par : $(*:b).scp >> $@

# dependencies rc files (native resources for windows)

$(MISC)/%.dprc : 
    @@-$(RM) $@
    $(COMMAND_ECHO)$(MAKEDEPEND) -f - -p$(RES) -o.res $(RCLANGFLAGS_{$(subst,$(@:d:d:d:u), $(@:d:d:u))}:u:s/ //) $(CDEFS) -DDLLPOSTFIX=$(DLLPOSTFIX) -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) -I$(SOLARENV)/inc $(*:b).rc >> $@
#	@echo $@ : $(RES)/{$(subst,$(@:d:d:d), $(@:d:d))}/$(*:b).res  >> $@
    @echo $@ : $(RES)/$(*:b).res  >> $@

$(MISC)/%.dpr :
    @@noop
.IF "$(nodep)"==""
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
    $(COMMAND_ECHO)dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_srs_deps=true $(DEPSRSFILES)
    $(COMMAND_ECHO)-$(TYPE) $(MISC)/$(TARGET).*.dprr >> $@
.ENDIF			# "$(nodep)"==""

$(MISC)/%.dpz :
    @@noop
.IF "$(nodep)"==""
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
    $(COMMAND_ECHO)dmake $(MFLAGS) $(MAKEFILE) $(CALLMACROS) make_zip_deps=true $(ZIPDEPFILES)
    $(COMMAND_ECHO)$(TYPE) $(ZIPDEPFILES) $(mktmp $(NULL)) >> $@
    @echo zipdep_langs=$(alllangiso) >> $@
    @@-$(RM) $(ZIPDEPFILES)
.ENDIF			# "$(nodep)"==""

# Dependencies fuer java - Files

$(MISC)/%.dpj :
    @echo "Making:   " $(@:f)
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

$(SLO)/%.obj : %.asm
       @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(COM)"=="GCC"
.IF "$(ASM)"=="ml" || "$(ASM)"=="ml64"
       $(COMMAND_ECHO)$(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)/$*.obj $*.asm
       @@-$(RM) $*.err
.ELSE			# "$(ASM)"=="ml"
        @@-$(RM) $@
        $(COMMAND_ECHO)$(ASM) $(AFLAGS) $*.asm,$(SLO)/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ELSE
.IF "$(COM)"=="WTC"
        $(COMMAND_ECHO)$(ASM) $(AFLAGS) -D$(COM) $*.asm -fo=$(SLO)/$*.obj
        @-$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml" || "$(ASM)"=="ml64"
    $(COMMAND_ECHO)$(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)/$*.obj $*.asm
    @-$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE			# "$(ASM)"=="ml"
    @@-$(RM) $@
    $(COMMAND_ECHO)$(ASM) $(AFLAGS) $*.asm,$(SLO)/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF
.ENDIF

$(OBJ)/%.obj : %.asm
       @echo $(COMPILE_ECHO_SWITCH) Compiling: $(PRJNAME)/$(PATH_IN_MODULE)/$(COMPILE_ECHO_FILE)
.IF "$(COM)"=="GCC"
.IF "$(ASM)"=="ml"
       $(COMMAND_ECHO)$(ASM) $(AFLAGS) -D$(COM) /Fo$(OBJ)/$*.obj $*.asm
       @@-$(RM) $*.err
.ELSE			# "$(ASM)"=="ml"
        @@-$(RM) $@
        $(COMMAND_ECHO)$(ASM) $(AFLAGS) $*.asm,$(OBJ)/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ELSE
.IF "$(COM)"=="MSC"
.IF "$(ASM)"=="ml"
    $(COMMAND_ECHO)$(ASM) $(AFLAGS) -D$(COM) /Fo$(SLO)/$*.obj $*.asm
    @-$(IFEXIST) $*.err $(THEN) $(RM:s/+//) $*.err $(FI)
.ELSE			# "$(ASM)"=="ml"
    @$(IFEXIST) $@ $(THEN) $(RM:s/+//) $@ >& $(NULLDEV) $(FI)
    $(COMMAND_ECHO)$(ASM) $(AFLAGS) $*.asm,$(OBJ)/$*.obj;
.ENDIF			# "$(ASM)"=="ml"
.ENDIF			 "$(COM)"=="MSC"
.ENDIF

$(OUT)/ucr/$(IDLPACKAGE)/%.urd : %.idl
        @noop $(assign all_outdated_idl+:=$<)

$(OUT)/ucrdoc/$(IDLPACKAGE)/%.urd : %.idl
        @noop $(assign all_outdated_doc_idl+:=$<)

# make *.xml descriptions available in $(MISC)
$(MISC)/%$($(WINVERSIONNAMES)_MAJOR).xml : %.xml
    $(COMMAND_ECHO)$(COPY) $< $@

# dummy rule to make sure xml file is in place when used in settings.mk
$(MISC)/%.mk : $(MISC)/%$($(WINVERSIONNAMES)_MAJOR).xml
    @$(TOUCH) $@
    @echo XML2MK_FILES += $(@:b) >> $@

#generate descriptions from xml
$(MISC)/%$($(WINVERSIONNAMES)_MAJOR)_description.cxx : $(MISC)/%$($(WINVERSIONNAMES)_MAJOR).xml 
    xml2cmp -func $(MISC)/$*$($(WINVERSIONNAMES)_MAJOR)_description.cxx $<

#generate private rdb
$(BIN)/%.rdb: $(MISC)/%$($(WINVERSIONNAMES)_MAJOR).xml
    $(COMMAND_ECHO)$(RDBMAKER) -BUCR -O$(BIN)/$*.rdb @$(mktmp $(foreach,i,$($(@:b)_XML2CMPTYPES) -T$i ) $(COMPRDB))

#strip dos lineends
$(MISC)/%.sh : %.sh
    @@-$(RM) -f $@
    @tr -d "\015" < $< > $@

# merge targets
.IF "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.ulf : %.ulf
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(ULFEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

$(COMMONMISC)/$(MYPATH)/%.xrm : %.xrm
    $(COMMAND_ECHO)-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    @echo trysdf = $(TRYSDF)
    $(COMMAND_ECHO)$(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

#$(COMMONMISC)/$(TARGET)/%.xrm : %.xrm
#    -$(MKDIRHIER) $(@:d)
#    -$(RM) $@
#	$(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
#    $(RENAME) $@.$(INPATH) $@
#    -$(RM) $@.$(INPATH)
#
#$(COMMONMISC)/%.xrm : %.xrm
#    -$(MKDIR) $(@:d)
#    -$(RM) $@
#	$(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
#    $(RENAME) $@.$(INPATH) $@
#    -$(RM) $@.$(INPATH)
.ENDIF			# "$(WITH_LANG)"!=""

.IF "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.jlf : $$(@:db).ulf
.ELSE			# "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.jlf : $$(@:b).ulf
.ENDIF			# "$(WITH_LANG)"!=""
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(ULFCONV) -o $@.$(INPATH) $<
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

.IF "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.mlf : $$(@:db).ulf
.ELSE			# "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.mlf : $$(@:b).ulf
.ENDIF			# "$(WITH_LANG)"!=""
    @-$(MKDIRHIER) $(@:d)
    @-$(RM) $@
    @$(ULFCONV) -o $@.$(INPATH) -t $(SOLARBINDIR)/msi-encodinglist.txt $<
    @$(RENAME) $@.$(INPATH) $@
    @-$(RM) $@.$(INPATH)

.IF "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.uulf : $$(@:db).ulf
.ELSE			# "$(WITH_LANG)"!=""
$(COMMONMISC)/$(TARGET)/%.uulf : $$(@:b).ulf
.ENDIF			# "$(WITH_LANG)"!=""
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    @$(COPY) $< $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@
    @-$(RM) $@.$(INPATH)

POLOCATION:=$(SRCDIR)$/translations/source
PORELPATH:=$(PRJNAME)$/$(PATH_IN_MODULE).po

$(COMMONMISC)/$(TARGET)/%.xrm : %.xrm
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(XRMEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(mktmp $(foreach,lang,$(subst,en-US, $(WITH_LANG)) $(POLOCATION)/$(lang)/$(PORELPATH))) -l all
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)
