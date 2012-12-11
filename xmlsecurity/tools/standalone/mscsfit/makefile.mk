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
TARGET=xmlsecurity-mscxsfit
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(CRYPTO_ENGINE)" == "mscrypto"

CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO -DXMLSEC_NO_XSLT
SOLARINC += \
    -I$(PRJ)$/source$/xmlsec \
    -I$(PRJ)$/source$/xmlsec$/mscrypt

# --- Files --------------------------------------------------------

SHARE_LIBS =	\
        $(CPPULIB)	\
        $(CPPUHELPERLIB) \
        $(SALLIB)

.IF "$(OS)"=="WNT"
SHARE_LIBS+= "libxml2.lib" "crypt32.lib" "advapi32.lib" "libxmlsec.lib" "libxmlsec-mscrypto.lib" "xsec_xmlsec.lib" "xs_comm.lib" "xs_mscrypt.lib"
.ELSE
SHARE_LIBS+= "-lxml2" "-lnss3" "-lnspr4" "-lxmlsec1" "-lxmlsec1-nss" "-lxsec_xmlsec" "-lxs_comm" "-lxs_nss"
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

