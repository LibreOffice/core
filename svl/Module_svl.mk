# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Module_Module,svl))

$(eval $(call gb_Module_add_targets,svl,\
    Library_fsstorage \
    Library_passwordcontainer \
    Library_svl \
))

$(eval $(call gb_Module_add_l10n_targets,svl,\
    AllLangResTarget_svl \
))

$(eval $(call gb_Module_add_check_targets,svl,\
	CppunitTest_svl_lngmisc \
	CppunitTest_svl_qa_cppunit \
	CppunitTest_svl_inetcontenttype \
	CppunitTest_svl_items \
	CppunitTest_svl_itempool \
	CppunitTest_svl_notify \
))

#FIXME: fails on MSVC
ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_check_targets,svl,\
	CppunitTest_svl_urihelper \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,svl,\
    JunitTest_svl_complex \
))
endif

#todo: dde platform dependent
#todo: package_inc
#todo: map file

# vim: set noet sw=4 ts=4:
