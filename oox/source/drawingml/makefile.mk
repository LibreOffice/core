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

PRJNAME=oox
TARGET=drawingml
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/clrscheme.obj\
        $(SLO)$/clrschemecontext.obj\
        $(SLO)$/color.obj\
        $(SLO)$/colorchoicecontext.obj\
        $(SLO)$/connectorshapecontext.obj\
        $(SLO)$/customshapegeometry.obj\
        $(SLO)$/customshapeproperties.obj\
        $(SLO)$/drawingmltypes.obj\
        $(SLO)$/embeddedwavaudiofile.obj\
        $(SLO)$/fillproperties.obj\
        $(SLO)$/fillpropertiesgroupcontext.obj\
        $(SLO)$/graphicshapecontext.obj\
        $(SLO)$/guidcontext.obj\
        $(SLO)$/hyperlinkcontext.obj\
        $(SLO)$/lineproperties.obj\
        $(SLO)$/linepropertiescontext.obj\
        $(SLO)$/objectdefaultcontext.obj\
        $(SLO)$/shape.obj\
        $(SLO)$/shapecontext.obj\
        $(SLO)$/shapegroupcontext.obj\
        $(SLO)$/shapepropertiescontext.obj\
        $(SLO)$/shapepropertymap.obj\
        $(SLO)$/shapestylecontext.obj\
        $(SLO)$/spdefcontext.obj\
        $(SLO)$/textbody.obj\
        $(SLO)$/textbodycontext.obj\
        $(SLO)$/textbodyproperties.obj\
        $(SLO)$/textbodypropertiescontext.obj\
        $(SLO)$/textcharacterproperties.obj\
        $(SLO)$/textcharacterpropertiescontext.obj\
        $(SLO)$/textfield.obj\
        $(SLO)$/textfieldcontext.obj\
        $(SLO)$/textfont.obj\
        $(SLO)$/textliststyle.obj \
        $(SLO)$/textliststylecontext.obj\
        $(SLO)$/textparagraph.obj\
        $(SLO)$/textparagraphproperties.obj\
        $(SLO)$/textparagraphpropertiescontext.obj\
        $(SLO)$/textrun.obj\
        $(SLO)$/textspacingcontext.obj\
        $(SLO)$/texttabstoplistcontext.obj\
        $(SLO)$/theme.obj\
        $(SLO)$/themeelementscontext.obj\
        $(SLO)$/themefragmenthandler.obj\
        $(SLO)$/transform2dcontext.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
