#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2004-05-03 08:56:46 $
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
TARGET=qa_osl_file

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk


# --- BEGIN --------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/osl_File.obj
SHL1TARGET= osl_File
SHL1STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)
# .IF "$(GUI)" == "WNT"
# SHL1STDLIBS+=	$(SOLARLIBDIR)$/cppunit.lib
# .ENDIF
# .IF "$(GUI)" == "UNX"
# SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
# .ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map

#-------------------------------------------------------------------

SHL2OBJS=$(SLO)$/test_cpy_wrt_file.obj
SHL2TARGET=tcwf
SHL2STDLIBS=$(SALLIB)
.IF "$(GUI)"=="WNT"
SHL2STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF
.IF "$(GUI)" == "UNX"
SHL2STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF
SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2VERSIONMAP = export.map
DEF2NAME    =$(SHL2TARGET)


# END --------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
SHL3OBJS=  \
    $(SLO)$/osl_old_test_file.obj
SHL3TARGET= osl_old_test_file
SHL3STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

SHL3IMPLIB= i$(SHL3TARGET)

DEF3NAME    =$(SHL3TARGET)
SHL3VERSIONMAP = export.map
# END --------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
