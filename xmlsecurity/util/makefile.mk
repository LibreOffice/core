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



PRJ=..

PRJNAME=xmlsecurity
TARGET=xmlsecurity

# Disable '-z defs' due to broken libxpcom.
#LINKFLAGSDEFS=$(0)
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(ENABLE_NSS_MODULE)" != "YES"
@all:
    @echo "No nss -> no libxmlsec -> no xmlsecurity..."
.ENDIF

# --- Files --------------------------------------------------------

BMP_IN=$(PRJ)$/res

# --- Shared-Library -----------------------------------------------

#
# The 1st shared library
#
SHL1NAME=xsec_fw
SHL1TARGET= $(SHL1NAME)
SHL1LIBS= $(SLB)$/fw.lib

SHL1STDLIBS +=		\
    $(SALLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)

SHL1IMPLIB = $(SHL1TARGET)
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)
DEF1EXPORTFILE = xsec_fw.dxp

#
# The 2nd shared library
#

SHL2NAME=xsec_xmlsec
.IF "$(GUI)"=="OS2"
SHL2NAME=xsec_xs
.ENDIF
SHL2TARGET= $(SHL2NAME)


SHL2LIBS= \
    $(SLB)$/xs_comm.lib

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL2LIBS += \
    $(SLB)$/xs_mscrypt.lib
.ENDIF

SHL2LIBS += \
    $(SLB)$/xs_nss.lib


SHL2STDLIBS +=			\
    $(SALLIB)			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(SALLIB)	\
    $(SVLLIB)			\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(XMLOFFLIB)        

.IF "$(OS)"=="SOLARIS"
SHL2STDLIBS +=-ldl
.ENDIF

.IF "$(ENABLE_NSS_MODULE)"=="YES"
.IF "$(SYSTEM_NSS)"!="YES"
.IF "$(NSPR_LIB)" != ""
SHL2STDLIBS += $(NSPR_LIB)
.ENDIF
.IF "$(NSS_LIB)" != ""
SHL2STDLIBS += $(NSS_LIB)
.ENDIF
.ELSE
SHL2STDLIBS += $(NSS_LIBS)
.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL2STDLIBS+= $(MSCRYPTOLIBS)
# SHL2STDLIBS+= $(XMLSECLIB) $(LIBXML2LIB) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
SHL2STDLIBS+= $(NSS3LIB) $(NSPR4LIB)
.ELSE
SHL2STDLIBS+= $(NSSCRYPTOLIBS)
.ENDIF


SHL2IMPLIB = $(SHL2TARGET)
SHL2DEF = $(MISC)$/$(SHL2TARGET).def
DEF2NAME = $(SHL2TARGET)
.IF "$(CRYPTO_ENGINE)" == "mscrypto"
DEF2EXPORTFILE = exports_xsmscrypt.dxp
.ENDIF

DEF2EXPORTFILE = exports_xsnss.dxp

SRSFILELIST=	\
                $(SRS)$/component.srs   \
                $(SRS)$/dialogs.srs

RESLIB1NAME=xmlsec
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRSFILELIST)

SHL4TARGET=$(TARGET)
.IF "$(GUI)"=="OS2"
SHL4TARGET=xmlsecur
.ENDIF
SHL4LIBS=\
                $(SLB)$/helper.lib      \
                $(SLB)$/dialogs.lib     \
                $(SLB)$/component.lib

SHL4STDLIBS=\
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(UCBHELPERLIB)	    \
                $(UNOTOOLSLIB)	    \
                $(VCLLIB)			\
                $(TOOLSLIB) 		\
                $(SVTOOLLIB) 		\
                $(SALLIB)			\
                $(SVLLIB)			\
                $(XMLOFFLIB)		\
                $(SVXCORELIB)

SHL4VERSIONMAP = $(SOLARENV)/src/component.map
SHL4DEPN=
SHL4IMPLIB=i$(TARGET)
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    $(TYPE) $(SHL3TARGET).flt > $@

ALLTAR : \
    $(MISC)/xmlsecurity.component \
    $(MISC)/xsec_fw.component \
    $(MISC)/xsec_xmlsec.component

.IF "$(OS)" == "WNT"
my_platform = .windows
.END

$(MISC)/xmlsecurity.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt xmlsecurity.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL4TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xmlsecurity.component

$(MISC)/xsec_fw.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xsec_fw.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xsec_fw.component

$(MISC)/xsec_xmlsec.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt xsec_xmlsec.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xsec_xmlsec$(my_platform).component
