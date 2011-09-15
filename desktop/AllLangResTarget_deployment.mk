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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,deployment))

$(eval $(call gb_AllLangResTarget_set_reslocation,deployment,desktop))

$(eval $(call gb_AllLangResTarget_add_srs,deployment,\
    deployment/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,deployment/res))

$(eval $(call gb_SrsTarget_set_include,deployment/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/registry/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,deployment/res,\
    desktop/source/deployment/manager/dp_manager.src \
    desktop/source/deployment/misc/dp_misc.src \
    desktop/source/deployment/registry/component/dp_component.src \
    desktop/source/deployment/registry/configuration/dp_configuration.src \
    desktop/source/deployment/registry/dp_registry.src \
    desktop/source/deployment/registry/help/dp_help.src \
    desktop/source/deployment/registry/package/dp_package.src \
    desktop/source/deployment/registry/script/dp_script.src \
    desktop/source/deployment/registry/sfwk/dp_sfwk.src \
    desktop/source/deployment/unopkg/unopkg.src \
))

# vim: set ts=4 sw=4 et:
