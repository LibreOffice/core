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

$(eval $(call gb_Module_Module,canvas))

$(eval $(call gb_Module_add_targets,canvas,\
	Library_canvastools \
	Library_nullcanvas \
	Library_simplecanvas \
	Library_vclcanvas \
	Library_canvasfactory \
	Package_inc \
))

ifeq ($(strip $(OS)),WNT)
ifneq ($(strip $(ENABLE_DIRECTX)),)

ifneq ($(strip $(USE_DIRECTX5)),)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_directx5canvas \
))
endif

$(eval $(call gb_Module_add_targets,canvas,\
	Library_directx9canvas \
	Library_gdipluscanvas \
))

endif
endif

ifneq ($(strip $(ENABLE_CAIRO_CANVAS)),)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_cairocanvas \
))
endif

# vim: set noet sw=4 ts=4:
