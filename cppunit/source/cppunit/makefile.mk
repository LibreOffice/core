#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: lla $ $Date: 2008-02-27 16:23:44 $
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
# PRJINC=..$/..$/inc$/pch

PRJNAME=cppunit
TARGET=cppunit
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
# ENABLE_RTTI=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# CXXFILES = \
# 		  querytemplate.cxx	\
# 		  stringhelper.cxx

# ENVCFLAGSCXX+=-DCPPUNIT_BUILD_DLL

CPPUNIT_TESTSHL_SLOFILES = \
    $(SLO)$/SourceLine.obj \
    $(SLO)$/Exception.obj \
    $(SLO)$/NotEqualException.obj \
    $(SLO)$/TestFailure.obj \
    $(SLO)$/joblist.obj \
    $(SLO)$/t_print.obj \
    $(SLO)$/signaltest.obj

CPPUNIT_STATIC_SOLFILES = \
    $(SLO)$/Asserter.obj \
    $(SLO)$/TestCase.obj \
    $(SLO)$/TestSuite.obj \
    $(SLO)$/TestAssert.obj \
    $(SLO)$/TestFactoryRegistry.obj \
    $(SLO)$/cmdlinebits.obj \
    $(SLO)$/tresregister.obj \
    $(SLO)$/tresstatewrapper.obj \
    $(SLO)$/registertestfunction.obj 


SLOFILES = \
    $(CPPUNIT_TESTSHL_SLOFILES) \
    $(CPPUNIT_STATIC_SOLFILES)
#-------------------------------------------------------------------------------
# This is statically cppunit library
LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1OBJFILES= \
    $(SLOFILES)

.IF "$(GUI)"=="WNT" 
.IF "$(COM)"=="GCC"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF

#-------------------------------------------------------------------------------
TARGET2=c5t_no_regallfkt
LIB2TARGET= $(LB)$/$(TARGET2).lib
LIB2OBJFILES= \
    $(CPPUNIT_TESTSHL_SLOFILES)

.IF "$(GUI)"=="WNT" 
.IF "$(COM)"=="GCC"
LIB2ARCHIV=$(LB)$/lib$(TARGET2)$(DLLPOSTFIX).a
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
LIB2ARCHIV=$(LB)$/lib$(TARGET2)$(DLLPOSTFIX).a
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

