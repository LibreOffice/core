#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-07-23 14:40:40 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2002
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2002 by Sun Microsystems, Inc.
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
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2002 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  static.mk

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

.IF "$(depend)" == ""
ALL : 	$(RDB)	\
        $(GENJAVAFILES) \
        ALLTAR
.ELSE
ALL: 		ALLDEP
.ENDIF

.INCLUDE :	target.mk

ALLIDLFILES= \
        mozillatesttoolserver.idl \
        xmozillatesttoolserver.idl

$(RDB): $(ALLIDLFILES)
    touch $@
    rm $@
    +unoidl -I$(PRJ) -I$(SOLARIDLDIR) -Burd -OH$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(ALLIDLFILES:f:s/.idl/.urd/)}
    +regmerge $@ / $(SOLARBINDIR)$/applicat.rdb
    +regcomp -register -r $@ -c $(DLLPRE)connectr$(DLLPOST)
    +regcomp -register -r $@ -c $(DLLPRE)acceptor$(DLLPOST)
    +regcomp -register -r $@ -c $(DLLPRE)brdgfctr$(DLLPOST)
    +regcomp -register -r $@ -c $(DLLPRE)remotebridge$(DLLPOST)
    touch $@


$(GENJAVAFILES): $(RDB)
    @echo Types: $(TYPES)
    @echo Javafiles: $(GENJAVAFILES)
     javamaker -BUCR -O$(OUT) $(TYPES) $(RDB)


