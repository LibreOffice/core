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

PRJNAME = xmlsecurity
TARGET = xs_comm

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(WITH_MOZILLA)" == "NO" || "$(ENABLE_NSS_MODULE)"!="YES"
.IF "$(SYSTEM_MOZILLA)" != "YES"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity.."
.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO
.ENDIF

CDEFS += -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------
SLOFILES = \
    $(SLO)$/biginteger.obj \
    $(SLO)$/certvalidity.obj \
    $(SLO)$/saxhelper.obj \
    $(SLO)$/xmldocumentwrapper_xmlsecimpl.obj \
    $(SLO)$/xmlelementwrapper_xmlsecimpl.obj \
    $(SLO)$/certificateextension_xmlsecimpl.obj \
    $(SLO)$/xmlstreamio.obj \
    $(SLO)$/errorcallback.obj \
    $(SLO)$/xsec_xmlsec.obj \
        $(SLO)$/diagnose.obj
    
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
