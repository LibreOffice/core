# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_dialog_save))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_dialog_save, \
    dbaccess/qa/extras/dialog-save \
))

$(eval $(call gb_CppunitTest_use_externals,dbaccess_dialog_save,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_dialog_save, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    dba \
    dbu \
    sdbt \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    test \
    subsequenttest \
    tl \
    tk \
    ucbhelper \
	unotest \
    utl \
    vbahelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,dbaccess_dialog_save,\
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,dbaccess_dialog_save,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_dialog_save))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_dialog_save))

$(eval $(call gb_CppunitTest_use_components,dbaccess_dialog_save,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    dbaccess/util/dbu \
    dbaccess/util/sdbt \
    dbaccess/source/filter/xml/dbaxml \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_dialog_save))

$(call gb_CppunitTest_get_target,dbaccess_dialog_save) : $(WORKDIR)/CppunitTest/testDialogSave.odb
$(WORKDIR)/CppunitTest/testDialogSave.odb : $(SRCDIR)/dbaccess/qa/extras/testdocuments/testDialogSave.odb
	mkdir -p $(dir $@)
	cp -P -f "$<" "$@"
.PHONY: $(WORKDIR)/CppunitTest/testDialogSave.odb

$(call gb_CppunitTest_get_target,dbaccess_dialog_save): \
    $(call gb_AllLangResTarget_get_target,ofa)

# vim: set noet sw=4 ts=4:
