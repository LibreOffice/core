#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 15:00:37 $
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
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
#
#*************************************************************************
PRJ=..$/..

PRJNAME=pyuno
TARGET=pythonloader.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

DLLPRE = 

.INCLUDE :  pyversion.mk
#-------------------------------------------------------------------

CFLAGS+=-I$(SOLARINCDIR)$/python

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISS4"
# no -Bdirect for SunWS CC
DIRECT = $(LINKFLAGSDEFS)
.ENDIF

.IF "$(GUI)" == "UNX"
PYUNOLIB=-lpyuno
PYTHONLIB=-lpython
.ELSE
PYUNOLIB=ipyuno.lib
PYTHONLIB=python$(PYMAJOR)$(PYMINOR).lib
.ENDIF

SHL1TARGET=	$(TARGET)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)		\
        $(PYUNOLIB)		\
        $(PYTHONLIB)

SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map
SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
SLOFILES=       $(SLO)$/pyuno_loader.obj

#COMPONENTS= \
#	tcv		\
#	inv		\
#	corefl		\
#	insp		\
#	invadp		\
#	proxyfac 	\
#	pythonloader.uno \

COMPONENTS= \
    typeconverter.uno	\
    invocation.uno		\
    corereflection.uno	\
    introspection.uno	\
    invocadapt.uno		\
    proxyfac.uno 		\
    pythonloader.uno

# --- Targets ------------------------------------------------------

ALL : ALLTAR \
    $(DLLDEST)$/pythonloader.py	\
    $(DLLDEST)$/pyuno_services.rdb

.INCLUDE :  target.mk

$(DLLDEST)$/%.py: %.py
    +cp $? $@

# For Mac OS X,
# The python loader component is linked against libpyuno.dylib,
# which hasn't been delivered yet but dyld needs to know where it is
# so regcomp can load the component.
$(DLLDEST)$/pyuno_services.rdb : makefile.mk
    -rm -f $@ $(DLLDEST)$/pyuno_services.tmp $(DLLDEST)$/pyuno_services.rdb
.IF "$(OS)"=="MACOSX"
    +cd $(DLLDEST) && sh -c "DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):$(OUT)$/lib;export DYLD_LIBRARY_PATH;regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))"
.ELSE
    +cd $(DLLDEST) && regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))
.ENDIF    # $(OS)=="MACOSX"
    +cd $(DLLDEST) && mv pyuno_services.tmp pyuno_services.rdb
