#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-05-03 09:04:09 $
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

PRJNAME=sal
TARGET=qa_osl_socket

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL1OBJS=  \
    $(SLO)$/osl_Socket.obj

SHL1TARGET=osl_SocketOld
SHL1STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
#SHL1STDLIBS+=	$(SOLARLIBDIR)$/cppunit.lib
SHL1STDLIBS+= ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
#SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
SHL1STDLIBS+= -ldl -lnsl
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
SHL2OBJS=  \
    $(SLO)$/osl_Socket_tests.obj
SHL2TARGET= osl_Socket_tests
SHL2STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME=    $(SHL2TARGET)
SHL2VERSIONMAP = export.map

# END --------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL3OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_StreamSocket.obj

SHL3TARGET= osl_StreamSocket
SHL3STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL3STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL3STDLIBS += -ldl -lnsl
.ENDIF

SHL3IMPLIB= i$(SHL3TARGET)
DEF3NAME=    $(SHL3TARGET)
SHL3VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL4OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_DatagramSocket.obj

SHL4TARGET= osl_DatagramSocket
SHL4STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL4STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL4STDLIBS += -ldl -lnsl
.ENDIF

SHL4IMPLIB= i$(SHL4TARGET)
DEF4NAME=    $(SHL4TARGET)
SHL4VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL5OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_SocketAddr.obj

SHL5TARGET= osl_SocketAddr
SHL5STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL5STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL5STDLIBS += -ldl -lnsl
.ENDIF

SHL5IMPLIB= i$(SHL5TARGET)
DEF5NAME=    $(SHL5TARGET)
SHL5VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL6OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_Socket2.obj

SHL6TARGET= osl_Socket2
SHL6STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL6STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL6STDLIBS += -ldl -lnsl
.ENDIF

SHL6IMPLIB= i$(SHL6TARGET)
DEF6NAME=    $(SHL6TARGET)
SHL6VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL7OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_ConnectorSocket.obj

SHL7TARGET= osl_ConnectorSocket
SHL7STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL7STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL7STDLIBS += -ldl -lnsl
.ENDIF

SHL7IMPLIB= i$(SHL7TARGET)
DEF7NAME=    $(SHL7TARGET)
SHL7VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl 
SHL8OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_AcceptorSocket.obj

SHL8TARGET= osl_AcceptorSocket
SHL8STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

.IF "$(GUI)" == "WNT"
SHL8STDLIBS += ws2_32.lib
.ENDIF

.IF "$(GUI)" == "UNX"
SHL8STDLIBS += -ldl -lnsl
.ENDIF

SHL8IMPLIB= i$(SHL8TARGET)
DEF8NAME=    $(SHL8TARGET)
SHL8VERSIONMAP = export.map
# auto generated Target:Socket
# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
