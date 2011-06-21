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
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,forui))

$(eval $(call gb_Library_add_precompiled_header,forui,$(SRCDIR)/formula/inc/pch/precompiled_formula))

$(eval $(call gb_Library_set_include,forui,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/formula/inc) \
    -I$(realpath $(SRCDIR)/formula/inc/pch) \
    -I$(realpath $(SRCDIR)/formula/source/ui/inc) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_defs,forui,\
    -DFORMULA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,forui,\
    cppu \
    cppuhelper \
    for \
    sal \
    sfx \
    svl \
    svt \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,forui,\
    formula/source/ui/dlg/formula \
    formula/source/ui/dlg/FormulaHelper \
    formula/source/ui/dlg/funcpage \
    formula/source/ui/dlg/funcutl \
    formula/source/ui/dlg/parawin \
    formula/source/ui/dlg/structpg \
    formula/source/ui/resource/ModuleHelper \
))

# vim: set noet ts=4 sw=4:
