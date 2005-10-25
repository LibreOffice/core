#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2005-10-25 11:14:19 $
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

# for dummy
TARGET = basetypes

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(USE_SHELL)"!="4nt"
ECHOQUOTE='
.ELSE
ECHOQUOTE=
.ENDIF

.IF "$(BUILD_FOR_CLI)" != ""

#!!! Always change version if code has changed. Provide a publisher
#policy assembly!!!
ASSEMBLY_VERSION="1.0.0.0"

ASSEMBLY_ATTRIBUTES = $(MISC)$/assembly_ure_$(TARGET).cs

ALLTAR : \
    $(BIN)$/cli_basetypes.dll

CSFILES = \
    uno$/Any.cs			\
    uno$/BoundAttribute.cs \
    uno$/ExceptionAttribute.cs	\
    uno$/ParameterizedTypeAttribute.cs	\
    uno$/TypeParametersAttribute.cs \
    uno$/TypeArgumentsAttribute.cs \
    uno$/OnewayAttribute.cs	\
    uno$/PolymorphicType.cs \
    $(ASSEMBLY_ATTRIBUTES)

$(ASSEMBLY_ATTRIBUTES) : assembly.cs makefile.mk $(BIN)$/cliuno.snk
    $(GNUCOPY) -p assembly.cs $@
    +echo $(ECHOQUOTE) \
    [assembly:System.Reflection.AssemblyVersion( $(ASSEMBLY_VERSION))] \
    [assembly:System.Reflection.AssemblyKeyFile(@"$(BIN)$/cliuno.snk")]$(ECHOQUOTE) \
    >> $@

$(BIN)$/cli_basetypes.dll : $(CSFILES)
    +$(CSC) $(CSCFLAGS) \
        -target:library \
        -out:$@ \
        -reference:System.dll \
        $(CSFILES)
    @echo "If code has changed then provide a policy assembly and change the version!"

.ENDIF
    

