#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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

PRJ=..$/..$/..

PRJNAME=xmlsecurity
TARGET=xmlsecurity-cxsfit
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
TARGETTYPE=CUI
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk


.IF "$(CRYPTO_ENGINE)" == "nss"

MOZ_INC = $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
NSS_INC = $(MOZ_INC)$/nss
NSPR_INC = $(MOZ_INC)$/nspr

CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT
SOLARINC += \
    -I$(MOZ_INC) \
        -I$(NSPR_INC) \
        -I$(NSS_INC) \
    -I$(PRJ)$/source$/xmlsec \
    -I$(PRJ)$/source$/xmlsec$/nss

# --- Files --------------------------------------------------------

SHARE_LIBS =	\
        $(CPPULIB)	\
        $(CPPUHELPERLIB) \
        $(SALHELPERLIB)	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
SHARE_LIBS+= "ixml2.lib" "nss3.lib" "nspr4.lib" "libxmlsec.lib" "libxmlsec-nss.lib" "xsec_xmlsec.lib"
.ELSE
SHARE_LIBS+= "-lxml2" "-lnss3" "-lnspr4" "-lxmlsec1" "-lxmlsec1-nss" "-lxsec_xmlsec"
.ENDIF
        
SHARE_OBJS =	\
        $(OBJ)$/helper.obj

#
# The 1st application
#

APP2TARGET=	signer
APP2OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/signer.obj
        
.IF "$(OS)" == "LINUX"
APP2STDLIBS+= -lstdc++
.ENDIF

APP2STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 2nd application
#
APP3TARGET=	encrypter
APP3OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/encrypter.obj
        
.IF "$(OS)" == "LINUX"
APP3STDLIBS+= -lstdc++
.ENDIF

APP3STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 3rd application
#
APP4TARGET=	verifier
APP4OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/verifier.obj
        
.IF "$(OS)" == "LINUX"
APP4STDLIBS+= -lstdc++
.ENDIF

APP4STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 4th application
#
APP5TARGET=	decrypter
APP5OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/decrypter.obj
        
.IF "$(OS)" == "LINUX"
APP5STDLIBS+= -lstdc++
.ENDIF

APP5STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 5th application
#
APP6TARGET=	certmngr
APP6OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/certmngr.obj
        
.IF "$(OS)" == "LINUX"
APP6STDLIBS+= -lstdc++
.ENDIF

APP6STDLIBS+=	\
        $(SHARE_LIBS)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

