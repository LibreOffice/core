#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# this is a subset of gbuild functionality, for use in custom makefiles

include $(GBUILDDIR)/Output.mk

# BuildDirs uses the Output functions already
include $(GBUILDDIR)/BuildDirs.mk

include $(GBUILDDIR)/Helper.mk

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else
ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
include $(GBUILDDIR)/platform/winmingw.mk
else
include $(GBUILDDIR)/platform/windows.mk
endif
else
ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else
ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
else
$(eval $(call gb_Output_error,Unsupported OS: $(OS)))
endif
endif
endif
endif

