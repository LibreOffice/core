##
## Version: MPL 1.1 / GPLv3+ / LGPLv3+
##
## The contents of this file are subject to the Mozilla Public License Version
## 1.1 (the "License"); you may not use this file except in compliance with
## the License or as specified alternatively below. You may obtain a copy of
## the License at http://www.mozilla.org/MPL/
##
## Software distributed under the License is distributed on an "AS IS" basis,
## WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
## for the specific language governing rights and limitations under the
## License.
##
## Major Contributor(s):
## [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
##   (initial developer) ]
##
## All Rights Reserved.
##
## For minor contributions see the git repository.
##
## Alternatively, the contents of this file may be used under the terms of
## either the GNU General Public License Version 3 or later (the "GPLv3+"), or
## the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
## in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
## instead of those above.
##

$(eval $(call gb_StaticLibrary_StaticLibrary,headless))

$(eval $(call gb_StaticLibrary_use_packages,headless,\
	basebmp_inc \
	basegfx_inc \
	comphelper_inc \
	cppu_inc \
	cppuhelper_inc \
	i18npool_inc \
	i18nutil_inc \
	o3tl_inc \
	tools_inc \
	unotools_inc \
))

$(eval $(call gb_StaticLibrary_use_api,headless,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,headless,\
    $$(FREETYPE_CFLAGS) \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,headless,\
    vcl/headless/svpbmp \
    vcl/headless/svpdummies \
    vcl/headless/svpelement \
    vcl/headless/svpframe \
    vcl/headless/svpprn \
    vcl/headless/svptext \
    vcl/headless/svpvd \
))

$(eval $(call gb_StaticLibrary_set_include,headless,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))
