#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: msdev.mk,v $
#
# $Revision: 1.3 $
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

.SUFFIXES: .urd .idl

# the root directory of the udk
# also adjust include statements if this is changed
ROOT=i:\udk100

include i:\udk100\msdev\defines.mk

# the project directory under runtime
PROJECT=datefunc

# the target of this makefile
TARGET=$(BINOUT)\datefunc.dll

CPPFLAGS = \
    -I$(RUNTIME)\vos\inc		\
    -I$(RUNTIME)\sal\inc		\
    -I$(RUNTIME)\cppu\inc		\
    -I$(RUNTIME)\cppuhelper\inc	\
    -I$(RUNTIME)\stoc\inc		\
    -I$(UNOHEADER_OUT)

OBJS=$(OBJOUT)\datefunc.obj

# flags and macros
include i:\udk100\msdev\flags.mk

URDS = $(URDOUT)\dateadd.urd

CPPUMAKER_OPT=-C
UNOHEADER_OUT=$(UNOHEADER_COM)

# the rdb files
RDBS=$(RDBOUT)\applicat.rdb $(RDBOUT)\dateadd.rdb

UNOHEADER=\
    $(UNOHEADER_OUT)\com\sun\star\lang\XComponent.hpp \
    $(UNOHEADER_OUT)\com\sun\star\lang\XMultiServiceFactory.hpp \
    $(UNOHEADER_OUT)\com\sun\star\lang\XSingleServiceFactory.hpp \
    $(UNOHEADER_OUT)\com\sun\star\uno\TypeClass.hpp \
    $(UNOHEADER_OUT)\com\sun\star\uno\XInterface.hpp \
    $(UNOHEADER_OUT)\com\sun\star\registry\XImplementationRegistration.hpp \
    $(UNOHEADER_OUT)\com\sun\star\sheet\XAddIn.hpp \
    $(UNOHEADER_OUT)\com\sun\star\lang\XServiceName.hpp \
    $(UNOHEADER_OUT)\com\sun\star\lang\XServiceInfo.hpp \
    $(UNOHEADER_OUT)\com\sun\star\lang\XTypeProvider.hpp \
    $(UNOHEADER_OUT)\com\sun\star\uno\XWeak.hpp \
    $(UNOHEADER_OUT)\com\sun\star\uno\XAggregation.hpp \
    $(UNOHEADER_OUT)\com\sun\star\util\Date.hpp \
    $(UNOHEADER_OUT)\com\sun\star\sheet\addin\XDateFunctions.hpp

$(UNOHEADER_OUT)\com\sun\star\sheet\addin\XDateFunctions.hpp: $(RDBS) $(CPPUMAKER) 
    -$(MKDIR) $(@D)
    $(CPPUMAKER) $(CPPUMAKER_OPT) -Tcom.sun.star.sheet.addin.XDateFunctions -B/UCR -O$(UNOHEADER_OUT) $(RDBS)

$(UNOHEADER_OUT)\com\sun\star\sheet\XAddIn.hpp: $(RDBS) $(CPPUMAKER) 
    -$(MKDIR) $(@D)
    $(CPPUMAKER) $(CPPUMAKER_OPT) -Tcom.sun.star.sheet.XAddIn -B/UCR -O$(UNOHEADER_OUT) $(RDBS)

$(UNOHEADER_OUT)\com\sun\star\lang\XServiceName.hpp: $(RDBS) $(CPPUMAKER) 
    -$(MKDIR) $(@D)
    $(CPPUMAKER) $(CPPUMAKER_OPT) -Tcom.sun.star.lang.XServiceName -B/UCR -O$(UNOHEADER_OUT) $(RDBS)

$(UNOHEADER_OUT)\com\sun\star\util\Date.hpp: $(RDBS) $(CPPUMAKER) 
    -$(MKDIR) $(@D)
    $(CPPUMAKER) $(CPPUMAKER_OPT) -Tcom.sun.star.util.Date -B/UCR -O$(UNOHEADER_OUT) $(RDBS)

include i:\udk100\msdev\genheader.mk

$(URDS): $(UNOIDL)

.idl{$(URDOUT)}.urd: 
    -$(MKDIR) $(@D)
    $(UNOIDL) -I$(ROOT)\idl $< -Burd -OH$(@D) 

$(RDBOUT)\dateadd.rdb: $(URDS) $(REGMERGE)
    -$(MKDIR) $(@D)
    $(REGMERGE) $@ /UCR  $(URDS)

.cxx{$(OBJOUT)}.obj: 
    -$(MKDIR) $(@D)
    $(CC) $(CCPPFLAGS) $(CCFLAGS) $< /Fo$@

$(TARGET):  $(UNOHEADER) $(OBJS)
    -$(MKDIR) $(@D)
    $(LIB32) $(LFLAGS) /DLL $(OBJS) /out:$@ -def:datefunc.def $(LINKCPPUHELPER) $(LINKCPPU) $(LINKVOS) $(LINKSAL) msvcirtd.lib $(LIBS)



