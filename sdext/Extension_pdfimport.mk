# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Extension_Extension,pdfimport,sdext/source/pdfimport))

$(eval $(call gb_Extension_add_file,pdfimport,components.rdb,$(call gb_Rdb_get_target,pdfimport)))

$(eval $(call gb_Extension_add_libraries,pdfimport,\
    pdfimport \
))

$(eval $(call gb_Extension_add_executables,pdfimport,\
    xpdfimport \
))

$(eval $(call gb_Extension_add_files,pdfimport,,\
    $(SRCDIR)/sdext/source/pdfimport/config/pdf_import_filter.xcu \
    $(SRCDIR)/sdext/source/pdfimport/config/pdf_types.xcu \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/xpdfimport_err.pdf \
))

$(eval $(call gb_Extension_add_files,pdfimport,basic,\
    $(SRCDIR)/sdext/source/pdfimport/dialogs/dialog.xlb \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/impress.png \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/Module1.xba \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/script.xlb \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/TargetChooser.xdl \
    $(SRCDIR)/sdext/source/pdfimport/dialogs/writer.png \
))

$(eval $(call gb_Extension_add_files,pdfimport,images,\
    $(SRCDIR)/icon-themes/galaxy/desktop/res/extension_32.png \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
