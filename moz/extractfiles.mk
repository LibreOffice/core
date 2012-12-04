#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
MOZ_DIST_DIR=$(MISC)$/build$/mozilla$/$(BUILD_DIR)$/dist
MOZ_BIN_DIR=$(MOZ_DIST_DIR)$/bin
RUNTIME_DIR=$(MISC)$/$(MOZTARGET)runtime
LIB_DIR=$(LB)
INCLUDE_DIR=$(INCCOM)

BIN_RUNTIMELIST= \
    xpcom \
    xpcom_core \
    xpcom_compat \
    js3250 	\
    mozz	\
    msgbsutl	\
    nsldap32v50		\
    nsldappr32v50

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
    pipnss  \
    xppref32	\
    xpc3250

COMREGISTRY_FILELIST=	\
    necko_dns.xpt	\
    xpcom_base.xpt	\
    xpcom_obsolete.xpt	\
    xpcom_components.xpt	\
    xpcom_ds.xpt	\
    xpcom_io.xpt	\
    xpcom_xpti.xpt	\
    addrbook.xpt	\
    mozldap.xpt \
    pref.xpt  \
	xpcom_thread.xpt

DEFAULTS_RUNTIMELIST=	\
    defaults$/pref$/browser-prefs.js	\
    defaults$/pref$/mailnews.js	\
    defaults$/pref$/mdn.js	\
    defaults$/pref$/smime.js \
    defaults$/autoconfig$/platform.js	\
    defaults$/autoconfig$/prefcalls.js	\
    greprefs$/all.js	\
    greprefs$/security-prefs.js


LIBLIST=	\
    embed_base_s.lib	\
    mozreg_s.lib	\
    nslber32v50.lib	\
    nsldap32v50.lib	\
    xpcom_core.lib	\
    xpcom.lib

INCLUDE_PATH=$(MOZ_DIST_DIR)$/include$/
PUBLIC_PATH=$(MOZ_DIST_DIR)$/public$/


REG_SUBFIX=	.exe

# --- Targets ------------------------------------------------------

extract_mozab_files:	$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) \
    make_temp_dir	\
    $(MISC)$/build$/so_moz_runtime_files	\
    $(MISC)$/build$/so_moz_include_files	\
    $(MISC)$/build$/so_moz_lib_files

make_temp_dir:
    @@-$(MKDIRHIER)	$(RUNTIME_DIR)
    @@-$(MKDIRHIER)	$(RUNTIME_DIR)$/components
    @@-$(MKDIRHIER)	$(RUNTIME_DIR)$/defaults
    @@-$(MKDIRHIER)	$(RUNTIME_DIR)$/defaults$/pref
    @@-$(MKDIRHIER)	$(LIB_DIR)
    @@-$(MKDIRHIER)	$(INCLUDE_DIR)
.IF "$(OS)"=="SOLARIS"
    @@-$(MKDIRHIER)	$(RUNTIME_DIR)$/res
.ENDIF

$(OUT)$/bin$/mozruntime.zip: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    noop

$(MISC)$/build$/so_moz_runtime_files: 	$(OUT)$/bin$/mozruntime.zip
# copy files in BIN_RUNTIMELIST
    $(foreach,file,$(BIN_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$(file)$(DLLPOST) \
    $(RUNTIME_DIR)$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)

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


# zip runtime files to mozruntime.zip
    cd $(RUNTIME_DIR) && zip -r ..$/..$/bin$/mozruntime.zip *

    $(TOUCH) $@

$(INCCOM)$/nsBuildID.h: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @-echo "You can delete $(INCCOM) to force it copy all include files again."


$(MISC)$/build$/so_moz_include_files: $(INCCOM)$/nsBuildID.h
    $(COPY) -pr $(INCLUDE_PATH)* $(INCLUDE_DIR)
    $(COPY) -pr $(PUBLIC_PATH)* $(INCLUDE_DIR)
    $(TOUCH) $@

# On UNX the rules for so_moz_runtime_files copy files into the same directory
# used here (LIB_DIR), and on MACOSX all those files together need to be
# processed here, hence the dependency on so_moz_runtime_files:
$(MISC)$/build$/so_moz_lib_files:		$(foreach,file,$(LIBLIST) $(LIB_DIR)$/$(file)) $(MISC)$/build$/so_moz_runtime_files
    echo $(foreach,file,$(LIBLIST) $(MOZ_DIST_DIR)$/lib$/$(file))
    $(foreach,file,$(LIBLIST) $(COPY) $(MOZ_DIST_DIR)$/lib$/$(file) \
    $(LIB_DIR)$/$(file) &&) \
    echo >& $(NULLDEV)
    $(TOUCH) $@

$(BIN_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/$(DLLPRE)$@$(DLLPOST)

$(COMPONENT_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/components$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/components$/$(DLLPRE)$@$(DLLPOST)

$(COMREGISTRY_FILELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/components$/$@ $(RUNTIME_DIR)$/components$/$@

$(DEFAULTS_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @$(COPY) $(MOZ_BIN_DIR)$/$@ $(RUNTIME_DIR)$/$@

$(LIB_DIR)$/%: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    noop

$(MISC)$/CREATETARBALL:	extract_mozab_files
    @@-$(MKDIR)	$(OUT)$/zipped
    $(COPY) $(BIN)$/mozruntime.zip $(OUT)$/zipped$/$(MOZTARGET)runtime.zip
    cd $(LB) && zip -r ..$/zipped$/$(MOZTARGET)lib.zip *
    cd $(INCCOM) && zip -r ..$/zipped$/$(MOZTARGET)inc.zip *
