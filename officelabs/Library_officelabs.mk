# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

$(eval $(call gb_Library_Library,officelabs))

$(eval $(call gb_Library_add_defs,officelabs,\
    -DOFFICELABS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_include,officelabs,\
    -I$(SRCDIR)/officelabs/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,officelabs))

$(eval $(call gb_Library_use_libraries,officelabs,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    tk \
    tl \
    utl \
    vcl \
    i18nlangtag \
))

$(eval $(call gb_Library_use_externals,officelabs,\
    boost_headers \
    curl \
))

$(eval $(call gb_Library_add_exception_objects,officelabs,\
    officelabs/source/AgentConnection \
    officelabs/source/DocumentController \
))

# === CEF WebView support (conditional on --with-cef) ===
ifeq ($(ENABLE_CEF),TRUE)

$(eval $(call gb_Library_add_defs,officelabs,\
    -DHAVE_FEATURE_CEF \
))

$(eval $(call gb_Library_set_include,officelabs,\
    -I$(SRCDIR)/officelabs/inc \
    -I$(CEF_DIR) \
    -I$(CEF_DIR)/include \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_libs,officelabs,\
    $(CEF_LIBS) \
    $(CEF_DIR)/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib \
))

$(eval $(call gb_Library_add_exception_objects,officelabs,\
    officelabs/source/CefInit \
    officelabs/source/WebViewPanel \
    officelabs/source/WebViewMessageHandler \
))

endif
# === End CEF ===

# vim: set noet sw=4 ts=4:
