#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 00:57:40 $
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

PRJ = ..$/..
PRJNAME = cli_ure

TARGET = cli_cppuhelper
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
LIBTARGET = NO
USE_DEFFILE = TRUE

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE : $(BIN)$/cliureversion.mk
use_shl_versions=

.IF "$(USE_SHELL)"!="4nt"
ECHOQUOTE='
.ELSE
ECHOQUOTE=
.ENDIF

.IF "$(BUILD_FOR_CLI)" != ""

ASSEMBLY_KEY="$(BIN)$/cliuno.snk"

ASSEMBLY_ATTRIBUTES = $(MISC)$/assembly_cppuhelper.cxx

POLICY_ASSEMBLY_FILE=$(BIN)$/$(CLI_CPPUHELPER_POLICY_ASSEMBLY).dll

ALLTAR : \
    $(ASSEMBLY_ATTRIBUTES) \
    $(POLICY_ASSEMBLY_FILE)	

ASSEMBLY_KEY_X=$(subst,\,\\ $(ASSEMBLY_KEY)) 

$(ASSEMBLY_ATTRIBUTES) : assembly.cxx $(BIN)$/cliuno.snk $(BIN)$/cliureversion.mk
    @+echo $(ASSEMBLY_KEY_X)
    $(GNUCOPY) -p assembly.cxx $@
    +echo $(ECHOQUOTE) \
    [assembly:System::Reflection::AssemblyVersion( "$(CLI_CPPUHELPER_NEW_VERSION)" )]; $(ECHOQUOTE) \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx	
    +echo $(ECHOQUOTE) \
    [assembly:System::Reflection::AssemblyKeyFile($(ASSEMBLY_KEY_X))]; $(ECHOQUOTE) \
    >> $(OUT)$/misc$/assembly_cppuhelper.cxx


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

# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
CFLAGS += -clr -AI $(OUT)$/bin -wd4339
#see  Microsoft Knowledge Base Article - 814472 
LINKFLAGS += -NOENTRY -NODEFAULTLIB:nochkclr.obj -INCLUDE:__DllMainCRTStartup@12

SLOFILES = \
        $(SLO)$/native_bootstrap.obj \
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

SHL1VERSIONMAP = msvc.map

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)


#do not forget to deliver cli_cppuhelper.config. It is NOT embedded in the policy file.
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_cppuhelper.config
    +$(WRAPCMD) AL.exe -out:$@ \
            -version:$(CLI_CPPUHELPER_POLICY_VERSION) \
            -keyfile:$(BIN)$/cliuno.snk \
            -link:$(BIN)$/cli_cppuhelper.config

#Create the config file that is used with the policy assembly
$(BIN)$/cli_cppuhelper.config: cli_cppuhelper_config $(BIN)$/cliureversion.mk 
    +$(PERL) $(PRJ)$/source$/scripts$/subst_template.pl \
    $< $@


.ENDIF

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(depend)"!=""
ALLDPC : $(ASSEMBLY_ATTRIBUTES) 
.ENDIF			# "$(depend)"!=""

#make sure we build cli_cppuhelper after the version changed
$(SHL1OBJS) : $(BIN)$/cli_cppuhelper.config 
    
