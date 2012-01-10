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
PRJNAME = cli_ure

# for dummy
TARGET = cliureversion.mk

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

ALLTAR : \
    $(BIN)$/cliureversion.mk 

#	INCVERSION

#always deliver a cliureversion.mk. It is needed for the packing process even for all other
#platforms. Therefore BUILD_FOR_CLI is not used here 
$(BIN)$/cliureversion.mk: version.txt 
    $(GNUCOPY) $< $@


#disabled because of #67482
#Create the config file that is used with the policy assembly 
#only if new published types have been added, the cliureversion.mk is written.
#INCVERSION .PHONY: version.txt incversions.txt  
#	$(PERL) $(PRJ)$/source$/scripts$/increment_version.pl \
#	$< $(BIN)$/cliureversion.mk $(SOLARBINDIR)$/unotype_statistics.txt




