#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 15:09:28 $
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

PRJNAME=extensions
TARGET=scn
ENABLE_EXCEPTIONS=TRUE
PACKAGE=com$/sun$/star$/scanner
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- UnoTypes -----------------------------------------------------

UNOTYPES=\
    com.sun.star.uno.TypeClass									\
    com.sun.star.uno.XInterface									\
    com.sun.star.uno.Exception									\
    com.sun.star.uno.RuntimeException							\
    com.sun.star.uno.XWeak										\
    com.sun.star.awt.XBitmap									\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.lang.XMultiServiceFactory						\
    com.sun.star.lang.XSingleServiceFactory						\
    com.sun.star.lang.EventObject								\
    com.sun.star.registry.XImplementationRegistration			\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.registry.XSimpleRegistry						\
    com.sun.star.scanner.XScannerManager						\
    com.sun.star.scanner.ScannerContext							\
    com.sun.star.scanner.ScannerException						

# --- Files --------------------------------------------------------
.IF "$(OS)"=="MACOSX"
dummy:
    @echo "Nothing to build for Mac OS X"
.ELSE
SLOFILES=\
                $(SLO)$/scnserv.obj		\
                $(SLO)$/scanner.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=\
                $(SLO)$/scanwin.obj		
.ENDIF

.IF "$(GUI)"=="UNX"
SLOFILES+=\
                $(SLO)$/sane.obj		\
                $(SLO)$/sanedlg.obj		\
                $(SLO)$/scanunx.obj		\
                $(SLO)$/grid.obj

SRS1NAME=$(TARGET)
SRC1FILES=\
    sanedlg.src		\
    grid.src

RESLIB1NAME=san
RESLIB1SRSFILES= $(SRS)$/scn.srs
RESLIB1DEPN= sanedlg.src sanedlg.hrc grid.src grid.hrc
.ENDIF

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(VOSLIB)			\
    $(SALLIB)			\
    $(ONELIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(SVTOOLLIB)		\
    $(GOODIESLIB)		

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="FREEBSD"
SHL1STDLIBS+=$(SVTOOLLIB) -ldl
.ENDIF
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF		# "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
