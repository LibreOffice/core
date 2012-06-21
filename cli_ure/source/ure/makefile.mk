#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    

