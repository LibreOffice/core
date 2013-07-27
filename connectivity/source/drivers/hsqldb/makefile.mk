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
PRJINC=..$/..
PRJNAME=connectivity
TARGET=hsqldb

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE
USE_DEFFILE=TRUE

# --- Settings ----------------------------------
.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building hsqldb because Java is disabled"
.ENDIF

.IF "$(SYSTEM_HSQLDB)" == "YES"
CDEFS+=-DSYSTEM_HSQLDB -DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\"
.ENDIF
# --- Resources ---------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        hsqlui.src
        
RES1FILELIST=\
    $(SRS)$/$(TARGET).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(SOLARSRC)$/$(RSCDEFIMG)$/database
RESLIB1SRSFILES=$(RES1FILELIST)

# Note that the resource file built here is currently *not* included
# in installation sets. See hsqlui.src for an explanation

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/HStorageMap.obj					\
        $(SLO)$/HStorageAccess.obj				\
        $(SLO)$/HDriver.obj						\
        $(SLO)$/HConnection.obj					\
        $(SLO)$/HTerminateListener.obj			\
        $(SLO)$/StorageNativeOutputStream.obj	\
        $(SLO)$/StorageNativeInputStream.obj	\
        $(SLO)$/StorageFileAccess.obj			\
        $(SLO)$/HTables.obj                     \
        $(SLO)$/HTable.obj                      \
        $(SLO)$/HView.obj                       \
        $(SLO)$/HViews.obj                      \
        $(SLO)$/HCatalog.obj                    \
        $(SLO)$/HColumns.obj                    \
        $(SLO)$/HUser.obj                       \
        $(SLO)$/HUsers.obj                      \
        $(SLO)$/Hservices.obj                   \
        $(SLO)$/HTools.obj                      \
        $(SLO)$/accesslog.obj

.IF "$(GUI)" != "OS2"
SHL1VERSIONMAP=$(HSQLDB_TARGET).map
.ENDIF

# --- Library -----------------------------------
# NO $(DLLPOSTFIX) otherwise we have to find on which plattform we are for the java files
SHL1TARGET=	$(HSQLDB_TARGET)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(JVMFWKLIB)				\
    $(COMPHELPERLIB)            \
    $(TOOLSLIB)            		\
    $(UNOTOOLSLIB)


SHL1DEPN=
SHL1CREATEJNILIB=TRUE
SHL1IMPLIB=	i$(HSQLDB_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
.IF "$(GUI)" != "OS2"
DEF1EXPORTFILE=	exports.dxp
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk



ALLTAR : $(MISC)/hsqldb.component

$(MISC)/hsqldb.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        hsqldb.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt hsqldb.component
