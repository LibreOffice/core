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
# $Revision: 1.7 $
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
PRJ=..$/..$/..$/..

PRJNAME=xmlsecurity
TARGET=xmlsecurity-mscsfit-boot
TARGETTYPE=GUI

GEN_HID=TRUE
APP2NOSAL=TRUE

# --- Settings -----------------------------------------------------------
USE_JAVAVER=TRUE

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

VERINFONAME=verinfo

.INCLUDE :  target.mk

# -------------------------------------------------------------------------
# --- Targets -------------------------------------------------------------

ALLTAR : $(BIN)$/boot_services.rdb

REGISTERLIBS= \
    i18npool.uno$(DLLPOST) \
    $(DLLPRE)tk$(DLLPOSTFIX)$(DLLPOST) \
    $(DLLPRE)mcnttype$(DLLPOST)
    
#	$(DLLPRE)i18n$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)i18npool$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)tk$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)mcnttype$(DLLPOST)

# Additional libraries
DLLPRE_NO=
REGISTERLIBS+= \
    $(DLLPRE_NO)dynamicloader.uno$(DLLPOST) \
    $(DLLPRE_NO)namingservice.uno$(DLLPOST) \
    $(DLLPRE_NO)bootstrap.uno$(DLLPOST) \
    $(DLLPRE)xsec_fw$(DLLPOST) \
    $(DLLPRE)xsec_xmlsec$(DLLPOST)

.IF "$(GUI)" == "UNX"
MY_DLLDIR=$(SOLARLIBDIR)
REGISTERLIBS+= \
    $(DLLPRE)dtransX11$(DLLPOSTFIX)$(DLLPOST)

.ELSE			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="WNT"

MY_DLLDIR=$(SOLARBINDIR)
REGISTERLIBS+= \
    $(DLLPRE)sysdtrans$(DLLPOST) \
    $(DLLPRE)ftransl$(DLLPOST) \
    $(DLLPRE)dnd$(DLLPOST)

.ELSE			# "$(GUI)"=="WNT"

    @echo "**********************************************************"
    @echo "*** unknown platform: don't know which librarys to use ***"
    @echo "**********************************************************"
    force_dmake_to_error
.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(GUI)" == "UNX"

.IF "$(SOLAR_JAVA)" != ""
.IF "$(JAVANUMVER)" >= "000100040000"
# native libraries, which are only necessary, when java shall run within setup
REGISTERLIBS_JAVA= \
    javavm.uno$(DLLPOST) \
    javaloader.uno$(DLLPOST)

# add here java components, which shall run with setup
REGISTERJARS=\
    java_uno_accessbridge.jar
#	xsec_jxsec.jar

# jar-files, which regcomp needs so that it can use java
REGCOMP_JARS=unoil.jar java_uno.jar ridl.jar jurt.jar juh.jar

# CLASSPATH, which regcomp needs to be run
# $(SOLARLIBDIR) needs to be included in case Java components are registered,
# because java_uno.jar needs to find the native java_uno shared library:
MY_CLASSPATH_TMP=$(foreach,i,$(REGCOMP_JARS) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)$(SOLARLIBDIR)$(PATH_SEPERATOR)$(XCLASSPATH)
REGCOMP_CLASSPATH=$(strip $(subst,!,$(PATH_SEPERATOR) $(MY_CLASSPATH_TMP:s/ /!/)))

.IF "$(GUI)"!="WNT"
DOLLAR_SIGN=\$$
MY_JAVA_COMPPATH=file://$(SOLARBINDIR)
.IF "$(OS)"=="MACOSX"
REGCOMP_ENV=\
    setenv CLASSPATH $(REGCOMP_CLASSPATH) && \
    setenv DYLD_LIBRARY_PATH $(DYLD_LIBRARY_PATH):$(JDKLIB)
.ELSE
REGCOMP_ENV=\
    setenv CLASSPATH $(REGCOMP_CLASSPATH) && \
    setenv LD_LIBRARY_PATH $(LD_LIBRARY_PATH):$(JDKLIB)
.ENDIF
.ELSE # "$(GUI)" != "WNT"
.IF "$(USE_SHELL)" != "4nt"
DOLLAR_SIGN=\$$
REGCOMP_ENV=\
    setenv CLASSPATH "$(strip $(subst,\,/ $(shell @guw.pl echo $(REGCOMP_CLASSPATH))))" && \
    setenv PATH "$(PATH):$(JREPATH)"
MY_JAVA_COMPPATH=$(strip $(subst,\,/ file:///$(shell @guw.pl echo $(SOLARBINDIR))))
.ELSE # "$(USE_SHELL)" != "4nt"
DOLLAR_SIGN=$$
REGCOMP_ENV=\
    set CLASSPATH=$(REGCOMP_CLASSPATH) && \
    set PATH=$(PATH);$(JREPATH)
MY_JAVA_COMPPATH=$(strip $(subst,\,/ file:///$(SOLARBINDIR)))
.ENDIF  # "$(USE_SHELL)" != "4nt"
.ENDIF  # "$(GUI)"!="WNT"
.ENDIF  # "$(JAVANUMVER)" >= "000100040000"
.ENDIF  # "$(SOLAR_JAVA)" != ""

$(BIN)$/boot_services.rdb: \
        makefile.mk \
        $(foreach,i,$(REGISTERLIBS) $(MY_DLLDIR)$/$(i)) \
        $(foreach,i,$(REGISTERLIBS_JAVA) $(MY_DLLDIR)$/$(i))
    -rm -f $@ $(BIN)$/regcomp.rdb $(BIN)$/boot_services.tmp
# register the native libraries
    $(REGCOMP) -register -r $(BIN)$/boot_services.tmp -c "$(strip $(REGISTERLIBS))"
.IF "$(SOLAR_JAVA)" != ""
.IF "$(OS)"!="MACOSX"
.IF "$(JAVANUMVER)" >= "000100040000"
    $(REGCOMP) -register -r $(BIN)$/boot_services.tmp -c "$(strip $(REGISTERLIBS_JAVA))"
# prepare a registry which regcomp can work on (types+java services)
    $(REGCOMP) -register -r $(BIN)$/regcomp.rdb -c "$(strip $(REGISTERLIBS_JAVA))"
    $(REGMERGE) $(BIN)$/regcomp.rdb / $(SOLARBINDIR)/udkapi.rdb
# now do the registration
    $(REGCOMP_ENV) && \
        $(REGCOMP) -br $(BIN)$/regcomp.rdb 			\
            -register					\
            -r $(BIN)$/boot_services.tmp 		\
            -l com.sun.star.loader.Java2			\
            -env:UNO_JAVA_COMPPATH=$(MY_JAVA_COMPPATH)	\
            $(foreach,i,$(REGISTERJARS) -c vnd.sun.star.expand:$(DOLLAR_SIGN)UNO_JAVA_COMPPATH/$(i))
#			$(foreach,i,$(REGISTERJARS) -c vnd.sun.star.expand:$(DOLLAR_SIGN)UNO_JAVA_COMPPATH/$(i)) -c $(MY_JAVA_COMPPATH)$/xsec_jxsec.jar

.ENDIF # "$(JAVANUMVER)" >= "000100040000"
.ENDIF # "$(OS)" != "MACOSX"
.ENDIF #  "$(SOLAR_JAVA)" != ""
# to have a transaction, move it now to the final name
    $(REGMERGE) $(BIN)$/boot_services.tmp / $(SOLARBINDIR)/types.rdb
    mv $(BIN)$/boot_services.tmp $@

