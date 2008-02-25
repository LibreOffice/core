#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:44:07 $
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

PRJ=..

PRJNAME=slideshow
PRJINC=$(PRJ)$/source 
TARGET=tests
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

# BEGIN target1 -------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/views.obj	  \
    $(SLO)$/slidetest.obj \
    $(SLO)$/testshape.obj \
    $(SLO)$/testview.obj	

SHL1TARGET= tests
SHL1STDLIBS= 	$(SALLIB)		 \
                $(BASEGFXLIB)	 \
                $(CPPUHELPERLIB) \
                $(CPPULIB)		 \
                $(CPPUNITLIB)	 \
                $(UNOTOOLSLIB)	 \
                $(VCLLIB)

.IF "$(OS)"=="WNT"
    SHL1STDLIBS+=$(LIBPRE) islideshowtest.lib
.ELSE
    SHL1STDLIBS+=-lslideshowtest$(DLLPOSTFIX)
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map 
# END target1 ----------------------------------------------------------

# BEGIN target2 --------------------------------------------------------
APP2OBJS=  \
    $(SLO)$/demoshow.obj

APP2TARGET= demoshow
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPCANVASLIB)		\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(BASEGFXLIB)

.IF "$(GUI)"!="UNX"
APP2DEF=	$(MISC)$/$(TARGET).def
.ENDIF
# END target2 ----------------------------------------------------------------

#------------------------------- All object files ----------------------------

# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS) 

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk 
