# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# OpenOffice.org - a multi-platform office productivity suite
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

$(eval $(call gb_Module_Module,unotools))

$(eval $(call gb_Module_add_targets,unotools,\
    Library_utl \
    Package_inc \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,unotools,\
    JunitTest_unotools_complex \
))

# vim: set noet ts=4 sw=4:
