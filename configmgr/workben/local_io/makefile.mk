#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:43:06 $
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
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=cfgfile
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# ... common for all test executables ..............................
APPSTDLIBS=\
            $(SALLIB) \
         $(VOSLIB) \
            $(CPPULIB)	\
        $(UNOTOOLSLIB) \
            $(CPPUHELPERLIB) \


# ... FileTests .....................................................
APP1STDLIBS = $(APPSTDLIBS) \
    $(STDLIBCPP) \

APP1STDLIBS+=$(STDLIBCPP)

APP1TARGET= $(TARGET)

APP1OBJS=	\
    $(SLO)$/cfgfile.obj \
    $(SLO)$/oslstream.obj \
    $(SLO)$/filehelper.obj \
    $(SLO)$/simpletest.obj \
    $(SLO)$/strimpl.obj \
    $(SLO)$/confname.obj \

#	$(SLO)$/xmlexport.obj \
#	$(SLO)$/xmlimport.obj \

# 	  $(SLO)$/xmltreebuilder.obj \
# 	  $(SLO)$/cmtree.obj \
# 	  $(SLO)$/cmtreemodel.obj \
# 	  $(SLO)$/typeconverter.obj \
# 	  $(SLO)$/changes.obj \
# 	  $(SLO)$/xmlformater.obj \
# 	  $(SLO)$/attributes.obj \
# 	  $(SLO)$/tracer.obj \


# ... common for all test executables ..............................
# APP2STDLIBS = $(APPSTDLIBS)
# 
# APP2STDLIBS+=$(STDLIBCPP)
# 
# APP2TARGET= cfglocal
# APP2OBJS=	\
# 	  $(SLO)$/cfglocal.obj	\
# 	  $(SLO)$/receivethread.obj	\
# 	  $(SLO)$/redirector.obj	\
# 	  $(SLO)$/socketstream.obj	\
# 	  $(SLO)$/attributes.obj	\
# 	  $(SLO)$/localsession.obj \
# 	  $(SLO)$/saxtools.obj \
# 	  $(SLO)$/mergeupdates.obj \
# 	  $(SLO)$/oslstream.obj \
# 	  $(SLO)$/configsession.obj \
# 	  $(SLO)$/confname.obj \
# 	  $(SLO)$/sessionstream.obj \
# 	  $(SLO)$/filehelper.obj \
# 	  $(SLO)$/tracer.obj \
# 	  $(SLO)$/updatedom.obj \
# 	  $(SLO)$/strconverter.obj \
# 	  $(SLO)$/strimpl.obj \
# 

.INCLUDE :  target.mk


