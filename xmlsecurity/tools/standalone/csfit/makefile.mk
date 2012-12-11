#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..$/..

PRJNAME=xmlsecurity
TARGET=xmlsecurity-cxsfit
ENABLE_EXCEPTIONS=TRUE
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

.IF "$(OS)"=="WNT"
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

