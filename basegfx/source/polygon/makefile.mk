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
PRJNAME=basegfx
TARGET=polygon

#UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/b2dpolygon.obj						\
        $(SLO)$/b2dpolygontools.obj					\
        $(SLO)$/b2dpolypolygon.obj					\
        $(SLO)$/b2dpolypolygontools.obj				\
        $(SLO)$/b2dsvgpolypolygon.obj				\
        $(SLO)$/b2dlinegeometry.obj					\
        $(SLO)$/b2dpolypolygoncutter.obj			\
        $(SLO)$/b2dpolypolygonrasterconverter.obj	\
        $(SLO)$/b2dpolygonclipper.obj				\
        $(SLO)$/b2dpolygontriangulator.obj			\
        $(SLO)$/b2dpolygoncutandtouch.obj			\
        $(SLO)$/b2dtrapezoid.obj					\
        $(SLO)$/b3dpolygon.obj						\
        $(SLO)$/b3dpolygontools.obj					\
        $(SLO)$/b3dpolypolygon.obj					\
        $(SLO)$/b3dpolypolygontools.obj				\
        $(SLO)$/b3dpolygonclipper.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
