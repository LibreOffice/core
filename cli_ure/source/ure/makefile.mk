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

PRJ = ..$/..
PRJNAME = cli_ure

# for dummy
TARGET = ure

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(BUILD_FOR_CLI)" != ""

.INCLUDE : $(BIN)$/cliureversion.mk

ASSEMBLY_ATTRIBUTES = $(MISC)$/assembly_ure_$(TARGET).cs

POLICY_ASSEMBLY_FILE=$(BIN)$/$(CLI_URE_POLICY_ASSEMBLY).dll
ALLTAR : \
    $(BIN)$/cli_ure.dll \
    $(POLICY_ASSEMBLY_FILE)

.IF "$(CCNUMVER)" >= "001399999999"
CSCFLAGS+=-keyfile:"$(BIN)$/cliuno.snk"
.ENDIF

CSFILES = \
    uno$/util$/DisposeGuard.cs					\
    uno$/util$/WeakAdapter.cs					\
    uno$/util$/WeakBase.cs						\
    uno$/util$/WeakComponentBase.cs	\
    $(ASSEMBLY_ATTRIBUTES)

.IF "$(CCNUMVER)" <= "001399999999"
$(ASSEMBLY_ATTRIBUTES) : assembly.cs makefile.mk $(BIN)$/cliuno.snk $(BIN)$/cliureversion.mk 
    $(GNUCOPY) -p assembly.cs $@
    echo \
    '[assembly:System.Reflection.AssemblyVersion( "$(CLI_URE_NEW_VERSION)")] \
    [assembly:System.Reflection.AssemblyKeyFile(@"$(BIN)$/cliuno.snk")]' \
    >> $@
.ELSE
$(ASSEMBLY_ATTRIBUTES) : assembly.cs makefile.mk $(BIN)$/cliuno.snk $(BIN)$/cliureversion.mk 
    $(GNUCOPY) -p assembly.cs $@
    echo \
    '[assembly:System.Reflection.AssemblyVersion( "$(CLI_URE_NEW_VERSION)")]' \
    >> $@
.ENDIF

$(BIN)$/cli_ure.dll : $(CSFILES) $(BIN)$/cli_uretypes.dll $(BIN)$/cliureversion.mk 
    $(CSC) $(CSCFLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(OUT)$/bin$/cli_uretypes.dll \
        -reference:System.dll \
        $(CSFILES)
    @echo "If code has changed then provide a policy assembly and change the version!"


#do not forget to deliver cli_ure.config. It is NOT embedded in the policy file.
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_ure.config
    $(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_URE_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:$(BIN)$/cli_ure.config

#Create the config file that is used with the policy assembly
$(BIN)$/cli_ure.config: cli_ure_config $(BIN)$/cliureversion.mk 
    $(PERL) $(SOLARENV)$/bin$/clipatchconfig.pl \
    $< $@


.ENDIF
    

