# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,cppumaker))

$(eval $(call gb_Executable_use_libraries,cppumaker,\
    reg \
    sal \
    salhelper \
))

$(eval $(call gb_Executable_use_static_libraries,cppumaker,\
    codemaker_cpp \
    codemaker \
))

$(eval $(call gb_Executable_add_exception_objects,cppumaker,\
    codemaker/source/cppumaker/cppumaker \
    codemaker/source/cppumaker/cppuoptions \
    codemaker/source/cppumaker/cpputype \
    codemaker/source/cppumaker/dumputils \
    codemaker/source/cppumaker/includes \
))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Executable_add_cxxflags,cppumaker,\
	-Ob0 \
))
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
