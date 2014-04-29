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




PRJ = ..$/..
PRJNAME = cli_ure

TARGET = cli_cppuhelper
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
LIBTARGET = NO
USE_DEFFILE = TRUE

# disable caching to avoid stale objects
# on version changes
CCACHE_DISABLE=TRUE
.EXPORT : CCACHE_DISABLE

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

use_shl_versions=

.IF "$(BUILD_FOR_CLI)" == ""
#do not even build the cxx files because they contain cli cpp
all:
.ELSE

.INCLUDE : $(BIN)$/cliureversion.mk

ASSEMBLY_KEY="$(BIN)$/cliuno.snk"

ASSEMBLY_ATTRIBUTES = $(MISC)$/assembly_cppuhelper.cxx

POLICY_ASSEMBLY_FILE=$(BIN)$/$(CLI_CPPUHELPER_POLICY_ASSEMBLY).dll

ASSEMBLY_KEY_X=$(subst,\,\\ $(ASSEMBLY_KEY))


LINKFLAGS += /delayload:cppuhelper3MSC.dll \
             /delayload:cppu3.dll \
             /delayload:sal3.dll

UWINAPILIB=

NO_OFFUH=TRUE
CPPUMAKERFLAGS =
UNOTYPES = \
    com.sun.star.lang.XSingleComponentFactory			\
    com.sun.star.loader.CannotActivateFactoryException 		\
    com.sun.star.container.XHierarchicalNameAccess		\
    com.sun.star.registry.CannotRegisterImplementationException	\
    com.sun.star.registry.XRegistryKey \
    com.sun.star.registry.XSimpleRegistry

#loader lock was solved as of VS 2005 (CCNUMVER = 0014..)
# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
.IF "$(CCNUMVER)" >= "001399999999"
CFLAGSCXX += -clr:oldSyntax -AI $(BIN) -wd4339
.ELSE
CFLAGSCXX += -clr -AI $(BIN) -wd4339
#see  Microsoft Knowledge Base Article - 814472
LINKFLAGS += -NOENTRY -NODEFAULTLIB:nochkclr.obj -INCLUDE:__DllMainCRTStartup@12
.ENDIF

SLOFILES = \
    $(SLO)$/native_bootstrap.obj \
    $(SLO)$/path.obj \
    $(SLO)$/assembly_cppuhelper.obj
    

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB)		\
    $(SALLIB)		\
    delayimp.lib \
    advapi32.lib \
    mscoree.lib \
    Advapi32.lib

.IF "$(CCNUMVER)" >= "001399999999"
SHL1STDLIBS += \
    msvcmrt.lib
.ENDIF

SHL1VERSIONMAP = msvc.map

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)


.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

SIGN= $(MISC)$/cppuhelper_is_signed_flag

ALLTAR: \
    $(POLICY_ASSEMBLY_FILE) \
    $(SIGN)



.IF "$(CCNUMVER)" >= "001399999999"
CFLAGSCXX += -clr:oldSyntax
.ENDIF

$(ASSEMBLY_ATTRIBUTES) : assembly.cxx $(BIN)$/cliuno.snk $(BIN)$/cliureversion.mk
    @echo $(ASSEMBLY_KEY_X)
    $(GNUCOPY) -p assembly.cxx $@
    echo \
    '[assembly:System::Reflection::AssemblyVersion( "$(CLI_CPPUHELPER_NEW_VERSION)" )];' \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx
    echo \
    '[assembly:System::Reflection::AssemblyKeyFile($(ASSEMBLY_KEY_X))];' \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx
    
    

#make sure we build cli_cppuhelper after the version changed
$(SHL1OBJS) : $(BIN)$/cli_cppuhelper.config

    

$(SIGN): $(SHL1TARGETN)
    $(WRAPCMD) sn.exe -R $(BIN)$/$(TARGET).dll	$(BIN)$/cliuno.snk	 && $(TOUCH) $@

#do not forget to deliver cli_cppuhelper.config. It is NOT embedded in the policy file.
.IF "$(CCNUMVER)" >= "001399999999"		
#.NET 2 and higher	
# If the x86 switch is omitted then the system assumes the assembly to be MSIL.
# The policy file is still found when an application tries to load an older
# cli_cppuhelper.dll but the system cannot locate it. It possibly assumes that the
# assembly is also 'MSIL'  like its policy file.
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_cppuhelper.config
    $(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_CPPUHELPER_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:$(BIN)$/cli_cppuhelper.config \
            -platform:x86
.ELSE
#.NET 1.1: platform flag not needed
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_cppuhelper.config
    $(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_CPPUHELPER_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:$(BIN)$/cli_cppuhelper.config		
.ENDIF			

#Create the config file that is used with the policy assembly
$(BIN)$/cli_cppuhelper.config: cli_cppuhelper_config $(BIN)$/cliureversion.mk 
    $(PERL) $(SOLARENV)$/bin$/clipatchconfig.pl \
    $< $@
    
.ENDIF			# "$(BUILD_FOR_CLI)" != ""

