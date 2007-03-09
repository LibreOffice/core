#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2007-03-09 08:54:10 $
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


