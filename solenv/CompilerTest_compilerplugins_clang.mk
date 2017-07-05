# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CompilerTest_CompilerTest,compilerplugins_clang))

$(eval $(call gb_CompilerTest_add_exception_objects,compilerplugins_clang, \
    compilerplugins/clang/test/casttovoid \
    compilerplugins/clang/test/cppunitassertequals \
    compilerplugins/clang/test/datamembershadow \
    compilerplugins/clang/test/externvar \
    compilerplugins/clang/test/finalprotected \
    compilerplugins/clang/test/loopvartoosmall \
    compilerplugins/clang/test/oncevar \
    compilerplugins/clang/test/oslendian-1 \
    compilerplugins/clang/test/oslendian-2 \
    compilerplugins/clang/test/oslendian-3 \
    compilerplugins/clang/test/passstuffbyref \
    compilerplugins/clang/test/redundantcast \
    compilerplugins/clang/test/redundantcopy \
    compilerplugins/clang/test/redundantinline \
    compilerplugins/clang/test/refcounting \
    compilerplugins/clang/test/salbool \
    compilerplugins/clang/test/salunicodeliteral \
    compilerplugins/clang/test/stringconstant \
    compilerplugins/clang/test/unnecessaryoverride-dtor \
    compilerplugins/clang/test/unnecessaryparen \
    compilerplugins/clang/test/unoany \
    compilerplugins/clang/test/unusedfields \
    compilerplugins/clang/test/useuniqueptr \
    compilerplugins/clang/test/vclwidgets \
))

$(eval $(call gb_CompilerTest_use_externals,compilerplugins_clang, \
    boost_headers \
    cppunit \
))

$(eval $(call gb_CompilerTest_use_udk_api,compilerplugins_clang))

# vim: set noet sw=4 ts=4:
