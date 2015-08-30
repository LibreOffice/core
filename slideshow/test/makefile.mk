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

PRJNAME=slideshow
PRJINC=$(PRJ)$/source
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

# BEGIN target1 -------------------------------------------------------

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.
 
.ELSE


APP1OBJS=  \
    $(SLO)$/main.obj \
    $(SLO)$/views.obj	  \
    $(SLO)$/slidetest.obj \
    $(SLO)$/testshape.obj \
    $(SLO)$/testview.obj

APP1TARGET= tests
APP1STDLIBS= 	$(SALLIB)		 \
                $(BASEGFXLIB)	 \
                $(CPPUHELPERLIB) \
                $(CPPULIB)		 \
                                $(TESTSHL2LIB)    \
                $(GTESTLIB)	 \
                $(UNOTOOLSLIB)	 \
                $(VCLLIB)

.IF "$(OS)"=="WNT"
    APP1STDLIBS+=$(LIBPRE) islideshowtest.lib
.ELSE
    APP1STDLIBS+=-lslideshowtest$(DLLPOSTFIX)
.ENDIF

APP1RPATH = NONE
APP1TEST = enabled

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
# END target1 ----------------------------------------------------------

# BEGIN target2 --------------------------------------------------------
APP2OBJS=  \
    $(SLO)$/demoshow.obj \
    $(SLO)$/main.obj

APP2TARGET= demoshow
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPCANVASLIB)		\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(BASEGFXLIB)

.IF "$(GUI)"!="UNX"
APP2DEF=	$(MISC)$/$(TARGET).def
.ENDIF
# END target2 ----------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
