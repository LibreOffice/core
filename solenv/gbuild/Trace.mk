# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Support for tracing what gbuild does.
# Run 'make GBUILD_TRACE=/somewhere/log.json', process the file
# using solenv/bin/finish-gbuild-trace.py, and then view it in Chromium
# using the chrome://tracing URL:
# - the '?' icon in the top-right is the help
# - 'gbuild' rows represent per-parallelism (per-CPU) usage in time
# - 'totals' rows represent sums for reach build type
# - note that 'EXTERNAL' targets do not detect whether the external package
#   is built with parallelism or not, so the actual CPU time may be higher
# - vertical lines represent targets that themselves do not take any time
#   to build
# - any target can be found using the search field in the top right corner
#   (just type e.g. '[MOD]: sal' and hit the '->' button, if you cannot see
#   it pressing 'm' or 'f' can help)

gb_TRACE :=
ifneq ($(GBUILD_TRACE),)
gb_TRACE := $(abspath $(GBUILD_TRACE))
endif

ifneq ($(gb_TRACE),)

# call gb_Trace_AddMark,marktype,detail,type,extra
# The (flock;cat) part is to minimize lock time.
define gb_Trace__AddMark
echo "{\"name\": \"$(3)\", \"ph\": \"$(1)\", \"pid\": 1, \"tid\": 1, \"ts\": $$(date +%s%N),\"args\":{\"message\":\"[$(3)]: $(2)\"}}," | ( flock 1; cat ) >>$(gb_TRACE)
endef

# call gb_Trace_StartRange,detail,type
define gb_Trace_StartRange
$(call gb_Trace__AddMark,B,$(1),$(2))
endef

# call gb_Trace_EndRange,detail,type
define gb_Trace_EndRange
$(call gb_Trace__AddMark,E,$(1),$(2))
endef

# call gb_Trace_MakeMark,detail,type
define gb_Trace_MakeMark
$(call gb_Trace__AddMark,i,$(1),$(2))
endef

ifeq ($(MAKE_RESTARTS),)
$(shell rm -f $(gb_TRACE) 2>/dev/null)
else
$(shell $(call gb_Trace__AddMark,i,make restart,MAKE))
endif

endif

# vim: set noet sw=4 ts=4:
