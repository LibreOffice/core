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

TARGET = cli_uno_glue
ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(USE_SHELL)"!="4nt"
ECHOQUOTE='
# emacs is a fool: '
.ELSE
ECHOQUOTE=
.ENDIF

.IF "$(ENABLE_MONO)" != "YES"
dummy:
     @echo "Mono binding disabled - skipping ..."
.ELSE

.INCLUDE : $(BIN)$/cliureversion.mk

ASSEMBLY_KEY="$(BIN)$/cliuno.snk"

ASSEMBLY_ATTRIBUTES = $(MISC)$/assembly_cppuhelper.cs

POLICY_ASSEMBLY_FILE=$(BIN)$/$(CLI_CPPUHELPER_POLICY_ASSEMBLY).dll

ALLTAR : \
    $(SHL1TARGETN) \
    $(ASSEMBLY_ATTRIBUTES) \
    $(BIN)$/cli_cppuhelper.dll \
    $(BIN)$/cli_cppuhelper.config \
    $(POLICY_ASSEMBLY_FILE)

CSFILES = \
    managed_bootstrap.cs \
    $(ASSEMBLY_ATTRIBUTES)

$(ASSEMBLY_ATTRIBUTES) .PHONY: assembly.cs $(BIN)$/cliuno.snk $(BIN)$/cliureversion.mk
    $(GNUCOPY) -p assembly.cs $@
    +echo $(ECHOQUOTE) \
    [assembly:System.Reflection.AssemblyVersion( "$(CLI_CPPUHELPER_NEW_VERSION)" )] $(ECHOQUOTE) \
    $(ECHOQUOTE) [assembly:System.Reflection.AssemblyKeyFile($(ASSEMBLY_KEY))] $(ECHOQUOTE) \
    >> $@

EXTERNAL_DIR=$(PRJ)$/..$/external/cli

$(OUT)$/bin$/cli_uretypes.dll : $(EXTERNAL_DIR)$/cli_uretypes.dll
    +$(COPY) $< $@

$(BIN)$/cli_cppuhelper.dll : $(CSFILES) $(OUT)$/bin$/cli_uretypes.dll
    +$(CSC) $(CSCFLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(OUT)$/bin$/cli_uretypes.dll \
        -reference:System.dll \
        $(CSFILES)
    @echo "If code has changed then provide a policy assembly and change the version!"

CFLAGS += $(MONO_CFLAGS)

SLOFILES= \
    $(SLO)$/native_glue.obj

SHL1OBJS = $(SLOFILES)

SHL1STDLIBS = \
    $(SALLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB)

SHL1STDLIBS += $(MONO_LIBS)

SHL1TARGET = $(TARGET)

$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_cppuhelper.config
    $(WRAPCMD) $(AL) -out:$@ \
            -version:$(CLI_CPPUHELPER_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:cli_cppuhelper.config,$(BIN)$/cli_cppuhelper.config

#Create the config file that is used with the policy assembly
$(BIN)$/cli_cppuhelper.config: ../native/cli_cppuhelper_config $(BIN)$/cliureversion.mk
    $(PERL) $(SOLARENV)$/bin$/clipatchconfig.pl \
    $< $@

.ENDIF

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk
