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
#               , Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

MYDIR := $(realpath $(dir $(firstword $(MAKEFILE_LIST))))

all : services.rdb

services.rdb : $(foreach newcomponentfile,\
		lotuswordpro/util/lwpfilter,\
	$(OUTDIR)/xml/component/$(newcomponentfile).component \
)

services.rdb :
	echo '<?xml version="1.0"?><components xmlns="http://openoffice.org/2010/uno-components">' > $@
	$(gb_AWK) -- \
		'/^<\?xml version.*/ { next; } \
		{ gsub(/vnd.sun.star.expand:\$$OOO_BASE_DIR\/program/, "vnd.sun.star.expand:$$OOO_BASE_DIR",$$0); gsub(/vnd.sun.star.expand:\$$BRAND_BASE_DIR\/program/, "vnd.sun.star.expand:$$BRAND_BASE_DIR",$$0); print; }' \
		$^ >> $@
	echo '</components>' >> $@

.PHONY : all
# vim: set noet sw=4 ts=4:
