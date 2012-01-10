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

PRJNAME=UnoControls
TARGET=base
ENABLE_EXCEPTIONS=TRUE
#LIBTARGET=NO
#USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :	$(PRJ)$/util$/makefile.pmk


# --- Files --------------------------------------------------------
SLOFILES=		$(SLO)$/multiplexer.obj				\
                $(SLO)$/basecontrol.obj				\
                $(SLO)$/basecontainercontrol.obj	\
                $(SLO)$/registercontrols.obj

#LIB1TARGET= 	$(SLB)$/$(TARGET).lib
#LIB1OBJFILES=	$(SLOFILES)

#SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

#SHL1STDLIBS=\
#		$(ONELIB)	 \
#		$(USRLIB)	 \
#		$(UNOLIB)	 \
#		$(VOSLIB) 	 \
#		$(OSLLIB) 	 \
#		$(TOOLSLIB) 	 \
#		$(RTLLIB)

#SHL1DEPN=		makefile.mk
#SHL1LIBS=		$(LIB1TARGET)
#SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

#DEF1NAME=		$(SHL1TARGET)
#DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk
#.INCLUDE :	$(PRJ)$/util$/target.pmk
