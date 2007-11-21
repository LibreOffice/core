#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 18:52:33 $
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

TARGET = climaker
TARGETTYPE = CUI
LIBTARGET = NO
ENABLE_EXCEPTIONS = TRUE

# disable caching to avoid stale objects
# on version changes
CCACHE_DISABLE=TRUE
.EXPORT : CCACHE_DISABLE

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(BUILD_FOR_CLI)" != ""

NO_OFFUH=TRUE
CPPUMAKERFLAGS =

UNOTYPES = \
    com.sun.star.uno.TypeClass					\
    com.sun.star.uno.XAggregation					\
    com.sun.star.uno.XWeak						\
    com.sun.star.uno.XComponentContext				\
    com.sun.star.lang.XTypeProvider					\
    com.sun.star.lang.XInitialization				\
    com.sun.star.lang.XComponent					\
    com.sun.star.lang.XMultiComponentFactory			\
    com.sun.star.lang.XMultiServiceFactory			\
    com.sun.star.lang.XSingleComponentFactory			\
    com.sun.star.container.XSet					\
    com.sun.star.container.XHierarchicalNameAccess			\
    com.sun.star.loader.XImplementationLoader			\
    com.sun.star.registry.XSimpleRegistry				\
    com.sun.star.registry.XRegistryKey				\
    com.sun.star.reflection.XTypeDescriptionEnumerationAccess	\
    com.sun.star.reflection.XConstantTypeDescription		\
    com.sun.star.reflection.XConstantsTypeDescription		\
    com.sun.star.reflection.XIndirectTypeDescription		\
    com.sun.star.reflection.XEnumTypeDescription			\
    com.sun.star.reflection.XInterfaceTypeDescription2		\
    com.sun.star.reflection.XInterfaceMethodTypeDescription		\
    com.sun.star.reflection.XInterfaceAttributeTypeDescription2	\
    com.sun.star.reflection.XCompoundTypeDescription		\
    com.sun.star.reflection.XServiceTypeDescription2		\
    com.sun.star.reflection.XSingletonTypeDescription2		\
    com.sun.star.reflection.XStructTypeDescription

CFLAGSCXX +=-AI$(BIN)


# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
.IF "$(COMEX)"=="10"
CFLAGSCXX += -clr:noAssembly -wd4339
.ELSE
CFLAGSCXX += -clr:oldSyntax -LN -wd4339 -wd4715
.ENDIF

OBJFILES = \
    $(OBJ)$/climaker_app.obj	\
    $(OBJ)$/climaker_emit.obj

APP1TARGET = $(TARGET)
APP1OBJS = $(OBJFILES)


APP1STDLIBS = \
    $(CPPUHELPERLIB)		\
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

.IF "$(CCNUMVER)" >= "001399999999"
APP1STDLIBS += \
    msvcmrt.lib
.ENDIF

.ENDIF



.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE :  target.mk

.IF "$(BUILD_FOR_CLI)" != ""

$(OBJFILES): $(BIN)$/cli_basetypes.dll

ALLTAR : $(BIN)$/climaker.exe.config

$(BIN)$/climaker.exe.config : climaker.exe.config
    $(GNUCOPY) -f $? $@
.IF "$(USE_SHELL)"!="4nt"
    chmod +x $@
.ENDIF

.ENDIF




