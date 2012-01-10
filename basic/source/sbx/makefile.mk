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

PRJNAME=basic
TARGET=sbx

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk


# --- Allgemein -----------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=	format.src

SLOFILES=	\
    $(SLO)$/sbxbase.obj	\
    $(SLO)$/sbxres.obj  	\
    $(SLO)$/sbxvalue.obj	\
    $(SLO)$/sbxvals.obj	\
    $(SLO)$/sbxvar.obj  	\
    $(SLO)$/sbxarray.obj	\
    $(SLO)$/sbxobj.obj	\
    $(SLO)$/sbxcoll.obj	\
    $(SLO)$/sbxexec.obj	\
    $(SLO)$/sbxint.obj 	\
    $(SLO)$/sbxlng.obj 	\
    $(SLO)$/sbxsng.obj 	\
    $(SLO)$/sbxmstrm.obj	\
    $(SLO)$/sbxdbl.obj 	\
    $(SLO)$/sbxcurr.obj 	\
    $(SLO)$/sbxdate.obj	\
    $(SLO)$/sbxstr.obj 	\
    $(SLO)$/sbxbool.obj	\
    $(SLO)$/sbxchar.obj	\
    $(SLO)$/sbxbyte.obj	\
    $(SLO)$/sbxuint.obj	\
    $(SLO)$/sbxulng.obj	\
    $(SLO)$/sbxform.obj	\
    $(SLO)$/sbxscan.obj 	\
    $(SLO)$/sbxdec.obj


EXCEPTIONSFILES=$(SLO)$/sbxarray.obj

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


