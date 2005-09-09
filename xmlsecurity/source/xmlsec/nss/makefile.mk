#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 17:33:26 $
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

PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_nss

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(CRYPTO_ENGINE)" != "nss"
LIBTARGET=NO
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "nss"

.IF "$(WITH_MOZILLA)" == "NO"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity/nss"
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
.IF "$(OS)" == "LINUX"
CFLAGS +=   -fPIC -g
CFLAGSCXX += \
            -fno-rtti -Wall -Wconversion -Wpointer-arith \
            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long -pthread
CDEFS     += -DTRACING
.ELIF "$(OS)" == "NETBSD"
CFLAGS +=   -fPIC
CFLAGSCXX += \
            -fno-rtti -Wall -Wconversion -Wpointer-arith \
            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long
CDEFS     += -DTRACING
.ENDIF
.ENDIF

CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------

SOLARINC += \
 -I$(MOZ_INC) \
-I$(NSPR_INC) \
-I$(PRJ)$/source$/xmlsec

.IF "$(SYSTEM_MOZILLA)" == "YES"
SOLARINC += -DSYSTEM_MOZILLA $(NSS_INC)
.ELSE
SOLARINC += -I$(NSS_INC)
.ENDIF

SLOFILES = \
    $(SLO)$/securityenvironment_nssimpl.obj \
    $(SLO)$/xmlencryption_nssimpl.obj \
    $(SLO)$/xmlsecuritycontext_nssimpl.obj \
    $(SLO)$/xmlsignature_nssimpl.obj \
    $(SLO)$/x509certificate_nssimpl.obj \
    $(SLO)$/seinitializer_nssimpl.obj \
    $(SLO)$/xsec_nss.obj

    
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
