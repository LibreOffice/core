# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,sw_python))

$(eval $(call gb_PythonTest_use_configuration,sw_python))

$(eval $(call gb_PythonTest_use_api,sw_python,offapi))

# FAIL: this brings in GconfBackend $(eval $(call gb_PythonTest_use_rdb,sw_python,services))

$(eval $(call gb_PythonTest_use_components,sw_python,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sax/source/fastparser/fastsax \
    sw/util/sw \
    sw/util/swd \
    sw/util/msword \
    sw/util/vbaswobj \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_PythonTest_add_modules,sw_python,$(SRCDIR)/sw/qa/python,\
	set_expression \
	get_expression \
))

# vim: set noet sw=4 ts=4:
