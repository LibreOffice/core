###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



# this file describes all the external libraries
# depending on the configure options these may be taken from the system,
# or the internal/bundled copy may be built.

# for every external, a function gb_LinkTarget_use__FOO is defined,
# once for the system case, once for the internal case.

# in the system case, no libraries should be registered, but the target-local
# variable LIBS should be set to FOO_LIBS, and INCLUDES to FOO_CFLAGS.


ifeq ($(SYSTEM_ZLIB),YES)

define gb_LinkTarget__use_zlib
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_ZLIB \
)
$(call gb_LinkTarget_add_libs,$(1),-lz)
endef

else # !SYSTEM_ZLIB

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    zlib \
))

define gb_LinkTarget__use_zlib
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    zlib \
)
endef

endif # SYSTEM_ZLIB


ifeq ($(SYSTEM_JPEG),YES)

define gb_LinkTarget__use_jpeg
$(call gb_LinkTarget_add_libs,$(1),-ljpeg)
$(call gb_LinkTarget_set_ldflags,$(1),\
    $$(filter-out -L/usr/lib/jvm%,$$(T_LDFLAGS)) \
)
ifeq ($(OS),FREEBSD)
# JDK jpeg library on FreeBSD lives under ${JAVA_HOME}/jre/lib/${ARCH}/
$(call gb_LinkTarget_set_ldflags,$(1),\
    $$(filter-out -L$$(JAVA_HOME)/jre/lib/%,$$(T_LDFLAGS)) \
)
endif
endef

else # !SYSTEM_JPEG

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    jpeglib \
))

define gb_LinkTarget__use_jpeg
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    jpeglib \
)
endef

endif # SYSTEM_JPEG


ifeq ($(SYSTEM_EXPAT),YES)

define gb_LinkTarget__use_expat
$(if $(2),,$(error gb_LinkTarget__use_expat needs additional parameter))

$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_EXPAT \
)

$(call gb_LinkTarget_add_libs,$(1),-lexpat)
endef

else # !SYSTEM_EXPAT

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    ascii_expat_xmlparse \
    expat_xmlparse \
    expat_xmltok \
))

define gb_LinkTarget__use_expat
$(if $(2),,$(error gb_LinkTarget__use_expat needs additional parameter))

$(if $(filter-out ascii_expat_xmlparse,$(2)),\
    $(call gb_LinkTarget_add_defs,$(1),\
        -DXML_UNICODE \
))

$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    $(2) \
    expat_xmltok \
)
endef

endif # SYSTEM_EXPAT

# now define 2 wrappers that select which internal static library to use...
define gb_LinkTarget__use_expat_utf8
$(call gb_LinkTarget__use_expat,$(1),ascii_expat_xmlparse)
endef

define gb_LinkTarget__use_expat_utf16
$(call gb_LinkTarget__use_expat,$(1),expat_xmlparse)
endef


ifeq ($(SYSTEM_LIBXML),YES)

define gb_LinkTarget__use_libxml2
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_LIBXML \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(LIBXML_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBXML_LIBS))
endef

else # !SYSTEM_LIBXML

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    xml2 \
))

define gb_LinkTarget__use_libxml2
$(call gb_LinkTarget_add_linked_libs,$(1),\
    xml2 \
)
endef

endif # SYSTEM_LIBXML


ifeq ($(SYSTEM_LIBXSLT),YES)

define gb_LinkTarget__use_libxslt
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(LIBXSLT_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBXSLT_LIBS))
endef

else # !SYSTEM_LIBXSLT

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    xslt \
))

define gb_LinkTarget__use_libxslt
$(call gb_LinkTarget_add_linked_libs,$(1),\
    xslt \
)
endef

endif # SYSTEM_LIBXSLT


ifeq ($(SYSTEM_REDLAND),YES)

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_REDLAND \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(REDLAND_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(REDLAND_LIBS))
endef

else # !SYSTEM_REDLAND

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    rdf \
))

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_add_linked_libs,$(1),\
    rdf \
)
endef

endif # SYSTEM_REDLAND


ifeq ($(SYSTEM_CAIRO),YES)

# FREETYPE_CLAGS from environment if ENABLE_CAIRO is used
define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(FREETYPE_CFLAGS) \
    $(CAIRO_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CAIRO_LIBS))
endef

else # !SYSTEM_CAIRO

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    cairo \
))
ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    pixman-1 \
))
endif # WNT

define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/cairo \
    $(FREETYPE_CFLAGS) \
)
$(call gb_LinkTarget_add_linked_libs,$(1),\
    cairo \
)
ifneq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),\
    pixman-1 \
)
endif
endef

endif # SYSTEM_CAIRO


ifeq ($(SYSTEM_GRAPHITE),YES)

#check for building with stlport, but graphite was not built with stlport
define gb_LinkTarget__use_graphite
ifneq ($(USE_SYSTEM_STL),YES)
$(call gb_LinkTarget_add_defs,$(1),\
    -DADAPT_EXT_STL \
)
endif
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(GRAPHITE_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(GRAPHITE_LIBS))
endef

else # !SYSTEM_GRAPHITE

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    graphite \
))
define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    graphite \
)
endef

endif # SYSTEM_GRAPHITE


ifeq ($(SYSTEM_ICU),YES)

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_libs,$(1),-licule)
endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_libs,$(1),-licuuc)
endef

else # !SYSTEM_ICU

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    icule \
    icuuc \
))

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_linked_libs,$(1),\
    icule \
)
endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_linked_libs,$(1),\
    icuuc \
)
endef

endif # SYSTEM_ICU


ifeq ($(SYSTEM_OPENSSL),YES)

define gb_LinkTarget__use_openssl
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(OPENSSL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(OPENSSL_LIBS))
endef

else # !SYSTEM_OPENSSL

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    crypto \
    ssl \
))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    crypto \
    ssl \
))
endif

define gb_LinkTarget__use_openssl
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),\
    crypto \
    ssl \
)
else
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    crypto \
    ssl \
)
ifeq ($(OS),SOLARIS)
$(call gb_LinkTarget_add_libs,$(1),\
    -lnsl \
    -lsocket \
)
endif
endif
endef

endif # SYSTEM_OPENSSL


ifeq ($(SYSTEM_APR),YES)

define gb_LinkTarget__use_apr
$(call gb_LinkTarget_add_defs,$(1),\
        -DSYSTEM_APR \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(APR_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(APR_LIBS))
endef

else # !SYSTEM_APR

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,libapr-1))
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,apr-1))
endif

define gb_LinkTarget__use_apr
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/apr \
)
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),libapr-1)
else
$(call gb_LinkTarget_add_linked_libs,$(1),apr-1)
endif
endef

endif # SYSTEM_APR


ifeq ($(SYSTEM_APR_UTIL),YES)

define gb_LinkTarget__use_apr_util
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_APR_UTIL \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(APR_UTIL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(APR_UTIL_LIBS))
endef

else # !SYSTEM_APR_UTIL

# on Windows apr-util is registered by ext_libraries/Repository.mk
ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,aprutil-1))
endif

define gb_LinkTarget__use_apr_util
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/apr-util \
)
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),apr-util)
else
$(call gb_LinkTarget_add_linked_libs,$(1),aprutil-1)
endif
endef

endif # SYSTEM_APR_UTIL


ifeq ($(SYSTEM_SERF),YES)

define gb_LinkTarget__use_serf
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_SERF \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(SERF_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(SERF_LIBS))
endef

else # !SYSTEM_SERF

# on Windows serf is registered by ext_libraries/Repository.mk
ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,serf-1))
endif

define gb_LinkTarget__use_serf
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/serf \
)
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),serf)
else
$(call gb_LinkTarget_add_linked_libs,$(1),serf-1)
endif
endef

endif # SYSTEM_SERF


ifeq ($(SYSTEM_CURL),YES)

define gb_LinkTarget__use_curl
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_CURL \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(CURL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CURL_LIBS))
endef

else # !SYSTEM_CURL

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,libcurl))
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,curl))
endif

define gb_LinkTarget__use_curl
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),libcurl)
else
$(call gb_LinkTarget_add_linked_libs,$(1),curl)
endif
endef

endif # SYSTEM_CURL


ifeq ($(SYSTEM_COINMP),YES)

define gb_LinkTarget__use_coinmp
$(call gb_LinkTarget_add_defs,$(1),\
    -DSYSTEM_COINMP \
)
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(COINMP_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(COINMP_LIBS))
endef

else # !SYSTEM_COINMP

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,CoinMP))
ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
    CoinUtils \
    Clp \
    Cbc \
    Osi \
    OsiClp \
    Cgl \
    CbcSolver \
))
endif

define gb_LinkTarget__use_coinmp
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/coinmp \
)
$(call gb_LinkTarget_add_linked_libs,$(1),CoinMP)
ifneq ($(OS),WNT)
ifneq ($(OS),OS2)
$(call gb_LinkTarget_add_linked_libs,$(1),\
    CoinUtils \
    Clp \
    Cbc \
    Osi \
    OsiClp \
    Cgl \
    CbcSolver \
)
endif
endif
endef

endif # SYSTEM_COINMP


ifeq ($(SYSTEM_HYPH),YES)

define gb_LinkTarget__use_hyphen
$(call gb_LinkTarget_add_defs,$(1),\
        -DSYSTEM_HYPH \
)
$(call gb_LinkTarget_add_libs,$(1),$(HYPHEN_LIB))
endef

else # !SYSTEM_HYPH

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
        hyphen \
))

define gb_LinkTarget__use_hyphen
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
        hyphen \
)
endef

endif # SYSTEM_HYPH


ifeq ($(SYSTEM_LIBTEXTCAT),YES)

define gb_LinkTarget__use_libtextcat
$(call gb_LinkTarget_add_libs,$(1),-ltextcat)
endef

else # !SYSTEM_LIBTEXTCAT

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    libtextcat \
    textcat \
))

define gb_LinkTarget__use_libtextcat
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_libs,$(1),libtextcat)
else
$(call gb_LinkTarget_add_linked_libs,$(1),textcat)
endif
endef

endif # SYSTEM_LIBTEXTCAT


ifeq ($(SYSTEM_MYTHES),YES)

define gb_LinkTarget__use_mythes
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(MYTHES_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MYTHES_LIBS))
endef

else # !SYSTEM_MYTHES

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,libmythes))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,mythes-1.2))
endif

define gb_LinkTarget__use_mythes
ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_linked_static_libs,$(1),libmythes)
else
$(call gb_LinkTarget_add_linked_static_libs,$(1),mythes-1.2)
endif
endef

endif # SYSTEM_MYTHES


ifeq ($(SYSTEM_HUNSPELL),YES)

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_set_include,$(1),\
        $$(INCLUDE) \
        $(HUNSPELL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(HUNSPELL_LIBS))
endef

else # !SYSTEM_HUNSPELL

ifeq ($(GUI),WNT)
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,libhunspell))
else ifeq ($(GUI),OS2)
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,hunspell))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,hunspell-1.3))
endif

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/hunspell \
)
$(call gb_LinkTarget_add_defs,$(1),\
    -DHUNSPELL_STATIC
)
ifeq ($(GUI),WNT)
$(call gb_LinkTarget_add_linked_static_libs,$(1),libhunspell)
else ifeq ($(GUI),OS2)
$(call gb_LinkTarget_add_linked_static_libs,$(1),hunspell)
else
$(call gb_LinkTarget_add_linked_static_libs,$(1),hunspell-1.3)
endif
endef

endif # SYSTEM_HUNSPELL

# vim: set noet sw=4 ts=4:
