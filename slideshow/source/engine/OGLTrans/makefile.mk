#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2007-11-09 10:17:54 $
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

PRJ=..$/..$/..

PRJNAME=slideshow
TARGET=OGLTrans
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Nothing to do if we're compiling with --disable-opengl -----------
.IF "$(ENABLE_OPENGL)" != "TRUE"
@all:
    @echo "Building without OpenGL Transitions..."
.ENDIF

# --- Common ----------------------------------------------------------
#.IF "$(GUI)"=="UNX"

#.IF "$(SYSTEM_OPENGL)" == "YES"
CFLAGS+=$(OGLTRANS_CFLAGS)
#.ELSE
#CFLAGS+=-I$(SOLARINCDIR)/OGLTrans
#.ENDIF

SLOFILES = \
        $(SLO)$/OGLTrans_TransitionImpl.obj \
        $(SLO)$/OGLTrans_TransitionerImpl.obj
        
SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(SALLIB) $(VCLLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB) -lGL -lGLU -lX11

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

#.ENDIF

# ==========================================================================

.INCLUDE :	target.mk
