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

$(eval $(call gb_Package_Package,automation_inc,$(SRCDIR)/automation/inc/automation))

$(eval $(call gb_Package_add_file,automation_inc,inc/automation/automation.hxx,automation.hxx))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/commdefines.hxx,commdefines.hxx))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/commtypes.hxx,commtypes.hxx))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/communi.hxx,communi.hxx))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/communidllapi.h,communidllapi.h))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/simplecm.hxx,simplecm.hxx))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/simplecmdllapi.h,simplecmdllapi.h))
$(eval $(call gb_Package_add_file,automation_inc,inc/automation/stsdllapi.h,stsdllapi.h))

# vim: set ts=4 sw=4 et:
