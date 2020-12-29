# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,ui-previewer))

$(eval $(call gb_Executable_use_externals,ui-previewer,\
    boost_headers \
))

$(eval $(call gb_Executable_use_api,ui-previewer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_static_libraries,ui-previewer,\
    vclmain \
))

$(eval $(call gb_Executable_use_libraries,ui-previewer,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    vcl \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)), \
        acc \
        avmedia \
        basegfx \
        bib \
        bootstrap \
        cairocanvas \
        canvasfactory \
        canvastools \
        chartcontroller \
        chartcore \
        collator_data \
        configmgr \
        cppcanvas \
        ctl \
        cui \
        dba \
        dbtools \
        dict_ja \
        dict_zh \
        drawinglayer \
        editeng \
        embobj \
        emfio \
        epoxy \
        evtatt \
        expwrap \
        filterconfig \
        frm \
        fsstorage \
        fwk \
        gie \
        gie \
        graphicfilter \
        graphicfilter \
        guesslang \
        hyphen \
        i18nlangtag \
        i18npool \
        i18npool \
        i18nsearch \
        i18nutil \
        icg \
        index_data \
        introspection \
        invocadapt \
        invocation \
        lng \
        lnth \
        localebe1 \
        localedata_en \
        localedata_es \
        localedata_euro \
        localedata_others \
        msfilter \
        mtfrenderer \
        namingservice \
        numbertext \
        odfflatxml \
        oox \
        package2 \
        pdffilter \
        proxyfac \
        reflection \
        reg \
        salhelper \
        sax \
        sax \
        sb \
        sfx \
        simplecanvas \
        sot \
        spell \
        srtrs1 \
        stocservices \
        storagefd \
        store \
        svgfilter \
        svgio \
        svl \
        svt \
        svx \
        svxcore \
        t602filter \
        textconv_dict \
        textfd \
        tl \
        tk \
        ucb1 \
        ucbhelper \
        ucpdav1 \
        ucpfile1 \
        ucphier1 \
        ucppkg1 \
        ucptdoc1 \
        unoidl \
        unordf \
        unoxml \
        utl \
        uui \
        vclcanvas \
        xmlfa \
        xmlfd \
        xmlreader \
        xmlscript \
        xo \
        xof \
        xsltdlg \
        xsltfilter \
        xsltfilter \
        xstor \
        $(if $(filter EMSCRIPTEN,$(OS)),vclplug_qt5) \
    ) \
))

$(eval $(call gb_Executable_add_exception_objects,ui-previewer,\
    vcl/source/uipreviewer/previewer \
))

$(eval $(call gb_Executable_add_defs,ui-previewer,\
    -DVCL_INTERNALS \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Executable_add_libs,ui-previewer,\
    -lm $(DLOPEN_LIBS) \
    -lX11 \
))

$(eval $(call gb_Executable_use_static_libraries,ui-previewer,\
    glxtest \
))
endif

$(eval $(call gb_Executable_add_default_nativeres,ui-previewer))

# vim: set noet sw=4 ts=4:
