#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 14:57:05 $
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

PRJNAME=pyuno
TARGET=pythonloader.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

DLLPRE = 

#-------------------------------------------------------------------

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISS4"
# no -Bdirect for SunWS CC
DIRECT = $(LINKFLAGSDEFS)
.ENDIF

.IF "$(SYSTEM_PYTHON)" == "YES"
PYTHONLIB=$(PYTHON_LIBS)
CFLAGS+=$(PYTHON_CFLAGS)
.IF "$(EXTRA_CFLAGS)"!=""
PYTHONLIB+=-framework Python
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELSE
.INCLUDE :  pyversion.mk

CFLAGS+=-I$(SOLARINCDIR)$/python
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
    stocservices.uno	\
    invocation.uno		\
    introspection.uno	\
    invocadapt.uno		\
    proxyfac.uno 		\
    reflection.uno	\
    pythonloader.uno

# --- Targets ------------------------------------------------------

ALL : ALLTAR \
    $(DLLDEST)$/pythonloader.py	\
    $(DLLDEST)$/pyuno_services.rdb

.INCLUDE :  target.mk

$(DLLDEST)$/%.py: %.py
    cp $? $@

# For Mac OS X,
# The python loader component is linked against libpyuno.dylib,
# which hasn't been delivered yet but dyld needs to know where it is
# so regcomp can load the component.
$(DLLDEST)$/pyuno_services.rdb : makefile.mk $(DLLDEST)$/$(DLLPRE)$(TARGET)$(DLLPOST)
    -rm -f $@ $(DLLDEST)$/pyuno_services.tmp $(DLLDEST)$/pyuno_services.rdb
.IF "$(OS)"=="MACOSX"
    cd $(DLLDEST) && sh -c "DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):$(OUT)$/lib;export DYLD_LIBRARY_PATH;regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))"
.ELSE
.IF "$(GUI)$(COM)"=="WNTGCC"
    cd $(DLLDEST) && sh -c "export PATH='$(PATH):$(OUT)$/bin'; regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))"
.ELSE
    cd $(DLLDEST) && regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))
.ENDIF    # "$(GUI)$(COM)"=="WNTGCC" 
.ENDIF    # $(OS)=="MACOSX"
    cd $(DLLDEST) && mv pyuno_services.tmp pyuno_services.rdb

