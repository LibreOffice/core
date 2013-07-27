#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        --assembly-company "Apache OpenOffice" \
        --assembly-description "This assembly contains metadata for the Apache OpenOffice API." \
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
    
