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

PRJNAME=xmlsecurity
TARGET=demo
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------

SHARE_LIBS =			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(SALLIB)			\
    $(UCBHELPERLIB)		\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(XMLOFFLIB)		\
    $(LIBXML2LIB)		\
    $(NSS3LIB)			\
    $(NSPR4LIB)			\
    $(XMLSECLIB)		\
    $(COMPHELPERLIB)

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
SHARE_LIBS+= $(XMLSECLIB-MS)
.ELSE
SHARE_LIBS+= $(XMLSECLIB-NSS)
.ENDIF



# HACK: Use SLO for demo directly...
SHARE_OBJS =	\
    $(OBJ)$/util.obj \
    $(OBJ)$/util2.obj \
    $(SLO)$/biginteger.obj \
    $(SLO)$/baseencoding.obj \
    $(SLO)/xmlsignaturehelper.obj	\
    $(SLO)/xmlsignaturehelper2.obj	\
    $(SLO)/xsecctl.obj	\
    $(SLO)/xsecparser.obj	\
    $(SLO)/xsecsign.obj	\
    $(SLO)/xsecverify.obj

#
# ---------- signdemo ----------
#
APP1TARGET=signdemo
APP1DEPN=makefile.mk
APP1STDLIBS+=$(SHARE_LIBS)
APP1OBJS= $(SHARE_OBJS)	$(OBJ)$/signdemo.obj

#
# ---------- verifydemo ----------
#
APP2TARGET=verifydemo
APP2DEPN=makefile.mk
APP2STDLIBS+=$(SHARE_LIBS)
APP2OBJS= $(SHARE_OBJS)	$(OBJ)$/verifydemo.obj

#
# ---------- multisigdemo ----------
#
APP3TARGET=multisigdemo
APP3DEPN=makefile.mk
APP3STDLIBS+=$(SHARE_LIBS)
APP3OBJS= $(SHARE_OBJS)	$(OBJ)$/multisigdemo.obj

#
# ---------- mozprofile ----------
#
APP4TARGET=mozprofile
APP4DEPN=makefile.mk
APP4STDLIBS+=$(SHARE_LIBS)
APP4OBJS= $(SHARE_OBJS)	$(OBJ)$/mozprofile.obj

#
# ---------- performance ----------
#
APP5TARGET=performance
APP5DEPN=makefile.mk
APP5STDLIBS+=$(SHARE_LIBS)
APP5OBJS= $(OBJ)$/util.obj	$(OBJ)$/performance.obj

#
# ---------- jflatfilter ----------
#
PACKAGE=	    com$/sun$/star$/xml$/security$/eval
JARFILES=       ridl.jar jurt.jar unoil.jar juh.jar
JAVAFILES:=     $(shell @ls *.java)
JAVACLASSFILES= $(CLASSDIR)$/$(PACKAGE)$/JavaFlatFilter.class
JARCLASSDIRS=   $(PACKAGE)
JARTARGET=      jflatfilter.jar
JARCOMPRESS=    TRUE


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(BIN)$/demo.rdb

$(JAVACLASSFILES) : $(JAVAFILES)

REGISTERLIBS=					\
    dynamicloader.uno$(DLLPOST) \
    namingservice.uno$(DLLPOST) \
    bootstrap.uno$(DLLPOST)	\
    sax.uno$(DLLPOST)			\
    $(DLLPRE)mozab2$(DLLPOST)

$(BIN)$/demo.rdb: \
        makefile.mk \
    $(foreach,i,$(REGISTERLIBS) $(SOLARSHAREDBIN)$/$(i))
    -rm -f $@ $(BIN)$/regcomp.rdb $(BIN)$/demo.tmp
    $(REGCOMP) -register -r $(BIN)$/demo.tmp -c "$(strip $(REGISTERLIBS))"
    $(REGCOMP) -register -r $(BIN)$/demo.tmp -c $(DLLPRE)xsec_fw$(DLLPOST)
    $(REGCOMP) -register -r $(BIN)$/demo.tmp -c $(DLLPRE)xsec_xmlsec$(DLLPOST)
    $(REGMERGE) $(BIN)$/demo.tmp / $(SOLARBINDIR)/types.rdb
    mv $(BIN)$/demo.tmp $@


