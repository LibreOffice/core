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
    compilerplugins/clang/test/buriedassign \
    compilerplugins/clang/test/casttovoid \
    compilerplugins/clang/test/collapseif \
    compilerplugins/clang/test/commaoperator \
    $(if $(filter-out WNT,$(OS)),compilerplugins/clang/test/constfields) \
    compilerplugins/clang/test/constparams \
    compilerplugins/clang/test/constvars \
    compilerplugins/clang/test/convertlong \
    compilerplugins/clang/test/cppunitassertequals \
    compilerplugins/clang/test/cstylecast \
    compilerplugins/clang/test/datamembershadow \
    compilerplugins/clang/test/dbgunhandledexception \
    compilerplugins/clang/test/dodgyswitch \
    compilerplugins/clang/test/doubleconvert \
    compilerplugins/clang/test/emptyif \
    compilerplugins/clang/test/externvar \
    compilerplugins/clang/test/expressionalwayszero \
    compilerplugins/clang/test/intvsfloat \
    compilerplugins/clang/test/faileddyncast \
    compilerplugins/clang/test/finalprotected \
    compilerplugins/clang/test/flatten \
    compilerplugins/clang/test/indentation \
    compilerplugins/clang/test/loopvartoosmall \
    compilerplugins/clang/test/oncevar \
    compilerplugins/clang/test/oslendian-1 \
    compilerplugins/clang/test/oslendian-2 \
    compilerplugins/clang/test/oslendian-3 \
    compilerplugins/clang/test/passparamsbyref \
    compilerplugins/clang/test/passstuffbyref \
    compilerplugins/clang/test/pointerbool \
    compilerplugins/clang/test/redundantcast \
    compilerplugins/clang/test/redundantfcast \
    compilerplugins/clang/test/redundantinline \
    compilerplugins/clang/test/redundantpointerops \
    compilerplugins/clang/test/refcounting \
    compilerplugins/clang/test/salbool \
    compilerplugins/clang/test/salcall \
    compilerplugins/clang/test/sallogareas \
    compilerplugins/clang/test/salunicodeliteral \
    compilerplugins/clang/test/shouldreturnbool \
    compilerplugins/clang/test/simplifybool \
    compilerplugins/clang/test/simplifyconstruct \
    compilerplugins/clang/test/simplifydynamiccast \
    compilerplugins/clang/test/singlevalfields \
    compilerplugins/clang/test/staticconstfield \
    compilerplugins/clang/test/staticvar \
    compilerplugins/clang/test/stringbuffer \
    compilerplugins/clang/test/stringconcat \
    compilerplugins/clang/test/stringconstant \
    compilerplugins/clang/test/stringloop \
    compilerplugins/clang/test/typedefparam \
    compilerplugins/clang/test/unnecessarycatchthrow \
    compilerplugins/clang/test/unnecessaryoverride \
    compilerplugins/clang/test/unnecessaryoverride-dtor \
    compilerplugins/clang/test/unnecessaryparen \
    compilerplugins/clang/test/unoany \
    compilerplugins/clang/test/unreffun \
    compilerplugins/clang/test/unusedindex \
    compilerplugins/clang/test/unusedenumconstants \
    compilerplugins/clang/test/unusedfields \
    compilerplugins/clang/test/unusedvariablecheck \
    compilerplugins/clang/test/unusedvariablemore \
    compilerplugins/clang/test/useuniqueptr \
    compilerplugins/clang/test/vclwidgets \
    compilerplugins/clang/test/writeonlyvars \
))

$(eval $(call gb_CompilerTest_use_externals,compilerplugins_clang, \
    boost_headers \
    cppunit \
))

$(eval $(call gb_CompilerTest_use_udk_api,compilerplugins_clang))

# vim: set noet sw=4 ts=4:
