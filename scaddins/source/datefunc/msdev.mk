#*************************************************************************
#
#   $RCSfile: msdev.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:45:45 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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



