# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
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

$(eval $(call gb_Executable_Executable,renewpo))

$(eval $(call gb_Executable_set_include,renewpo,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,renewpo,\
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,renewpo,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,renewpo,\
    l10ntools/source/renewpo \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
