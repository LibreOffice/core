# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

$(eval $(call gb_Library_Library,officelabs))

$(eval $(call gb_Library_set_include,officelabs,\
    $(INCLUDE) \
    -I$(SRCDIR)/officelabs/inc \
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
    officelabs/source/agent/AgentConnection \
    officelabs/source/agent/DocumentController \
    officelabs/source/ui/ChatPanel \
))

# vim: set noet sw=4 ts=4:
