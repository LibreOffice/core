#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: bm $ $Date: 2003-10-06 09:58:34 $
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************
PRJ=..

PRJNAME=anyperformance
TARGET=	anyperformance
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=$(OUT)$/inc$/test
INCPRE+=$(OUT)$/inc$/test

OBJFILES= \
        $(OBJ)$/anyperformance.obj

APP1TARGET=	anyperformance
APP1OBJS=	\
        $(OBJ)$/anyperformance.obj

APP1STDLIBS+=	\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)

APP1DEF=$(MISC)$/$(APP1TARGET).def

#ALLIDLFILES:= \
#		cpputest.idl 		\
#		language_binding.idl	\
#		alignment.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
#ALL : $(BIN)$/anyperformance.rdb unoheader ALLTAR
ALL : ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.IF "$(COM)" == "MSC"
.IF "$(debug)" != ""
CFLAGS += /Ob0
.ENDIF
.ENDIF

.IF "$(extra_mapping)" != ""
CFLAGS += -DEXTRA_MAPPING
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS = -C
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

#TYPES:=		-Ttest.XLanguageBindingTest \
#		-Ttest.XSimpleInterface \
#		-Ttest.Test1 \
#		-Ttest.Test2 \
#		-Ttest.TdTest1 \
#		-Ttest.Test3 \
#		-Ttest.Base \
#		-Ttest.Base1 \
#		-Ttest.Base2 \
# 		-Tcom.sun.star.lang.XMultiServiceFactory \
# 		-Tcom.sun.star.lang.XSingleServiceFactory \
# 		-Tcom.sun.star.lang.XInitialization \
# 		-Tcom.sun.star.lang.XServiceInfo \
# 		-Tcom.sun.star.lang.XEventListener \
# 		-Tcom.sun.star.lang.XTypeProvider \
# 		-Tcom.sun.star.registry.XSimpleRegistry \
# 		-Tcom.sun.star.registry.XRegistryKey \
# 		-Tcom.sun.star.loader.XImplementationLoader \
# 		-Tcom.sun.star.registry.XImplementationRegistration \
# 		-Tcom.sun.star.lang.XComponent \
# 		-Tcom.sun.star.uno.XComponentContext \
# 		-Tcom.sun.star.container.XSet \
# 		-Tcom.sun.star.container.XNameContainer \
# 		-Tcom.sun.star.uno.TypeClass \
# 		-Tcom.sun.star.uno.XReference \
# 		-Tcom.sun.star.uno.XAdapter \
# 		-Tcom.sun.star.uno.XAggregation \
# 		-Tcom.sun.star.uno.XWeak \
# 		-Tcom.sun.star.beans.XPropertySet \
# 		-Tcom.sun.star.reflection.XIdlClassProvider \
# 		-Tcom.sun.star.container.XHierarchicalNameAccess \
# 		-Tcom.sun.star.uno.XCurrentContext

$(BIN)$/anyperformance.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(UNOUCRRDB)
    touch $@

# 	+regcomp -register -r $@ -c javaloader.dll
# 	+regcomp -register -r $@ -c jen.dll

unoheader: $(BIN)$/anyperformance.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/anyperformance.rdb
#	+cunomaker -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/anyperformance.rdb

