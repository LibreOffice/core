#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:13:40 $
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
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=configshutdown
TARGETTYPE=CUI
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=\"$(UPD)$(DLLPOSTFIX)\"

APPSTDLIBS=\
            $(SALLIB) \
            $(VOSLIB) \
            $(CPPULIB)	\
            $(CPPUHELPERLIB)


APP1STDLIBS = $(APPSTDLIBS)

.IF "$(GUI)"=="UNX"
APP1TARGET= $(TARGET).bin
.ELSE
APP1TARGET= $(TARGET)
.ENDIF

APP1OBJS=	\
    $(SLO)$/configshutdown.obj

.INCLUDE :  target.mk

#**************************************************************************
# history:
#	$Log: not supported by cvs2svn $
#	Revision 1.4  2000/09/15 09:51:48  willem.vandorp
#	OpenOffice header added
#	
#	Revision 1.3  2000/08/27 15:44:53  fs
#	#78183# replaced cout/cerr with fprintf
#	
#	Revision 1.2  2000/08/25 13:22:39  fs
#	#78116# on unx, build *.bin
#	
#	Revision 1.1  2000/08/25 13:02:07  fs
#	common reg server related command line tools
#	
#	Revision 1.1  2000/06/23 08:47:33  fs
#	socket related helpers/samples
#	
#
#	Revision 1.0 26.05.00 17:11:32  fs
#**************************************************************************

