#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: mmi $ $Date: 2004-07-23 03:00:41 $
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

PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_nss

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(CRYPTO_ENGINE)" != "nss"
LIBTARGET=NO
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "nss"

MOZ_INC = $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
NSS_INC = $(MOZ_INC)$/nss
NSPR_INC = $(MOZ_INC)$/nspr


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
-I$(NSS_INC) \
-I$(PRJ)$/source$/xmlsec

SLOFILES = \
    $(SLO)$/moz_profile.obj \
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
