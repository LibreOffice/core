# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.


$(eval $(call gb_AllLangResTarget_AllLangResTarget,sm))

$(eval $(call gb_AllLangResTarget_set_reslocation,sm,starmath))

$(eval $(call gb_AllLangResTarget_add_srs,sm,\
	sm/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sm/res))

$(eval $(call gb_SrsTarget_set_include,sm/res,\
	-I$(SRCDIR)/starmath/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_use_packages,sm/res,\
	svx_globlmn_hrc \
))

$(eval $(call gb_SrsTarget_add_files,sm/res,\
    starmath/source/smres.src \
    starmath/source/commands.src \
    starmath/source/symbol.src \
	starmath/source/toolbox.src \
))

# vim: set noet sw=4 ts=4:
