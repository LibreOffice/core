#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: mt $ $Date: 2004-07-12 13:15:25 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************
PRJ=..$/..$/..$/..

PRJNAME=xmlsecurity
TARGET=xmlsecurity-secfit-boot
TARGETTYPE=GUI

VERSION=$(UPD)
GEN_HID=TRUE
APP2NOSAL=TRUE

# --- Settings -----------------------------------------------------------
USE_JAVAVER=TRUE

.INCLUDE :  settings.mk

VERINFONAME=verinfo

.INCLUDE :  target.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

# -------------------------------------------------------------------------
# --- Targets -------------------------------------------------------------

ALLTAR : $(BIN)$/boot_services.rdb

REGISTERLIBS= \
    i18npool.uno$(DLLPOST) \
    $(DLLPRE)tk$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
    $(DLLPRE)mcnttype$(DLLPOST)
    
#	$(DLLPRE)i18n$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)i18npool$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)tk$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
#	$(DLLPRE)mcnttype$(DLLPOST)

# Additional libraries
DLLPRE_NO=
REGISTERLIBS+= \
    $(DLLPRE_NO)shlibloader.uno$(DLLPOST) \
    $(DLLPRE_NO)dynamicloader.uno$(DLLPOST) \
    $(DLLPRE_NO)namingservice.uno$(DLLPOST) \
    $(DLLPRE_NO)servicemgr.uno$(DLLPOST) \
    $(DLLPRE)xsec_fw$(DLLPOST) \
    $(DLLPRE)xsec_xmlsec$(DLLPOST)

#	$(DLLPRE)implreg.uno$(DLLPOST) \
#	$(DLLPRE)nestedreg.uno$(DLLPOST) \
#	$(DLLPRE)simplereg.uno$(DLLPOST) \


.IF "$(GUI)" == "UNX"
MY_DLLDIR=$(SOLARLIBDIR)
REGISTERLIBS+= \
    $(DLLPRE)dtransX11$(UPD)$(DLLPOSTFIX)$(DLLPOST)

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
REGCOMP_JARS=unoil.jar java_uno.jar ridl.jar sandbox.jar jurt.jar juh.jar

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
    setenv CLASSPATH "$(strip $(subst,\,/ $(shell guw.pl echo $(REGCOMP_CLASSPATH))))" && \
    setenv PATH "$(PATH):$(JREPATH)"
MY_JAVA_COMPPATH=$(strip $(subst,\,/ file:///$(shell guw.pl echo $(SOLARBINDIR))))
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
    +$(REGCOMP_ENV) && \
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

