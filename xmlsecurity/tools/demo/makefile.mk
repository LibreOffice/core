#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: mmi $ $Date: 2004-07-14 08:12:30 $
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

PRJ=..$/..

PRJNAME=xmlsecurity
TARGET=demo

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk


CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT

ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
LIBTARGET=NO

# --- Files --------------------------------------------------------

SHARE_LIBS =	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB) \
    $(SALLIB)	\
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB)	    \
    $(TOOLSLIB) \
    $(XMLOFFLIB)

        
.IF "$(GUI)"=="WNT"
SHARE_LIBS+= "ixml2.lib" "nss3.lib" "nspr4.lib" "xmlsec.lib" "xmlsec-nss.lib" "helper.lib" "xsec_xmlsec.lib"
.ELSE
SHARE_LIBS+= "-lxml2" "-lnss3" "-lnspr4" "-lxmlsec" "-lxmlsec-nss" "-lhelper" "-lxsec_xmlsec"
.ENDIF

SHARE_OBJS =	\
    $(OBJ)$/util.obj

#
# The 1st application
#
APP1TARGET=	signdemo
APP1OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/signdemo.obj
        
.IF "$(OS)" == "LINUX"
APP1STDLIBS+= -lstdc++
.ENDIF

APP1STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 2rd application
#
APP2TARGET=	verifydemo
APP2OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/verifydemo.obj
        
.IF "$(OS)" == "LINUX"
APP2STDLIBS+= -lstdc++
.ENDIF

APP2STDLIBS+=	\
        $(SHARE_LIBS)

#
# The 3rd application
#
APP3TARGET=	multisigdemo
APP3OBJS=	\
        $(SHARE_OBJS)	\
        $(OBJ)$/multisigdemo.obj
        
.IF "$(OS)" == "LINUX"
APP3STDLIBS+= -lstdc++
.ENDIF

APP3STDLIBS+=	\
        $(SHARE_LIBS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

