#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:32:29 $
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

PRJNAME=extensions
TARGET=workben
LIBTARGET=NO

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
# --- Files --------------------------------------------------------

OBJFILES=	$(OBJ)$/testcomponent.obj \
        $(OBJ)$/testconnection.obj

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

UNOTYPES =	com.sun.star.connection.XConnector \
        com.sun.star.connection.XAcceptor  \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.lang.XComponent \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.test.XSimpleTest            \
        com.sun.star.lang.XSingleComponentFactory \
        com.sun.star.lang.XMultiComponentFactory


#
# std testcomponent
#

APP1TARGET = testcomponent
APP1OBJS   = $(OBJ)$/testcomponent.obj
APP1STDLIBS = 	$(SALLIB) \
        $(CPPULIB)\
        $(CPPUHELPERLIB)

APP2TARGET = testconnection
APP2OBJS   = $(OBJ)$/testconnection.obj
APP2STDLIBS = 	$(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB)



# --- Targets ------------------------------------------------------

ALL : 	$(BIN)$/applicat.rdb	\
    ALLTAR

$(BIN)$/applicat.rdb: $(SOLARBINDIR)$/udkapi.rdb
    +rm -f $@
    +regmerge $@ / $?


.INCLUDE :  target.mk
