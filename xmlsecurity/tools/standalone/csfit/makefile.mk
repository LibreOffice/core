#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 17:41:04 $
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

PRJNAME=xmlsecurity
TARGET=xmlsecurity-cxsfit
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
TARGETTYPE=CUI
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
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

