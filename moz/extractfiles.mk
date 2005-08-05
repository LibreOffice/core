#*************************************************************************
#
#   $RCSfile: extractfiles.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2005-08-05 12:58:00 $
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
    @+-$(MKDIR)	$(RUNTIME_DIR)	>& $(NULLDEV)
    @+-$(MKDIR)	$(RUNTIME_DIR)$/components	>& $(NULLDEV)
    @+-$(MKDIR)	$(RUNTIME_DIR)$/defaults	>& $(NULLDEV)
    @+-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/pref	>& $(NULLDEV)
    @+-$(MKDIR)	$(LIB_DIR)	>& $(NULLDEV)
    @+-$(MKDIR)	$(INCLUDE_DIR)	>& $(NULLDEV)
.IF "$(OS)"=="SOLARIS"
    +-$(MKDIR)	$(RUNTIME_DIR)$/res	>& $(NULLDEV)
.ENDIF

$(OUT)$/bin$/mozruntime.zip: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    +echo $@>& $(NULLDEV)

$(MISC)$/build$/so_moz_runtime_files: 	$(OUT)$/bin$/mozruntime.zip
# copy files in BIN_RUNTIMELIST
    +$(foreach,file,$(BIN_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$(file)$(DLLPOST) \
    $(RUNTIME_DIR)$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.IF "$(GUI)" == "UNX"
    +$(foreach,file,$(BIN_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$(file)$(DLLPOST) \
    $(LIB_DIR)$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ENDIF

# copy files in RES_FILELIST
.IF "$(OS)"=="SOLARIS"
    @+$(COPY) $(MOZ_BIN_DIR)$/res$/charsetalias.properties $(RUNTIME_DIR)$/res$/charsetalias.properties
.ELSE
    @echo No Res Files to copy.
.ENDIF

# copy files in COMPONENT_RUNTIMELIST
    +$(foreach,file,$(COMPONENT_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/components$/$(DLLPRE)$(file)$(DLLPOST) \
    $(RUNTIME_DIR)$/components$/$(DLLPRE)$(file)$(DLLPOST) &&) \
    echo >& $(NULLDEV)

# copy files in COMREGISTRY_FILELIST
    +$(foreach,file,$(COMREGISTRY_FILELIST) $(COPY) $(MOZ_BIN_DIR)$/components$/$(file) \
    $(RUNTIME_DIR)$/components$/$(file) &&) \
    echo >& $(NULLDEV)

# copy files in DEFAULTS_RUNTIMELIST
    @+-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/pref	>& $(NULLDEV)
    @+-$(MKDIR)	$(RUNTIME_DIR)$/defaults$/autoconfig	>& $(NULLDEV)
    @+-$(MKDIR)	$(RUNTIME_DIR)$/greprefs	>& $(NULLDEV)
    +$(foreach,file,$(DEFAULTS_RUNTIMELIST) $(COPY) $(MOZ_BIN_DIR)$/$(file) $(RUNTIME_DIR)$/$(file) &&) \
    echo >& $(NULLDEV)
# copy regxpcom
    @+$(COPY) $(MOZ_BIN_DIR)$/regxpcom$(REG_SUBFIX) $(RUNTIME_DIR)$/regxpcom$(REG_SUBFIX)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
    +cd $(RUNTIME_DIR) && strip *$(DLLPOST)
    +cd $(RUNTIME_DIR)$/components && strip *$(DLLPOST)
.ENDIF
.ENDIF

# zip runtime files to mozruntime.zip
    +cd $(RUNTIME_DIR) && .$/regxpcom$(REG_SUBFIX)
    +$(COPY) $(RUNTIME_DIR)$/components$/xpti.dat $(RUNTIME_DIR)$/components$/xptitemp.dat
    +$(RM) $(RUNTIME_DIR)$/regxpcom$(REG_SUBFIX)
    +cd $(RUNTIME_DIR) && zip -r ..$/..$/bin$/mozruntime.zip *
    
    +$(TOUCH) $@

$(INCCOM)$/nsBuildID.h: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @+-echo "You can delete $(INCCOM) to force it copy all include files again."
    
$(MISC)$/build$/so_moz_include_files: $(INCCOM)$/nsBuildID.h
.IF "$(USE_SHELL)"=="4nt"
    +$(COPY) /QSZ $(INCLUDE_PATH)* $(INCLUDE_DIR)
    +$(COPY) /QSZ $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ELSE
.IF "$(OS)"!="SOLARIS"
    +$(GNUCOPY) -prL $(INCLUDE_PATH)* $(INCLUDE_DIR)
    +$(GNUCOPY) -prL $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ELSE			# "$(OS)"!="SOLARIS"
    +$(COPY) -pr $(INCLUDE_PATH)* $(INCLUDE_DIR)
    +$(COPY) -pr $(PUBLIC_PATH)* $(INCLUDE_DIR)
.ENDIF			# "$(OS)"!="SOLARIS"
.ENDIF

.IF "$(GUI)"=="UNX"
    +chmod -R 775 $(INCCOM)
.ENDIF
    +$(TOUCH) $@

$(MISC)$/build$/so_moz_lib_files:		$(foreach,file,$(LIBLIST) $(LIB_DIR)$/$(file))
    +echo $(foreach,file,$(LIBLIST) $(MOZ_DIST_DIR)$/lib$/$(file))
    +$(foreach,file,$(LIBLIST) $(COPY) $(MOZ_DIST_DIR)$/lib$/$(file) \
    $(LIB_DIR)$/$(file) &&) \
    echo >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    +chmod -R 775 $(LB)
.ENDIF
    +$(TOUCH) $@
    
$(BIN_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @+$(COPY) $(MOZ_BIN_DIR)$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/$(DLLPRE)$@$(DLLPOST)

$(COMPONENT_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @+$(COPY) $(MOZ_BIN_DIR)$/components$/$(DLLPRE)$@$(DLLPOST) $(RUNTIME_DIR)$/components$/$(DLLPRE)$@$(DLLPOST)
    
$(COMREGISTRY_FILELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @+$(COPY) $(MOZ_BIN_DIR)$/components$/$@ $(RUNTIME_DIR)$/components$/$@

$(DEFAULTS_RUNTIMELIST): $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    @+$(COPY) $(MOZ_BIN_DIR)$/$@ $(RUNTIME_DIR)$/$@

RES_FILELIST: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
.IF "$(OS)"=="SOLARIS"
    @+$(COPY) $(MOZ_BIN_DIR)$/res$/charsetalias.properties $(RUNTIME_DIR)$/res$/charsetalias.properties
.ELSE
    @echo No Res Files to copy.
.ENDIF

    
$(LIB_DIR)$/%: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) 
    +echo  >& $(NULLDEV)

$(MISC)$/CREATETARBALL:	extract_mozab_files
    @+-$(MKDIR)	$(OUT)$/zipped	>& $(NULLDEV)
    +$(COPY) $(BIN)$/mozruntime.zip $(OUT)$/zipped$/$(MOZTARGET)runtime.zip
.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
    +cd $(LB) && strip *$(DLLPOST)
.ENDIF
.ENDIF
    +cd $(LB) && zip -r ..$/zipped$/$(MOZTARGET)lib.zip *
    +cd $(INCCOM) && zip -r ..$/zipped$/$(MOZTARGET)inc.zip *
