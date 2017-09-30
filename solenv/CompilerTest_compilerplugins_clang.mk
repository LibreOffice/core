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
    compilerplugins/clang/test/badstatics \
    compilerplugins/clang/test/blockblock \
    compilerplugins/clang/test/casttovoid \
    compilerplugins/clang/test/constparams \
    $(if $(filter-out INTEL,$(CPU)),compilerplugins/clang/test/convertuintptr) \
    compilerplugins/clang/test/cppunitassertequals \
    compilerplugins/clang/test/datamembershadow \
    compilerplugins/clang/test/droplong \
    compilerplugins/clang/test/externvar \
    compilerplugins/clang/test/expressionalwayszero \
    compilerplugins/clang/test/finalprotected \
    compilerplugins/clang/test/flatten \
    compilerplugins/clang/test/loopvartoosmall \
    compilerplugins/clang/test/oncevar \
    compilerplugins/clang/test/oslendian-1 \
    compilerplugins/clang/test/oslendian-2 \
    compilerplugins/clang/test/oslendian-3 \
    compilerplugins/clang/test/passparamsbyref \
    compilerplugins/clang/test/passstuffbyref \
    compilerplugins/clang/test/redundantcast \
    compilerplugins/clang/test/redundantcopy \
    compilerplugins/clang/test/redundantinline \
    compilerplugins/clang/test/redundantpointerops \
    compilerplugins/clang/test/refcounting \
    compilerplugins/clang/test/salbool \
    compilerplugins/clang/test/salunicodeliteral \
    compilerplugins/clang/test/stringconstant \
    compilerplugins/clang/test/unnecessarycatchthrow \
    compilerplugins/clang/test/unnecessaryoverride-dtor \
    compilerplugins/clang/test/unnecessaryparen \
    compilerplugins/clang/test/unoany \
    compilerplugins/clang/test/useuniqueptr \
    compilerplugins/clang/test/vclwidgets \
))

# clang-3.8 and clang trunk differ in how they represent ranged-for
# which leads to a test failure
#    compilerplugins/clang/test/unusedfields \

$(eval $(call gb_CompilerTest_use_externals,compilerplugins_clang, \
    boost_headers \
    cppunit \
))

$(eval $(call gb_CompilerTest_use_udk_api,compilerplugins_clang))

# vim: set noet sw=4 ts=4:
