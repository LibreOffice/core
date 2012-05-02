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

$(eval $(call gb_Jar_Jar,FCFGMerge))

$(eval $(call gb_Jar_set_packageroot,FCFGMerge,com))

$(eval $(call gb_Jar_set_manifest,FCFGMerge,$(SRCDIR)/l10ntools/source/filter/merge/Manifest.mf))

$(eval $(call gb_Jar_add_sourcefiles,FCFGMerge,\
	l10ntools/source/filter/merge/FCFGMerge \
	l10ntools/source/filter/merge/Merger \
	l10ntools/source/filter/utils/AnalyzeStartupLog \
	l10ntools/source/filter/utils/Cache \
	l10ntools/source/filter/utils/ConfigHelper \
	l10ntools/source/filter/utils/FileHelper \
	l10ntools/source/filter/utils/Logger \
	l10ntools/source/filter/utils/MalformedCommandLineException \
	l10ntools/source/filter/utils/XMLHelper \
))

$(eval $(call gb_Jar_add_packagefile,FCFGMerge,\
	com/sun/star/filter/config/tools/merge/FCFGMerge.cfg,\
	$(SRCDIR)/l10ntools/source/filter/merge/FCFGMerge.cfg \
))
