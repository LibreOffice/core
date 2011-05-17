# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#        Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (Canonical Ltd.)
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s): Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (Canonical Ltd.)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,sc_qa_unit,$(WORKDIR)/CustomTarget/sc/qa/unit))
$(eval $(call gb_Package_add_customtarget,sc_qa_unit,sc/qa/unit))

# dependencies that cause the CustomTarget Makefile to be called recursively for
# (re)build
$(eval $(call gb_CustomTarget_add_outdir_dependencies,sc/qa/unit,\
	$(foreach newcomponentfile,\
		framework/util/fwk \
		sfx2/util/sfx \
		unoxml/source/service/unoxml,\
	$(OUTDIR)/xml/component/$(newcomponentfile).component) \
	$(foreach oldcomponentfile, \
		i18npool \
		ucb1 \
		ucpfile1, \
	$(OUTDIR)/xml/$(oldcomponentfile).component) \
))

# vim: set noet sw=4:
