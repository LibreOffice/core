# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cppu))

$(eval $(call gb_Library_set_soversion_script,cppu,$(SRCDIR)/cppu/util/cppu.map))

$(eval $(call gb_Library_use_udk_api,cppu))

$(eval $(call gb_Library_add_defs,cppu,\
	-DCPPU_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,cppu,boost_headers))

$(eval $(call gb_Library_use_libraries,cppu,\
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_cobjects,cppu,\
	cppu/source/uno/EnvDcp \
))

$(eval $(call gb_Library_add_exception_objects,cppu,\
	cppu/source/cppu/cppu_opt \
	cppu/source/threadpool/current \
	cppu/source/threadpool/jobqueue \
	cppu/source/threadpool/thread \
	cppu/source/threadpool/threadident \
	cppu/source/threadpool/threadpool \
	cppu/source/typelib/static_types \
	cppu/source/typelib/typelib \
	cppu/source/uno/any \
	cppu/source/uno/cascade_mapping \
	cppu/source/uno/data \
	cppu/source/uno/EnvStack \
	cppu/source/uno/env_subst \
	cppu/source/uno/IdentityMapping \
	cppu/source/uno/lbenv \
	cppu/source/uno/lbmap \
	cppu/source/uno/loadmodule \
	cppu/source/uno/sequence \
))

# vim: set noet sw=4 ts=4:
