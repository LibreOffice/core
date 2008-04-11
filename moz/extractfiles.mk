#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: extractfiles.mk,v $
#
# $Revision: 1.13 $
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

MOZTARGET=$(OS)$(COM)$(CPU)
MOZ_DIST_DIR=$(MISC)$/build$/mozilla$/dist
MOZ_BIN_DIR=$(MOZ_DIST_DIR)$/bin
RUNTIME_DIR=$(MISC)$/$(MOZTARGET)runtime
LIB_DIR=$(LB)
INCLUDE_DIR=$(INCCOM)

BIN_RUNTIMELIST=	\
    nspr4	\
    plc4	\
    plds4	\
    xpcom	\
    xpcom_compat	\
    nss3	\
    ssl3	\
    softokn3	\
    smime3
    
.IF "$(GUI)"=="WNT"
BIN_RUNTIMELIST+=	\
    js3250 	\
    mozz	\
    msgbsutl	\
    nsldap32v50		\
    nsldappr32v50
.ELSE	#"$(GUI)"=="WNT"
BIN_RUNTIMELIST+=	\
    mozjs	\
    mozz	\
    msgbaseutil	\
    ldap50	\
    prldap50
.IF	"$(OS)"=="SOLARIS"
.IF	"$(CPU)"=="S"
BIN_RUNTIMELIST+=	\
    freebl_hybrid_3
.ENDIF
.ENDIF #"$(OS)"=="SOLARIS"
.ENDIF

COMPONENT_RUNTIMELIST=	\
    addrbook	\
    mork	\
    mozldap	\
    necko	\
    profile	\
    xpcom_compat_c	\
    rdf	\
    uconv	\
    vcard	\
    i18n 	\
    pipnss
    
.IF "$(GUI)"=="WNT"
COMPONENT_RUNTIMELIST+=	\
    xppref32	\
    xpc3250
.ELSE	#"$(GUI)"=="WNT"
COMPONENT_RUNTIMELIST+=	\
    pref	\
    xpconnect
.ENDIF

COMREGISTRY_FILELIST=	\
    necko_dns.xpt	\
    xpcom_base.xpt	\
    xpcom_obsolete.xpt	\
    xpcom_components.xpt	\
    xpcom_ds.xpt	\
    xpcom_io.xpt	\
    xpcom_xpti.xpt	\
    addrbook.xpt	\
    mozldap.xpt

.IF "$(GUI)"=="WNT"
COMREGISTRY_FILELIST+=	xpcom_thread.xpt
.ELSE	#"$(GUI)"=="WNT"
COMREGISTRY_FILELIST+=	xpcom_threads.xpt
.ENDIF

DEFAULTS_RUNTIMELIST=	\
    defaults$/pref$/browser-prefs.js	\
    defaults$/pref$/mailnews.js	\
    defaults$/pref$/mdn.js	\
    defaults$/pref$/smime.js \
    defaults$/autoconfig$/platform.js	\
    defaults$/autoconfig$/prefcalls.js	\
    greprefs$/all.js	\
    greprefs$/security-prefs.js

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
LIBLIST=	\
    libembed_base_s.a	\
    libmozreg_s.a	\
    libnslber32v50.a	\
    libnsldap32v50.a	\
    libnspr4.a 	\
    libxpcom.dll.a	\
    libnss3.a	\
    libsmime3.a
.ELSE
LIBLIST=	\
    embed_base_s.lib	\
    mozreg_s.lib	\
    nslber32v50.lib	\
    nsldap32v50.lib	\
    nspr4.lib 	\
    xpcom.lib	\
    plc4.lib	\
    plds4.lib	\
    nss3.lib	\
    ssl3.lib	\
    smime3.lib
.ENDIF
.ELSE	#"$(GUI)"=="WNT"
LIBLIST=	\
    libembed_base_s.a	\
    libmozreg_s.a	\
    liblber50.a	\
    libnspr4$(DLLPOST)	\
    libxpcom$(DLLPOST)	\
    libmsgbaseutil$(DLLPOST)	\
    libldap50$(DLLPOST) \
    libsoftokn3$(DLLPOST) \
    libplc4$(DLLPOST) \
    libplds4$(DLLPOST) \
    libnss3$(DLLPOST)	\
    libssl3$(DLLPOST)	\
    libsmime3$(DLLPOST)
.ENDIF

