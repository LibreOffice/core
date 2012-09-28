# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
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

$(eval $(call gb_Library_Library,cppu))

$(eval $(call gb_Library_set_soversion_script,cppu,3,$(SRCDIR)/cppu/util/cppu.map))

$(eval $(call gb_Library_use_package,cppu,cppu_inc))

$(eval $(call gb_Library_use_sdk_api,cppu))

$(eval $(call gb_Library_add_defs,cppu,\
	-DCPPU_DLLIMPLEMENTATION \
))

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
