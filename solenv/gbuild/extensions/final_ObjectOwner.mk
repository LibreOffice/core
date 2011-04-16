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
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
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

ifeq ($(gb_CHECKOBJECTOWNER),$(true))

define gb_LinkTarget_check_object_owner
ifneq (1,$$(words $$(filter $(1):%,$$(gb_LinkTarget_OBJECTOWNER))))
$$(info \
	$(patsubst $(WORKDIR)/%,%,$(1))\
	is linked in by\
	$$(patsubst $(1):%,%,$$(filter $(1):%,$$(gb_LinkTarget_OBJECTOWNER))))
gb_CHECKOBJECTOWNER_VALID := $(false)
endif
endef

gb_CHECKOBJECTOWNER_VALID := $(true)
$(foreach object,$(sort $(gb_LinkTarget_OBJECTS)),$(eval $(call gb_LinkTarget_check_object_owner,$(object))))
ifneq ($(gb_CHECKOBJECTOWNER_VALID),$(true))
$(eval $(call gb_Output_error,duplicate linked objects))
endif

endif
# vim: set noet ts=4 sw=4:
