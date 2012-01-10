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



PRJ=..$/..$/..

PRJNAME=slideshow
TARGET=transitions
ENABLE_EXCEPTIONS=TRUE
PRJINC=..$/..

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

SLOFILES = \
        $(SLO)$/barwipepolypolygon.obj \
        $(SLO)$/boxwipe.obj \
        $(SLO)$/clippingfunctor.obj \
        $(SLO)$/combtransition.obj \
        $(SLO)$/fourboxwipe.obj \
        $(SLO)$/barndoorwipe.obj \
        $(SLO)$/iriswipe.obj \
        $(SLO)$/veewipe.obj \
        $(SLO)$/ellipsewipe.obj \
        $(SLO)$/checkerboardwipe.obj \
        $(SLO)$/randomwipe.obj \
        $(SLO)$/waterfallwipe.obj \
        $(SLO)$/clockwipe.obj \
        $(SLO)$/fanwipe.obj \
        $(SLO)$/pinwheelwipe.obj \
        $(SLO)$/snakewipe.obj \
        $(SLO)$/spiralwipe.obj \
        $(SLO)$/sweepwipe.obj \
        $(SLO)$/figurewipe.obj \
        $(SLO)$/doublediamondwipe.obj \
        $(SLO)$/zigzagwipe.obj \
        $(SLO)$/parametricpolypolygonfactory.obj \
        $(SLO)$/shapetransitionfactory.obj \
        $(SLO)$/slidetransitionfactory.obj \
        $(SLO)$/transitionfactorytab.obj \
        $(SLO)$/transitiontools.obj \
        $(SLO)$/slidechangebase.obj

# ==========================================================================

.INCLUDE :      target.mk
