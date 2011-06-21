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

$(eval $(call gb_Library_Library,sdui))

$(eval $(call gb_Library_add_precompiled_header,sdui,$(SRCDIR)/sd/inc/pch/precompiled_sd))

$(eval $(call gb_Library_set_include,sdui,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/sd/inc) \
    -I$(realpath $(SRCDIR)/sd/inc/pch) \
    -I$(realpath $(SRCDIR)/sd/source/ui/inc) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_defs,sdui,\
    -DSD_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sdui,\
    basegfx \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    sal \
    sd \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sdui,\
    sd/source/filter/html/htmlattr \
    sd/source/ui/dlg/brkdlg \
    sd/source/ui/dlg/copydlg \
    sd/source/ui/dlg/masterlayoutdlg \
    sd/source/ui/dlg/morphdlg \
    sd/source/ui/dlg/paragr \
    sd/source/ui/dlg/present \
    sd/source/ui/dlg/prltempl \
    sd/source/ui/dlg/prntopts \
    sd/source/ui/dlg/sddlgfact \
    sd/source/ui/dlg/sdpreslt \
    sd/source/ui/dlg/sduiexp \
    sd/source/ui/dlg/tabtempl \
    sd/source/ui/dlg/tpaction \
    sd/source/ui/dlg/tpoption \
    sd/source/ui/dlg/vectdlg \
    sd/source/ui/dlg/custsdlg \
    sd/source/ui/dlg/dlgass \
    sd/source/ui/dlg/dlgassim \
    sd/source/ui/dlg/dlgchar \
    sd/source/ui/dlg/dlgfield \
    sd/source/ui/dlg/dlgolbul \
    sd/source/ui/dlg/dlgpage \
    sd/source/ui/dlg/dlgsnap \
    sd/source/ui/dlg/headerfooterdlg \
    sd/source/ui/dlg/inspagob \
    sd/source/filter/html/pubdlg \
))

# $(WORKDIR)/inc/sd/sddll0.hxx :
	# echo "#define" DLL_NAME \"$(if $(filter UNX,$(GUI)),lib)sd$(DLLPOSTFIX)$(DLLPOST)\" > $@

# vim: set noet ts=4 sw=4:
