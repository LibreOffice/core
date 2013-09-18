# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cppu))

$(eval $(call gb_Module_add_targets,cppu,\
	InternalUnoApi_cppu_qa_cppumaker_types \
	InternalUnoApi_cppu_qa_types \
	Library_affine_uno \
	Library_cppu \
	Library_log_uno \
	Library_purpenvhelper \
	Library_unsafe_uno \
))

$(eval $(call gb_Module_add_check_targets,cppu,\
	CppunitTest_cppu_qa_any \
	CppunitTest_cppu_qa_recursion \
	CppunitTest_cppu_qa_reference \
	CppunitTest_cppu_qa_unotype \
	CppunitTest_cppu_test_cppumaker \
))

# vim: set noet sw=4 ts=4:
