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

.IF "$(GUI)"=="WNT"
PRJ=..$/..
PRJNAME=winaccessibility
TARGET=winaccessibility
LIBTARGET=NO
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VERSIONOBJ=
AUTOSEG=true
NO_BSYMBOLIC=	TRUE

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc
# adding to inludepath
INCPRE+=$(UNOUCROUT)
INCPRE+=$(MISC)
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
SLOFILES=   $(SLO)$/AccObject.obj		\
            $(SLO)$/ResIDGenerator.obj		\
            $(SLO)$/AccObjectWinManager.obj			\
            $(SLO)$/AccObjectManagerAgent.obj			\
            $(SLO)$/AccEventListener.obj	\
            $(SLO)$/AccComponentEventListener.obj	\
            $(SLO)$/AccContainerEventListener.obj	\
            $(SLO)$/AccDialogEventListener.obj	\
            $(SLO)$/AccFrameEventListener.obj	\
            $(SLO)$/AccWindowEventListener.obj	\
            $(SLO)$/AccMenuEventListener.obj	\
            $(SLO)$/AccTextComponentEventListener.obj	\
            $(SLO)$/AccObjectContainerEventListener.obj	\
            $(SLO)$/AccParagraphEventListener.obj	\
            $(SLO)$/AccDescendantManagerEventListener.obj	\
            $(SLO)$/AccListEventListener.obj	\
            $(SLO)$/AccTableEventListener.obj	\
            $(SLO)$/AccTreeEventListener.obj	\
            $(SLO)$/AccTopWindowListener.obj	\
            $(SLO)$/msaaservice_impl.obj 	\
            $(SLO)$/checkmt.obj	\
            $(SLO)$/AccResource.obj

#SRCFILES=winaccessibility.src
#RES1FILELIST=$(SRS)$/winaccessibility.srs
#RESLIB1NAME=$(TARGET)
#RESLIB1SRSFILES=$(RES1FILELIST)

SHL1TARGET=winaccessibility
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=\
                $(VCLLIB)		\
                $(CPPULIB)		\
                $(SALLIB)		\
                $(CPPUHELPERLIB) \
                $(TOOLSLIB) \
                $(TKLIB)		\
                $(VOSLIB)	\
                $(UWINAPILIB)      \
                $(GDI32LIB)        \
                $(WINSPOOLLIB)     \
                $(OLE32LIB)       \
                $(SHELL32LIB)      \
                $(ADVAPI32LIB)     \
                $(IMM32LIB)		\
                $(UUIDLIB)			\
                oleacc.lib				
                
SHL1OBJS=$(SLOFILES) 
SHL1VERSIONOBJ=
SHL1DESCRIPTIONOBJ=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp



# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
    $(MISC)/winaccessibility.component

.IF "$(OS)" == "WNT"
my_platform = .windows
.END

$(MISC)/winaccessibility.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt winaccessibility.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt winaccessibility.component


.ENDIF			# "$(GUI)"=="WNT"
