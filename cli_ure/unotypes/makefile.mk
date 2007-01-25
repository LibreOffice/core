#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:44:25 $
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
