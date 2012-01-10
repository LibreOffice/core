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



