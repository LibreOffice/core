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



PRJ = ../..
PRJNAME = jurt
TARGET = jpipe

NO_DEFAULT_STL = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT"
SHL1TARGET = jpipx
.ELSE
SHL1TARGET = jpipe
.END

SHL1CODETYPE = C
SHL1CREATEJNILIB=TRUE
SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLO)/com_sun_star_lib_connections_pipe_PipeConnection.obj
SHL1RPATH = URELIB
SHL1STDLIBS = $(SALLIB)
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.IF "$(SOLAR_JAVA)" == ""
nothing .PHONY :
.END

.INCLUDE: target.mk
