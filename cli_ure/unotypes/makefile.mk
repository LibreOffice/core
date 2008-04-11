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
# $Revision: 1.18 $
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
PRJNAME = cli_ure

# for dummy
TARGET = unotypes

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(BUILD_FOR_CLI)" != ""

.INCLUDE : $(BIN)$/cliureversion.mk

POLICY_ASSEMBLY_FILE=$(BIN)/$(CLI_TYPES_POLICY_ASSEMBLY).dll

ALLTAR : \
    $(OUT)$/bin$/cli_types.dll \
    $(POLICY_ASSEMBLY_FILE)



CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF


#When changing the assembly version then this must also be done in scp2
$(OUT)$/bin$/cli_types.dll : $(OUT)$/bin$/climaker.exe $(SOLARBINDIR)$/types.rdb $(BIN)$/cliureversion.mk
    $(WRAPCMD) $(OUT)$/bin$/climaker.exe $(CLIMAKERFLAGS) \
        --out $@ \
                --keyfile $(BIN)$/cliuno.snk \
        --assembly-version $(CLI_TYPES_NEW_VERSION) \
        --assembly-description "This assembly contains metadata for the StarOffice/OpenOffice.org API." \
        --assembly-company "OpenOffice.org" \
        $(SOLARBINDIR)$/types_doc.rdb

#		--assembly-copyright "2003" \

#do not forget to deliver cli_types.config. It is NOT embedded in the policy file.
# iz62624: Add dependency for "$(OUT)$/bin$/cli_types.dll" because climaker locks cliuno.mk.
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_types.config $(OUT)$/bin$/cli_types.dll
    $(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_TYPES_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:$(BIN)$/cli_types.config

#Create the config file that is used with the policy assembly
$(BIN)$/cli_types.config: cli_types_config $(BIN)$/cliureversion.mk 
    $(PERL) $(PRJ)$/source$/scripts$/subst_template.pl \
    $< $@


.ENDIF
