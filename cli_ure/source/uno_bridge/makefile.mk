#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:32:11 $
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

TARGET = cli_uno
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
USE_DEFFILE = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(COM)$(COMEX)" == "MSC11" || "$(COM)$(COMEX)" == "MSC10"

.IF "$(COM)" == "MSC"
# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
.IF "$(CCNUMVER)" <= "001399999999"
CFLAGSCXX += -clr -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ELSE
CFLAGSCXX += -clr:oldSyntax -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ENDIF

.IF "$(debug)" != ""
CFLAGS += -Ob0
.ENDIF
.ENDIF




.IF "$(CCNUMVER)" <= "001399999999"
#see  Microsoft Knowledge Base Article - 814472
LINKFLAGS += -NOENTRY -NODEFAULTLIB:nochkclr.obj -INCLUDE:__DllMainCRTStartup@12
.ENDIF
# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/cli_environment.obj	\
    $(SLO)$/cli_bridge.obj		\
    $(SLO)$/cli_data.obj		\
    $(SLO)$/cli_proxy.obj		\
    $(SLO)$/cli_uno.obj

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1STDLIBS = \
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

.IF "$(CCNUMVER)" >= "001399999999"
SHL1STDLIBS += \
    msvcmrt.lib
.ENDIF

SHL1VERSIONMAP = bridge_exports.map

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
