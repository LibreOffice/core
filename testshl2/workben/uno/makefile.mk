#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2007-08-03 10:17:39 $
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
PRJ=..$/..

PRJNAME=sal
TARGET=qa_uno
# this is removed at the moment because we need some enhancements
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES= \
#	  $(SLO)$/unotest.obj

#----------------------------------- ByteSequence -----------------------------------

SHL1OBJS= \
    $(SLO)$/unotest.obj 

SHL1TARGET= unotest
SHL1STDLIBS=\
       $(SALLIB) \
        $(CPPUHELPERLIB) \
        $(COMPHELPERLIB) \
        $(CPPULIB) \
        $(CPPUNITLIB)

# link $(CPPUNIT) to SHL1STDLIBS instead
# .IF "$(GUI)" == "WNT"
# SHL1STDLIBS+=	$(SOLARLIBDIR)$/cppunit.lib
# .ENDIF
# .IF "$(GUI)" == "UNX"
# SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
# .ENDIF

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = export.map



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