INCLUDE_PATH=$(MOZ_DIST_DIR)$/include$/
PUBLIC_PATH=$(MOZ_DIST_DIR)$/public$/


.IF "$(GUI)"=="WNT"
REG_SUBFIX=	.exe
.ELSE	#"$(GUI)"=="WNT"
REG_SUBFIX=
.ENDIF

# --- Targets ------------------------------------------------------

extract_mozab_files:	$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) \
    make_temp_dir	\
    $(MISC)$/build$/so_moz_runtime_files	\
    $(MISC)$/build$/so_moz_include_files	\
    $(MISC)$/build$/so_moz_lib_files
    
make_temp_dir:
    @@-$(MKDIR)	$(RUNTIME_DIR)
    @@-$(MKDIR)	$(RUNTIME_DIR)$/components
    @@-$(MKDIR)	$(RUNTIME_DIR)$/defaults
    @@-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/pref
    @@-$(MKDIR)	$(LIB_DIR)
    @@-$(MKDIR)	$(INCLUDE_DIR)
.IF "$(OS)"=="SOLARIS"
    @@-$(MKDIR)	$(RUNTIME_DIR)$/res
.ENDIF

$(OUT)$/bin$/mozruntime.zip: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    noop

$(MISC)$/build$/so_moz_runtime_files: 	$(OUT)$/bin$/mozruntime.zip
# copy files in BIN_RUNTIMELIST
    $(foreach,file,$(BIN_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$(file)$(DLLPOST) \
    $(RUNTIME_DIR)$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.IF "$(GUI)" == "UNX"
    $(foreach,file,$(BIN_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$(file)$(DLLPOST) \
    $(LIB_DIR)$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ENDIF

# copy files in RES_FILELIST
.IF "$(OS)"=="SOLARIS"
    @$(COPY) $(MOZ_BIN_DIR)$/res$/charsetalias.properties $(RUNTIME_DIR)$/res$/charsetalias.properties
.ELSE
    @echo No Res Files to copy.
.ENDIF

# copy files in COMPONENT_RUNTIMELIST
    $(foreach,file,$(COMPONENT_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/components$/$(DLLPRE)$(file)$(DLLPOST) \
    $(RUNTIME_DIR)$/components$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)

# copy files in COMREGISTRY_FILELIST
    $(foreach,file,$(COMREGISTRY_FILELIST) $(COPY) $(MOZ_BIN_DIR)$/components$/$(file) \
    $(RUNTIME_DIR)$/components$/$(file) &&) \
    echo >& $(NULLDEV)

# copy files in DEFAULTS_RUNTIMELIST
    @@-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/pref
    @@-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/autoconfig
    @@-$(MKDIR)	$(RUNTIME_DIR)$/greprefs
    $(foreach,file,$(DEFAULTS_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(file) $(RUNTIME_DIR)$/$(file) &&) \
    echo >& $(NULLDEV)
# copy regxpcom
    @$(COPY) $(MOZ_BIN_DIR)$/regxpcom$(REG_SUBFIX) $(RUNTIME_DIR)$/regxpcom$(REG_SUBFIX)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
    cd $(RUNTIME_DIR) && strip *$(DLLPOST)
    cd $(RUNTIME_DIR)$/components && strip *$(DLLPOST)
.ENDIF
.ENDIF
.IF "$(OS)"=="MACOSX"
    $(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl extshl OOO \
        $(RUNTIME_DIR)$/*$(DLLPOST)
# A crude hack to adapt all the install names in the components subdir:
    $(foreach,file,$(shell ls $(RUNTIME_DIR)$/components$/*$(DLLPOST)) \
        install_name_tool \
        -change @executable_path/libldap50.dylib \
            @loader_path/../libldap50.dylib \
        -change @executable_path/libmozjs.dylib @loader_path/../libmozjs.dylib \
        -change @executable_path/libmozz.dylib @loader_path/../libmozz.dylib \
        -change @executable_path/libmsgbaseutil.dylib \
            @loader_path/../libmsgbaseutil.dylib \
        -change @executable_path/libnspr4.dylib @loader_path/../libnspr4.dylib \
        -change @executable_path/libnss3.dylib @loader_path/../libnss3.dylib \
        -change @executable_path/libplc4.dylib @loader_path/../libplc4.dylib \
        -change @executable_path/libplds4.dylib @loader_path/../libplds4.dylib \
        -change @executable_path/libprldap50.dylib \
            @loader_path/../libprldap50.dylib \
        -change @executable_path/libsmime3.dylib \
            @loader_path/../libsmime3.dylib \
        -change @executable_path/libsoftokn3.dylib \
            @loader_path/../libsoftokn3.dylib \
        -change @executable_path/libssl3.dylib @loader_path/../libssl3.dylib \
        -change @executable_path/libxpcom.dylib @loader_path/../libxpcom.dylib \
        -change @executable_path/libxpcom_compat.dylib \
            @loader_path/../libxpcom_compat.dylib \
        $(file) &&) true
.ENDIF

# zip runtime files to mozruntime.zip
    cd $(RUNTIME_DIR) && .$/regxpcom$(REG_SUBFIX)
    $(COPY) $(RUNTIME_DIR)$/components$/xpti.dat $(RUNTIME_DIR)$/components$/xptitemp.dat
    $(RM) $(RUNTIME_DIR)$/regxpcom$(REG_SUBFIX)
    cd $(RUNTIME_DIR) && zip -r ..$/..$/bin$/mozruntime.zip *
    
    $(TOUCH) $@

$(INCCOM)$/nsBuildID.h: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @-echo "You can delete $(INCCOM) to force it copy all include files again."
    
$(MISC)$/build$/so_moz_include_files: $(INCCOM)$/nsBuildID.h
.IF "$(USE_SHELL)"=="4nt"
    $(COPY) /QSZ $(INCLUDE_PATH)* $(INCLUDE_DIR)
    $(COPY) /QSZ $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ELSE
.IF "$(OS)"!="SOLARIS"
    $(GNUCOPY) -pRL $(INCLUDE_PATH)* $(INCLUDE_DIR)
    $(GNUCOPY) -pRL $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ELSE			# "$(OS)"!="SOLARIS"
    $(COPY) -pr $(INCLUDE_PATH)* $(INCLUDE_DIR)
    $(COPY) -pr $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ENDIF			# "$(OS)"!="SOLARIS"
.ENDIF

.IF "$(GUI)"=="UNX"
    chmod -R 775 $(INCCOM)
.ENDIF
    $(TOUCH) $@

# On UNX the rules for so_moz_runtime_files copy files into the same directory
# used here (LIB_DIR), and on MACOSX all those files together need to be
# processed here, hence the dependency on so_moz_runtime_files:
$(MISC)$/build$/so_moz_lib_files:		$(foreach,file,$(LIBLIST) $(LIB_DIR)$/$(file)) $(MISC)$/build$/so_moz_runtime_files
    echo $(foreach,file,$(LIBLIST) $(MOZ_DIST_DIR)$/lib$/$(file))
    $(foreach,file,$(LIBLIST) $(COPY) $(MOZ_DIST_DIR)$/lib$/$(file) \
    $(LIB_DIR)$/$(file) &&) \
    echo >& $(NULLDEV)
.IF "$(OS)"=="MACOSX"
    $(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl extshl OOO \
        $(LIB_DIR)$/*$(DLLPOST)
.ENDIF
.IF "$(GUI)"=="UNX"
    chmod -R 775 $(LB)
.ENDIF
    $(TOUCH) $@
    
$(BIN_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/$(DLLPRE)$@$(DLLPOST)

$(COMPONENT_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/components$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/components$/$(DLLPRE)$@$(DLLPOST)
    
$(COMREGISTRY_FILELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/components$/$@ $(RUNTIME_DIR)$/components$/$@

$(DEFAULTS_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/$@ $(RUNTIME_DIR)$/$@

RES_FILELIST: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
.IF "$(OS)"=="SOLARIS"
    @$(COPY) $(MOZ_BIN_DIR)$/res$/charsetalias.properties $(RUNTIME_DIR)$/res$/charsetalias.properties
.ELSE
    @echo No Res Files to copy.
.ENDIF

    
$(LIB_DIR)$/%: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    noop

$(MISC)$/CREATETARBALL:	extract_mozab_files
    @@-$(MKDIR)	$(OUT)$/zipped
    $(COPY) $(BIN)$/mozruntime.zip $(OUT)$/zipped$/$(MOZTARGET)runtime.zip
.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
    cd $(LB) && strip *$(DLLPOST)
.ENDIF
.ENDIF
    cd $(LB) && zip -r ..$/zipped$/$(MOZTARGET)lib.zip *
    cd $(INCCOM) && zip -r ..$/zipped$/$(MOZTARGET)inc.zip *
