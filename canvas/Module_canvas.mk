###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



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

ifeq ($(strip $(ENABLE_CAIRO_CANVAS)),TRUE)
$(eval $(call gb_Module_add_targets,canvas,\
	Library_cairocanvas \
))
endif

# vim: set noet sw=4 ts=4:
