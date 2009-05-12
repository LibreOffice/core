#*************************************************************************
# 
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#  
#  Copyright 2008 by Sun Microsystems, Inc.
# 
#  OpenOffice.org - a multi-platform office productivity suite
# 
#  $RCSfile: makefile.mk,v $
#  $Revision: 1.26.8.1 $
# 
#  This file is part of OpenOffice.org.
# 
#  OpenOffice.org is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License version 3
#  only, as published by the Free Software Foundation.
# 
#  OpenOffice.org is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
# 
#  You should have received a copy of the GNU Lesser General Public License
#  version 3 along with OpenOffice.org.  If not, see
#  <http://www.openoffice.org/license.html>
#  for a copy of the LGPLv3 License.
# 
# ************************************************************************/


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

.IF "$(USE_SHELL)"!="4nt"
ECHOQUOTE='
.ELSE
ECHOQUOTE=
.ENDIF


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
    echo $(ECHOQUOTE) \
    [assembly:System::Reflection::AssemblyVersion( "$(CLI_CPPUHELPER_NEW_VERSION)" )]; $(ECHOQUOTE) \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx
    echo $(ECHOQUOTE) \
    [assembly:System::Reflection::AssemblyKeyFile($(ASSEMBLY_KEY_X))]; $(ECHOQUOTE) \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx
    
    

#make sure we build cli_cppuhelper after the version changed
$(SHL1OBJS) : $(BIN)$/cli_cppuhelper.config

    

$(SIGN): $(SHL1TARGETN)
    $(WRAPCMD) sn.exe -R $(BIN)$/$(TARGET).dll	$(BIN)$/cliuno.snk	 && $(TOUCH) $@

#do not forget to deliver cli_cppuhelper.config. It is NOT embedded in the policy file.
.IF "$(CCNUMVER)" >= "001399999999"		
#.NET 2 and higher	
# If the x86 switch is ommitted then the system assumes the assembly to be MSIL.
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

