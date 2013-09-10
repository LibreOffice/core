# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,moz_runtime,mozruntime))

$(eval $(call gb_ExternalPackage_set_outdir,moz_runtime,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files_with_dir,moz_runtime,$(gb_Package_PROGRAMDIRNAME),\
	components/addrbook.dll \
	components/addrbook.xpt \
	components/i18n.dll \
	components/mork.dll \
	components/mozldap.dll \
	components/mozldap.xpt \
	components/necko.dll \
	components/necko_dns.xpt \
	components/pipnss.dll \
	components/pref.xpt \
	components/profile.dll \
	components/rdf.dll \
	components/uconv.dll \
	components/vcard.dll \
	components/xpc3250.dll \
	components/xpcom_base.xpt \
	components/xpcom_compat_c.dll \
	components/xpcom_components.xpt \
	components/xpcom_ds.xpt \
	components/xpcom_io.xpt \
	components/xpcom_obsolete.xpt \
	components/xpcom_thread.xpt \
	components/xpcom_xpti.xpt \
	components/xppref32.dll \
	defaults/autoconfig/platform.js \
	defaults/autoconfig/prefcalls.js \
	defaults/pref/browser-prefs.js \
	defaults/pref/mailnews.js \
	defaults/pref/mdn.js \
	defaults/pref/smime.js \
	greprefs/all.js \
	greprefs/security-prefs.js \
	js3250.dll \
	mozz.dll \
	msgbsutl.dll \
	nsldap32v50.dll \
	nsldappr32v50.dll \
	xpcom.dll \
	xpcom_compat.dll \
	xpcom_core.dll \
))

# vim: set noet sw=4 ts=4:
