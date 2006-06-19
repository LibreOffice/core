#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2006-06-19 23:54:07 $
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

PRJNAME=bridges
TARGET=test
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------
ALLIDLFILES = test_bridge.idl
CPPUMAKERFLAGS += -C


UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb  $(BIN)$/test.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

# adding to inludeoath
INCPRE+=$(UNOUCROUT)
CFLAGS += -I..$/source$/remote$/urp

UNOTYPES = \
        com.sun.star.uno.XWeak\
        com.sun.star.uno.XNamingService\
        com.sun.star.uno.XAggregation \
        com.sun.star.uno.TypeClass\
        com.sun.star.io.XInputStream\
        com.sun.star.io.XOutputStream\
        com.sun.star.lang.XInitialization \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.registry.XSimpleRegistry \
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.corba.giop.TargetAddress \
        com.sun.star.corba.giop.TargetAddressGroup \
        com.sun.star.lang.XComponent \
        com.sun.star.bridge.XBridgeFactory\
        com.sun.star.connection.XAcceptor\
        com.sun.star.connection.XConnector\
        com.sun.star.beans.Property\
        com.sun.star.corba.giop.RequestHeader_1_2\
             com.sun.star.container.XSet\
        com.sun.star.lang.XServiceInfo\
        test.XTestFactory \
        com.sun.star.test.performance.XPerformanceTest \
        com.sun.star.lang.XMain \
        com.sun.star.lang.XMultiComponentFactory \
        com.sun.star.lang.XSingleComponentFactory

#UNOTYPES= com.sun.star.corba.giop.MsgType_1_1
#UNOTYPES=	com.sun.star.uno.XInterface \
#	  	com.sun.star.uno.TypeClass \
#		com.sun.star.corba.CorbaString8 
#com.sun.star.corba.giop.RequestHeader_1_1

JARFILES = jurt.jar unoil.jar



# GENJAVACLASSFILES = \
# 	$(CLASSDIR)$/test$/TestTypes.class \
# 	$(CLASSDIR)$/test$/TestBridgeException.class \
# 	$(CLASSDIR)$/test$/XCallMe.class \
# 	$(CLASSDIR)$/test$/XInterfaceTest.class \
# 	$(CLASSDIR)$/test$/XTestFactory.class \
    

# JAVACLASSFILES= \
# 	$(CLASSDIR)$/testclient.class



# TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
# GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
# JAVAFILES= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) $(GENJAVAFILES) 


OBJFILES=	\
            $(OBJ)$/testserver.obj \
            $(OBJ)$/testclient.obj \
            $(OBJ)$/testcomp.obj  \
            $(OBJ)$/testsameprocess.obj 


APP2TARGET=	testserver
APP2OBJS=	$(OBJ)$/testserver.obj \
        $(OBJ)$/testcomp.obj 

.IF "$(OS)" == "LINUX"
APP2STDLIBS+= -lstdc++
.ENDIF

APP2STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)	\
            $(LIBCIMT) 

#APP2DEF=	$(MISC)$/$(APP2TARGET).def

APP3TARGET=	testclient
APP3OBJS=	$(OBJ)$/testclient.obj \
        $(OBJ)$/testcomp.obj

.IF "$(OS)" == "LINUX"
APP3STDLIBS+= -lstdc++
.ENDIF

APP3STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)	\
            $(LIBCIMT) 
#			imsci_uno.lib

#APP3DEF=	$(MISC)$/$(APP3TARGET).def

#----------------------------------

APP4TARGET=	testsameprocess
APP4OBJS=	$(OBJ)$/testsameprocess.obj \
        $(OBJ)$/testcomp.obj

.IF "$(OS)" == "LINUX"
APP4STDLIBS+= -lstdc++
.ENDIF

APP4STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)	\
            $(LIBCIMT) 

APP4DEF=	$(MISC)$/$(APP4TARGET).def

#----------------------------------

# APP5TARGET=	testoffice
# APP5OBJS=	$(OBJ)$/testoffice.obj \
# 		$(OBJ)$/testcomp.obj

# .IF "$(OS)" == "LINUX"
# APP5STDLIBS+= -lstdc++
# .ENDIF

# APP5STDLIBS+=	\
# 			$(CPPULIB)	\
# 			$(CPPUHELPERLIB)\
# 			$(SALLIB)	\
# 			$(LIBCIMT) 

# APP5DEF=	$(MISC)$/$(APP5TARGET).def


# --- Targets ------------------------------------------------------

#.IF "$(depend)" == ""
ALL : 	$(BIN)$/test.rdb	\
    $(BIN)$/server.rdb	\
    $(BIN)$/client.rdb	\
    $(GENJAVAFILES)		\
        ALLTAR 

#.ELSE
#ALL: 	ALLDEP
#.ENDIF

.INCLUDE :  target.mk

$(BIN)$/test.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@

$(BIN)$/client.rdb: $(BIN)$/test.rdb
    +rm -f $(BIN)$/client.rdb
    +regmerge $@ / $(BIN)$/test.rdb $(SOLARBINDIR)$/udkapi.rdb

$(BIN)$/server.rdb: $(BIN)$/test.rdb
    +rm -f $(BIN)$/client.rdb
    +regmerge $@ / $(BIN)$/test.rdb $(SOLARBINDIR)$/udkapi.rdb

# $(GENJAVAFILES) : $(RDB)
# 	+javamaker -BUCR -O$(OUT) $(TYPES) $(UNOUCRRDB)

