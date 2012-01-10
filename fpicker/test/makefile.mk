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



PRJ=..

PRJNAME=SV
TARGET=svdem
LIBTARGET=NO
#TARGETTYPE=GUI


.IF "$(GUI)" == "OS2"
TARGETTYPE=GUI
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

OBJFILES=		$(OBJ)$/svdem.obj
APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1OBJS=		$(OBJFILES)

#				$(OBJ)$/salmain.obj

APP1STDLIBS=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(VCLLIB)			\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(SVLIB)  -l$(LB)/fps_office.uno.lib

APP1DEPN=		$(L)$/itools.lib	\
                $(L)$/sot.lib

.IF "$(GUI)"=="WIN" || "$(GUI)"=="OS200"
APP1DEF=		$(MISC)$/$(TARGET).def
.ENDIF

# --- Targets ------------------------------------------------------

ALL : \
    ALLTAR \
    $(BIN)$/applicat.rdb

.INCLUDE :	target.mk

$(BIN)$/applicat.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
     +cd $(BIN) && \
         regcomp -register -r applicat.rdb \
             -c i18nsearch.uno$(DLLPOST) \
             -c i18npool.uno$(DLLPOST)

# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME			$(TARGET)							>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
    echo  EXETYPE		WINDOWS 						   >>$@
    echo  STUB			'winSTUB.EXE'                      >>$@
    echo  PROTMODE										   >>$@
    echo  CODE			PRELOAD MOVEABLE DISCARDABLE	   >>$@
    echo  DATA			PRELOAD MOVEABLE MULTIPLE		   >>$@
    echo  HEAPSIZE		8192							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF

