#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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

PRJ = ..
PRJNAME = unoil

# for dummy
TARGET = unotypes

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

#.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.INCLUDE : version.txt

POLICY_ASSEMBLY_FILE=$(BIN)/$(CLI_OOOTYPES_POLICY_ASSEMBLY).dll

.IF "$(BUILD_FOR_CLI)" != ""

ALLTAR : \
    $(BIN)$/clioootypesversion.mk \
    $(BIN)$/cli_oootypes.dll \
    $(POLICY_ASSEMBLY_FILE)

.ELSE

ALLTAR : \
    $(BIN)$/clioootypesversion.mk 

.ENDIF

    
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF

RDB = $(SOLARBINDIR)$/offapi.rdb
EXTRA_RDB = $(SOLARBINDIR)$/udkapi.rdb

$(BIN)/cli_oootypes.dll : $(RDB) $(EXTRA_RDB) version.txt
        $(CLIMAKER) $(CLIMAKERFLAGS) \
        --out $@ \
         --assembly-version $(CLI_OOOTYPES_NEW_VERSION) \
        --assembly-company "OpenOffice.org" \
        --assembly-description "This assembly contains metadata for the StarOffice/OpenOffice.org API." \
        -X $(EXTRA_RDB) \
        -r $(SOLARBINDIR)$/cli_uretypes.dll \
        --keyfile $(SOLARBINDIR)$/cliuno.snk \
        $(RDB)
        

#Create the config file that is used with the policy assembly
$(BIN)$/cli_oootypes.config: cli_oootypes_config version.txt
    $(PERL) $(SOLARENV)$/bin$/clipatchconfig.pl \
    $< $@

$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_oootypes.config $(BIN)$/cli_oootypes.dll
    $(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_OOOTYPES_POLICY_VERSION) \
            -keyfile:$(SOLARBINDIR)$/cliuno.snk \
            -link:$(BIN)$/cli_oootypes.config

#always deliver a clioootypesversion.mk. It is needed for the packing process even for all other
#platforms. Therefore BUILD_FOR_CLI is not used here 
$(BIN)$/clioootypesversion.mk: version.txt
    $(GNUCOPY) $< $@
    
