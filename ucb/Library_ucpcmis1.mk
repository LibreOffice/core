# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpcmis1))

$(eval $(call gb_Library_set_componentfile,ucpcmis1,ucb/source/ucp/cmis/ucpcmis1,services))

$(eval $(call gb_Library_use_sdk_api,ucpcmis1))

$(eval $(call gb_Library_use_libraries,ucpcmis1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	sax \
	ucbhelper \
	tl \
))

$(eval $(call gb_Library_use_externals,ucpcmis1,\
	boost_headers \
	boost_date_time \
	libcmis \
	curl \
	libxml2 \
))

# On Windows, libcmis.lib(ws-relatedmultipart.o) references BCryptCloseAlgorithmProvider,
# BCryptGenRandom, and BCryptOpenAlgorithmProvider via
# workdir/UnpackedTarball/boost/boost/winapi/bcrypt.hpp:
ifeq ($(OS),WNT)
ifeq ($(SYSTEM_LIBCMIS)$(SYSTEM_BOOST),)
$(eval $(call gb_Library_add_libs,ucpcmis1, \
    Bcrypt.lib \
))
endif
endif

$(eval $(call gb_Library_add_exception_objects,ucpcmis1,\
	ucb/source/ucp/cmis/auth_provider \
	ucb/source/ucp/cmis/certvalidation_handler \
	ucb/source/ucp/cmis/cmis_content \
	ucb/source/ucp/cmis/cmis_repo_content \
	ucb/source/ucp/cmis/cmis_datasupplier \
	ucb/source/ucp/cmis/cmis_provider \
	ucb/source/ucp/cmis/cmis_resultset \
	ucb/source/ucp/cmis/cmis_url \
	ucb/source/ucp/cmis/std_inputstream \
	ucb/source/ucp/cmis/std_outputstream \
))

# vim: set noet sw=4 ts=4:
