# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
# Portions created by the Initial Developer are Copyright (C) 2012 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_subsequent_rtfexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_subsequent_rtfexport, \
    sw/qa/extras/rtfexport/rtfexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_subsequent_rtfexport, \
    cppu \
	cppuhelper \
    sal \
	sw \
    test \
    unotest \
	utl \
	tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_subsequent_rtfexport,\
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_subsequent_rtfexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_subsequent_rtfexport,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_subsequent_rtfexport))

$(eval $(call gb_CppunitTest_use_components,sw_subsequent_rtfexport,\
	basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    embeddedobj/util/embobj \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    starmath/util/sm \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    sw/util/msword \
    sw/util/sw \
    sw/util/swd \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    writerfilter/util/writerfilter \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_subsequent_rtfexport))

$(eval $(call gb_CppunitTest_use_filter_configuration,sw_subsequent_rtfexport))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_subsequent_rtfexport))

# vim: set noet sw=4 ts=4:
