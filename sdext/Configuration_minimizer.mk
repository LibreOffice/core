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

$(eval $(call gb_Configuration_Configuration,presentation-minimizer,nodeliver))

$(eval $(call gb_Configuration_add_schemas,presentation-minimizer,sdext/source/minimizer/registry/schema,\
    org/openoffice/Office/extension/SunPresentationMinimizer.xcs \
))

$(eval $(call gb_Configuration_add_datas,presentation-minimizer,sdext/source/minimizer/registry/data,\
    org/openoffice/Office/ProtocolHandler.xcu \
))

$(eval $(call gb_Configuration_add_localized_datas,presentation-minimizer,sdext/source/minimizer/registry/data,\
    org/openoffice/Office/Addons.xcu \
    org/openoffice/Office/extension/SunPresentationMinimizer.xcu \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
