#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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


