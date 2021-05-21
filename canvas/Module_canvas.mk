# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Module_Module,canvas))

$(eval $(call gb_Module_add_targets,canvas,\
	Library_canvasfactory \
	Library_canvastools \
	Library_simplecanvas \
	Library_vclcanvas \
    $(if $(filter DESKTOP,$(BUILD_TYPE)), \
        $(if $(or $(DISABLE_GUI),$(DISABLE_DYNLOADING)),,Executable_canvasdemo)) \
))

ifeq ($(ENABLE_CAIRO_CANVAS),TRUE)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_cairocanvas \
))
endif

ifeq ($(ENABLE_OPENGL_CANVAS),TRUE)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_oglcanvas \
	Package_opengl \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_directx9canvas \
	Library_gdipluscanvas \
	StaticLibrary_directxcanvas \
))
endif

$(eval $(call gb_Module_add_check_targets,canvas,\
    CppunitTest_canvas_test \
))

# vim: set noet sw=4 ts=4:
