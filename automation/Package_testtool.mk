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

$(eval $(call gb_Package_Package,automation_testtool,$(WORKDIR)/CustomTarget/automation/source/testtool))

$(eval $(call gb_Package_add_file,automation_testtool,bin/classes,classes))
$(eval $(call gb_Package_add_file,automation_testtool,bin/keycodes,keycodes))
$(eval $(call gb_Package_add_file,automation_testtool,bin/res_type,res_type))

ifeq ($(GUI),UNX)

$(eval $(call gb_Package_add_file,automation_testtool,bin/testtoolrc,testtool.ini))

else ifeq ($(GUI),WNT)

$(eval $(call gb_Package_add_file,automation_testtool,bin/testtool.ini,testtool.ini))

endif

$(eval $(call gb_Package_add_customtarget,automation_testtool,automation/source/testtool,SRCDIR))

$(eval $(call gb_CustomTarget_add_dependencies,automation/source/testtool,\
    automation/source/inc/rcontrol.hxx \
    automation/source/testtool/filter.pl \
    automation/source/testtool/testtool.ini \
))

$(eval $(call gb_CustomTarget_add_outdir_dependencies,automation/source/testtool,\
    $(OUTDIR)/inc/tools/wintypes.hxx \
    $(OUTDIR)/inc/vcl/keycodes.hxx \
))

# vim: set ts=4 sw=4 et:
