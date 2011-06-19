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

# if no environment is set, try to find one, source it and re-exec
ifeq ($(strip $(SOLARENV)),)

all:
	if test -f ./source_soenv.sh; then . ./source_soenv.sh; fi && \
	if test -z "$${SOLARENV}"; then echo "No environment set!"; exit 1; fi && \
	$(MAKE)

else # SOLARENV

GBUILDDIR := $(SOLARENV)/gbuild
include $(GBUILDDIR)/gbuild.mk

$(eval $(call gb_Module_make_global_targets,$(SRC_ROOT)/Module_tail_build.mk))

endif # SOLARENV

# vim: set noet sw=4 ts=4:
