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
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),\
        dtoa \
        icuuc \
        icui18n \
        icudata \
        libpng \
        libjpeg \
        harfbuzz \
        libxml2 \
        graphite \
        liblangtag \
        qt5 \
        cairo \
        fontconfig \
        lcms2 \
        freetype \
        openssl \
        expat \
        orcus-parser \
        orcus \
        qrcodegen \
        boost_filesystem \
        boost_iostreams \
        boost_system \
        mdds_headers \
    ) \
))

$(eval $(call gb_Executable_use_api,ui-previewer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_static_libraries,ui-previewer,\
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),boost_locale) \
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
        cui \
        dbtools \
        drawinglayer \
        editeng \
        emfio \
        epoxy \
        fwk \
        gcc3_uno \
        gie \
        graphicfilter \
        i18nlangtag \
        i18nutil \
        lng \
        reg \
        salhelper \
        sax \
        sfx \
        sot \
        store \
        svl \
        svt \
        svx \
        svxcore \
        tk \
        ucbhelper \
        unoidl \
        utl \
        xmlreader \
        xo \
        xof \
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
