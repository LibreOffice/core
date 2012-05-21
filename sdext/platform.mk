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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

# TODO
# https://bugs.freedesktop.org/show_bug.cgi?id=50163
# move definition of PLATFORMID into configure.in
#

sdext__LOWERCASE_TABLE := A-a B-b C-c D-d E-e F-f G-g H-h I-i J-j K-k L-l M-m N-n O-o P-p R-r S-s T-t U-u V-v W-w X-x Y-y Z-z

define sdext__lcase_impl
$(if $(3),$(call sdext__lcase,$(firstword $(3)),$(3),$(subst $(1),$(2),$(4))),$(subst $(1),$(2),$(4)))
endef

define sdext__lcase
$(call sdext__lcase_impl,$(firstword  $(subst -, ,$(1))),$(lastword $(subst -, ,$(1))),$(wordlist 2,$(words $(2)),$(2)),$(3))
endef

define sdext__lowercase
$(call sdext__lcase,$(firstword $(sdext__LOWERCASE_TABLE)),$(sdext__LOWERCASE_TABLE),$(1))
endef

sdext_PLATFORM := $(call sdext__lowercase,$(RTL_OS)_$(RTL_ARCH))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
