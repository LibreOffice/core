#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: mt $ $Date: 2004-07-21 14:34:06 $
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

PRJ=..

PRJNAME=xmlsecurity
TARGET=xmlsecurity

# Disable '-z defs' due to broken libxpcom.
#LINKFLAGSDEFS=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk


# --- Allgemein ----------------------------------------------------
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE

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
SHL2TARGET= $(SHL2NAME)
SHL2LIBS= \
    $(SLB)$/xs_comm.lib

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL2LIBS += \
    $(SLB)$/xs_mscrypt.lib
.ELSE
SHL2LIBS += \
    $(SLB)$/xs_nss.lib
.ENDIF

SHL2STDLIBS +=			\
    $(SALLIB)			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(SVLLIB)			\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(XMLOFFLIB)
    
.IF "$(GUI)"=="WNT"
.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHL2STDLIBS+= "ixml2.lib" "xmlsec.lib" "xmlsec-mscrypto.lib" "crypt32.lib" "advapi32.lib"
.ELSE
SHL2STDLIBS+= "ixml2.lib" "nss3.lib" "nspr4.lib" "plc4.lib" "xmlsec.lib" "xmlsec-nss.lib"
.ENDIF
.ELSE
SHL2STDLIBS+= "-lxml2" "-lnss3" "-lnspr4" "-lplc4" "-lxmlsec" "-lxmlsec-nss"  
.ENDIF
    
SHL2IMPLIB = $(SHL2TARGET)
SHL2DEF = $(MISC)$/$(SHL2TARGET).def
DEF2NAME = $(SHL2TARGET)
.IF "$(CRYPTO_ENGINE)" == "mscrypto"
DEF2EXPORTFILE = exports_xsmscrypt.dxp
.ELSE
DEF2EXPORTFILE = exports_xsnss.dxp
.ENDIF

SRSFILELIST=	\
                $(SRS)$/dialogs.srs

RESLIB1NAME=xmlsec
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRSFILELIST)

SHL4TARGET=$(TARGET)
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
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(SVLLIB)			\
                $(SFXLIB)			\
                $(XMLOFFLIB)		\
                $(SVXLIB)
                
.IF "$(GUI)"=="WNT"
SHL4STDLIBS+= "xmlsec.lib" "xsec_xmlsec.lib"
.ELSE
SHL4STDLIBS+= "-lxmlsec" "-lxsec_xmlsec"
.ENDIF
                
        
SHL4VERSIONMAP = xmlsecurity.map
SHL4DEPN=
SHL4IMPLIB=i$(TARGET)
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    +$(TYPE) $(SHL3TARGET).flt > $@
