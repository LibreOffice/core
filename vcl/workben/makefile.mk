#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: cp $ $Date: 2001-02-23 18:37:48 $
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

PRJNAME=SV
TARGET=svdem
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

CXXFILES=		svdem.cxx

OBJFILES=		$(OBJ)$/svdem.obj

APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1OBJS=		$(OBJFILES)

#				$(OBJ)$/salmain.obj

APP1STDLIBS=	$(CPPULIB)			\
                $(VCLLIB)			\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(SVLIB)

APP1DEPN=		$(L)$/itools.lib	\
                $(L)$/sot.lib

.IF "$(GUI)"=="WIN" || "$(GUI)"=="OS2"
APP1DEF=		$(MISC)$/$(TARGET).def
.ENDIF

.IF "$(remote)"!=""
EXCEPTIONSFILES=$(OBJ)$/svdem.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

#svdem.cxx: $(I)tools.hxx	$(I)svgen.hxx	$(I)svwindow.hxx	\
#			$(I)sv.hxx

# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME			$(TARGET)							>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
    echo  EXETYPE		WINDOWS 						   >>$@
    echo  STUB			'winSTUB.EXE'                      >>$@
    echo  PROTMODE										   >>$@
    echo  CODE			PRELOAD MOVEABLE DISCARDABLE	   >>$@
    echo  DATA			PRELOAD MOVEABLE MULTIPLE		   >>$@
    echo  HEAPSIZE		8192							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF

# ------------------------------------------------------------------
# OS2
# ------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME			$(TARGET) WINDOWAPI 				>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
.IF "$(COM)" != "BLC"
    echo  STUB			'os2STUB.EXE'                      >>$@
.ENDIF
.IF "$(COM)"!="MTW"
    echo  EXETYPE		OS2 							   >>$@
.ENDIF
    echo  PROTMODE										   >>$@
    echo  CODE			LOADONCALL						   >>$@
    echo  DATA			PRELOAD MULTIPLE				   >>$@
    echo  HEAPSIZE		16384							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF
