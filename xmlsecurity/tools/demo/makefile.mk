#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.14 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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


