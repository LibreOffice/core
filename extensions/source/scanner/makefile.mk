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
PRJNAME=extensions
TARGET=scn
ENABLE_EXCEPTIONS=TRUE
PACKAGE=com$/sun$/star$/scanner
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
                $(SLO)$/scnserv.obj		\
                $(SLO)$/scanner.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=\
                $(SLO)$/scanwin.obj		
.ENDIF

.IF "$(GUI)"=="UNX" || "$(GUI)" =="OS2"
SLOFILES+=\
                $(SLO)$/sane.obj		\
                $(SLO)$/sanedlg.obj		\
                $(SLO)$/scanunx.obj		\
                $(SLO)$/grid.obj

.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=\
    sanedlg.src		\
    grid.src

RESLIB1NAME=san
RESLIB1IMAGES=$(PRJ)$/source$/scanner
RESLIB1SRSFILES= $(SRS)$/scn.srs
RESLIB1DEPN= sanedlg.src sanedlg.hrc grid.src grid.hrc

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(VOSLIB)			\
    $(SALLIB)			\
    $(ONELIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(SVTOOLLIB)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="FREEBSD"
.IF "$(OS)"!="NETBSD"
SHL1STDLIBS+=$(SVTOOLLIB) -ldl
.ENDIF
.ENDIF
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/scn.component

$(MISC)/scn.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        scn.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt scn.component
