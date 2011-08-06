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
# Contributor(s): Jan Holesovsky <kendy@suse.cz>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

all : afm_hash.hpp

afm_hash.hpp : $(realpath $(dir $(firstword $(MAKEFILE_LIST))))/afm_keyword_list
	$(GPERF) -C -t -l -L C++ -m 20 -Z AfmKeywordHash -k '1,4,6,$$' $< | sed -e "s/(char\*)0/(char\*)0, NOPE/g" | grep -v "^#line" > $@

.PHONY : all
# vim: set noet sw=4:
