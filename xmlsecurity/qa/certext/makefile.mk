#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
#***********************************************************************/

PRJ = ../..
PRJNAME = xmlsecurity
TARGET = qa_certext

ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END


.INCLUDE: settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "nss"

.IF "$(WITH_MOZILLA)" == "NO" || "$(ENABLE_NSS_MODULE)"!="YES"
.IF "$(SYSTEM_MOZILLA)" != "YES"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity/nss"
.ENDIF
.ENDIF

.IF "$(SYSTEM_MOZILLA)" != "YES"
MOZ_INC = $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
NSS_INC = $(MOZ_INC)$/nss
NSPR_INC = $(MOZ_INC)$/nspr
.ELSE
# MOZ_INC already defined from environment
NSS_INC = $(MOZ_NSS_CFLAGS)
NSPR_INC = $(MOZ_INC)$/nspr
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
CFLAGS += -features=tmplife
#This flag is needed to build mozilla 1.7 code
.ENDIF		# "$(COMNAME)"=="sunpro5"
.ENDIF

.IF "$(GUI)" == "WNT"
.IF "$(DBG_LEVEL)" == "0"
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
CFLAGS +=   -GR- -W3 -Gy -MD -UDEBUG
.ELSE
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
CFLAGS += -Zi -GR- -W3 -Gy -MDd -UNDEBUG
.ENDIF
.ENDIF
.IF "$(GUI)" == "UNX"
INCPOST += \
$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
.ENDIF

CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT

SOLARINC += \
 -I$(MOZ_INC) \
-I$(NSPR_INC) \
-I$(PRJ)$/source$/xmlsec

.IF "$(SYSTEM_MOZILLA)" == "YES"
SOLARINC += -DSYSTEM_MOZILLA $(NSS_INC)
.ELSE
SOLARINC += -I$(NSS_INC)
.ENDIF
.ENDIF




CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUNITLIB)     \
              $(SALLIB)         \
              $(NEON3RDLIB)     \
              $(CPPULIB)        \
              $(XMLOFFLIB)      \
              $(CPPUHELPERLIB)	\
              $(SVLLIB)			\
              $(TOOLSLIB)	    \
              $(COMPHELPERLIB)
              
    
    
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS +=-ldl
.ENDIF

.IF "$(SYSTEM_MOZILLA)" == "YES"
.IF "$(NSPR_LIB)" != ""
SHL1STDLIBS += $(NSPR_LIB)
.ENDIF
.IF "$(NSS_LIB)" != ""
SHL1STDLIBS += $(NSS_LIB)
.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL1STDLIBS+= $(MSCRYPTOLIBS)
.ELSE
CDEFS += -DNSS_ENGINE
SHL1STDLIBS+= $(NSSCRYPTOLIBS)
.ENDIF	

.IF "$(ENABLE_NSS_MODULE)"=="YES" || "$(SYSTEM_MOZILLA)" == "YES"

SHL1LIBS= \
    $(SLB)$/xs_comm.lib

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL1LIBS += \
    $(SLB)$/xs_mscrypt.lib
.ELSE
SHL1LIBS += \
    $(SLB)$/xs_nss.lib
.ENDIF

.ENDIF	

SHL1TARGET = qa_CertExt
SHL1VERSIONMAP = $(PRJ)/qa/certext/export.map
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SLO)/SanCertExt.obj

.INCLUDE: target.mk

ALLTAR : test

test .PHONY : $(SHL1TARGETN)
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/types.rdb
