# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Package_Package,ucb_xml,$(SRCDIR)/ucb/source))

$(eval $(call gb_Package_add_file,ucb_xml,xml/cached.xml,cacher/cached.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucb.xml,core/ucb.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/srtrs.xml,sorter/srtrs.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpfile.xml,ucp/file/ucpfile.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpftp.xml,ucp/ftp/ucpftp.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpgvfs.xml,ucp/gvfs/ucpgvfs.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucphier.xml,ucp/hierarchy/ucphier.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucppkg.xml,ucp/package/ucppkg.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucptdoc.xml,ucp/tdoc/ucptdoc.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpdav.xml,ucp/webdav/ucpdav.xml))

# These were not delivered:
# $(eval $(call gb_Package_add_file,ucb_xml,xml/ucpcmis.xml,ucp/cmis/ucpcmis.xml))
# $(eval $(call gb_Package_add_file,ucb_xml,xml/ucpgio.xml,ucp/gio/ucpgio.xml))
# $(eval $(call gb_Package_add_file,ucb_xml,xml/ucpodma.xml,ucp/odma/ucpodma.xml))

# vim: set noet sw=4 ts=4:
