#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_AllLangResTarget_AllLangResTarget,spa))

$(eval $(call gb_AllLangResTarget_set_reslocation,spa,padmin))

$(eval $(call gb_AllLangResTarget_add_srs,spa,\
    padmin/source \
))

$(eval $(call gb_SrsTarget_SrsTarget,padmin/source))

$(eval $(call gb_SrsTarget_set_include,padmin/source,\
    $$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,padmin/source,\
    padmin/source/padialog.src \
    padmin/source/rtsetup.src \
))


# vim: set noet sw=4 ts=4:
