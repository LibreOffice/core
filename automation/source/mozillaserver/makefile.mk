#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=automation
TARGET=xxx
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

PACKAGE := com$/sun$/star$/testtool
TARGET  := com_sun_star_testtool


#RDB=$(SOLARBINDIR)$/applicat.rdb
RDB=$(BIN)$/mozillatesttool.rdb


GENJAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/XAction.class \
    $(CLASSDIR)$/$(PACKAGE)$/XActionControl.class \
    $(CLASSDIR)$/$(PACKAGE)$/XActionCommand.class \
    $(CLASSDIR)$/$(PACKAGE)$/XActionControl.class \
    $(CLASSDIR)$/$(PACKAGE)$/XActionFlow.class \
    $(CLASSDIR)$/$(PACKAGE)$/XResult.class \
    $(CLASSDIR)$/$(PACKAGE)$/ResultType.class \
    $(CLASSDIR)$/$(PACKAGE)$/XMozillaTesttoolServer.class


TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
#JAVAFILES= $(GENJAVAFILES)



UNOUCRDEP=$(RDB)
UNOUCRRDB=$(RDB)

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

UNOTYPES= \
        com.sun.star.uno.TypeClass		\
        com.sun.star.uno.XAggregation		\
        com.sun.star.uno.XWeak			\
        com.sun.star.lang.XTypeProvider		\
        com.sun.star.lang.XServiceInfo		\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XComponent		\
        com.sun.star.lang.XMain			\
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.bridge.XUnoUrlResolver	\
        com.sun.star.container.XSet \
        com.sun.star.testtool.XAction \
        com.sun.star.testtool.XActionControl \
        com.sun.star.testtool.XActionCommand \
        com.sun.star.testtool.XActionControl \
        com.sun.star.testtool.XActionFlow \
        com.sun.star.testtool.XResult \
        com.sun.star.testtool.ResultType \
        com.sun.star.testtool.XMozillaTesttoolServer



#SLOFILES= \
#		$(SLO)$/iserverproxy.obj \
#		$(SLO)$/broadcastclient.obj \
#		$(SLO)$/genericinformation.obj \
#		$(SLO)$/informationclient.obj

# ---- test ----

#LIB1TARGET=$(SLB)$/iserverproxy.lib
#LIB1OBJFILES=$(SLOFILES)

#SHL1TARGET=iserverproxy
#SHL1STDLIBS= \
#		$(VOSLIB) $(OSLLIB) $(RTLLIB) $(TOOLSLIB) \
#		$(CPPULIB)		\
#		$(CPPUHELPERLIB)	\
#		$(UNOLIB)	\
#		$(SVTOOLLIB)	 \
#		$(SALLIB)		 \
#		$(BOOTSTRP)

#SHL1LIBS=	$(LIB1TARGET) $(LB)$/ico.lib
#SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
#DEF1NAME=	$(SHL1TARGET)
#DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(GENJAVAFILES)

ALLIDLFILES= \
        mozillatesttoolserver.idl \
        xmozillatesttoolserver.idl

$(RDB): $(ALLIDLFILES)
    $(TOUCH) $@
    $(RM) $@
    unoidl -I$(PRJ) -I$(SOLARIDLDIR) -Burd -OH$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(ALLIDLFILES:f:s/.idl/.urd/)}
    $(REGMERGE) $@ / $(SOLARBINDIR)$/applicat.rdb
    $(REGCOMP) -register -r $@ -c $(DLLPRE)connectr$(DLLPOST)
    $(REGCOMP) -register -r $@ -c $(DLLPRE)acceptor$(DLLPOST)
    $(REGCOMP) -register -r $@ -c $(DLLPRE)brdgfctr$(DLLPOST)
    $(REGCOMP) -register -r $@ -c $(DLLPRE)remotebridge$(DLLPOST)
    $(TOUCH) $@


$(GENJAVAFILES): $(RDB)
    @echo Types: $(TYPES)
    @echo Javafiles: $(GENJAVAFILES)
     $(JAVAMAKER) -BUCR -O$(OUT) $(TYPES) $(RDB)


