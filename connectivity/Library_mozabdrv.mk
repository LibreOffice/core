# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mozabdrv))

$(eval $(call gb_Library_set_include,mozabdrv,\
	-I$(SRCDIR)/connectivity/source/drivers/mozab \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc) \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/addrbook \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/content \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/embed_base \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/intl \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/mime \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/mozldap \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/msgbase \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/necko \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/pref \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/profile \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/rdf \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/string \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/uconv \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/xpcom \
	-I$(call gb_UnpackedTarball_get_dir,moz_inc)/xpcom_obsolete \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,mozabdrv))

$(eval $(call gb_Library_use_package,mozabdrv,moz_lib))

$(eval $(call gb_Library_use_unpacked,mozabdrv,moz_inc))

$(eval $(call gb_Library_use_libraries,mozabdrv,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,mozabdrv,\
	boost_headers \
	mozilla \
	nss3 \
))

$(eval $(call gb_Library_add_exception_objects,mozabdrv,\
	connectivity/source/drivers/mozab/MCatalog \
	connectivity/source/drivers/mozab/MColumnAlias \
	connectivity/source/drivers/mozab/MColumns \
	connectivity/source/drivers/mozab/MConfigAccess \
	connectivity/source/drivers/mozab/MConnection \
	connectivity/source/drivers/mozab/MDatabaseMetaData \
	connectivity/source/drivers/mozab/MPreparedStatement \
	connectivity/source/drivers/mozab/MResultSet \
	connectivity/source/drivers/mozab/MResultSetMetaData \
	connectivity/source/drivers/mozab/MStatement \
	connectivity/source/drivers/mozab/MTable \
	connectivity/source/drivers/mozab/MTables \
	connectivity/source/drivers/mozab/bootstrap/MMozillaBootstrap \
	connectivity/source/drivers/mozab/bootstrap/MNSFolders \
	connectivity/source/drivers/mozab/bootstrap/MNSINIParser \
	connectivity/source/drivers/mozab/bootstrap/MNSInit \
	connectivity/source/drivers/mozab/bootstrap/MNSProfile \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileDirServiceProvider \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileDiscover \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileManager \
	connectivity/source/drivers/mozab/bootstrap/MNSRunnable \
	connectivity/source/drivers/mozab/mozillasrc/MDatabaseMetaDataHelper \
	connectivity/source/drivers/mozab/mozillasrc/MLdapAttributeMap \
	connectivity/source/drivers/mozab/mozillasrc/MNSMozabProxy \
	connectivity/source/drivers/mozab/mozillasrc/MNSTerminateListener \
	connectivity/source/drivers/mozab/mozillasrc/MNameMapper \
	connectivity/source/drivers/mozab/mozillasrc/MQuery \
	connectivity/source/drivers/mozab/mozillasrc/MQueryHelper \
	connectivity/source/drivers/mozab/mozillasrc/MTypeConverter \
))

# vim: set noet sw=4 ts=4:
