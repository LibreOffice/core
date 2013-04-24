# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,uno-skeletonmaker))

$(eval $(call gb_Executable_use_external,uno-skeletonmaker,boost_headers))

$(eval $(call gb_Executable_set_include,uno-skeletonmaker,\
    -I$(SRCDIR)/unodevtools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,uno-skeletonmaker,\
    sal \
    salhelper \
    unoidl \
))

$(eval $(call gb_Executable_use_static_libraries,uno-skeletonmaker,\
    codemaker \
    codemaker_cpp \
    codemaker_java \
))

$(eval $(call gb_Executable_add_exception_objects,uno-skeletonmaker,\
    unodevtools/source/unodevtools/options \
    unodevtools/source/skeletonmaker/skeletonmaker \
    unodevtools/source/skeletonmaker/skeletoncommon \
    unodevtools/source/skeletonmaker/javatypemaker \
    unodevtools/source/skeletonmaker/cpptypemaker \
    unodevtools/source/skeletonmaker/javacompskeleton \
    unodevtools/source/skeletonmaker/cppcompskeleton \
))

# vim:set noet sw=4 ts=4:
