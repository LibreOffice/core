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

# Not built
$(eval $(call gb_Library_Library,rtftok))

$(eval $(call gb_Library_set_include,rtftok,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
))

include $(realpath $(SRCDIR)/writerfilter/debug_setup.mk)

$(eval $(call gb_Library_set_defs,rtftok,\
	$$(DEFS) \
	-DWRITERFILTER_RTFTOK_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
))

$(eval $(call gb_Library_add_linked_libs,rtftok,\
    cppu \
    cppuhelper \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,rtftok,\
))

# vim: set noet ts=4 sw=4:
