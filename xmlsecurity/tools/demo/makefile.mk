#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:47:42 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
JARFILES=       sandbox.jar ridl.jar jurt.jar unoil.jar juh.jar
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


