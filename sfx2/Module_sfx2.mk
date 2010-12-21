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

$(eval $(call gb_Module_Module,sfx2))

$(eval $(call gb_Module_add_targets,sfx2,\
    AllLangResTarget_sfx2 \
    Library_sfx \
    Package_inc \
    Package_sdi \
))

ifeq ($(OS),LINUX)
ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Module_add_targets,sfx2,\
    Library_qstart \
))
endif
endif

#todo: map file?
#todo: source/appl ohne Optimierung?
#todo: source/control ohne Optimierung?
#todo: source/dialog BUILD_VER_STRING
#todo: source/doc SYSTEM_LIBXML2
#todo: noopt for acldetect.cxx?
#todo: ENABLE_LAYOUT
#todo: quickstarter
#todo: link against cocoa on Mac

# vim: set noet sw=4 ts=4:
