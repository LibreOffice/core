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



PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_nss

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(ENABLE_NSS_MODULE)"!="YES"
@all:
    @echo "No nss -> no libxmlsec -> no xmlsecurity/nss"
.ENDIF

.IF "$(SYSTEM_NSS)" != "YES"
MOZ_INC = $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
NSS_INC = $(MOZ_INC)$/nss
NSPR_INC = $(MOZ_INC)$/nspr
.ELSE
# TODO: better use pkgconfig to find the proper system include path
NSS_INC = /usr/include/nss3
NSPR_INC = /usr/include/nspr4
.ENDIF

.IF "$(GUI)" == "WNT"
.IF "$(DBG_LEVEL)" == "0"
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
.IF "$(COM)"=="GCC"
CFLAGS += $(CFLAGSDEBUG)
.ELSE
CFLAGS +=   -GR- -W3 -Gy -MD -UDEBUG
.ENDIF
.ELSE
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
.IF "$(COM)"=="GCC"
.ELSE
CFLAGS += -Zi -GR- -W3 -Gy -MDd -UNDEBUG
.ENDIF
.ENDIF
.ENDIF
.IF "$(GUI)" == "UNX"
INCPOST += \
$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
#.IF "$(OS)" == "LINUX"
#CFLAGS +=   -fPIC -g
#CFLAGSCXX += \
#            -fno-rtti -Wall -Wconversion -Wpointer-arith \
#            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
#            -Wno-long-long -pthread
#CDEFS     += -DTRACING
#.ELIF "$(OS)" == "NETBSD"
#CFLAGS +=   -fPIC
#CFLAGSCXX += \
#            -fno-rtti -Wall -Wconversion -Wpointer-arith \
#            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
#            -Wno-long-long
#CDEFS     += -DTRACING
#.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "nss"
CDEFS += -DXMLSEC_CRYPTO_NSS
.ENDIF

CDEFS += -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------

SOLARINC += \
 -I$(NSPR_INC) \
 -I$(PRJ)$/source$/xmlsec

SOLARINC += -I$(NSS_INC)

SLOFILES = \
    $(SLO)$/nssinitializer.obj \
    $(SLO)$/digestcontext.obj \
    $(SLO)$/ciphercontext.obj \
    $(SLO)$/xsec_nss.obj

.IF "$(CRYPTO_ENGINE)" == "nss"
SLOFILES += \
    $(SLO)$/securityenvironment_nssimpl.obj \
    $(SLO)$/seinitializer_nssimpl.obj \
    $(SLO)$/xmlencryption_nssimpl.obj \
    $(SLO)$/xmlsecuritycontext_nssimpl.obj \
    $(SLO)$/xmlsignature_nssimpl.obj \
    $(SLO)$/x509certificate_nssimpl.obj \
    $(SLO)$/sanextension_nssimpl.obj \
    $(SLO)$/secerror.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
