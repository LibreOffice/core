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



$(eval $(call gb_Package_Package,canvas_inc,$(SRCDIR)/canvas/inc))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/basemutexhelper.hxx,canvas/base/basemutexhelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/bitmapcanvasbase.hxx,canvas/base/bitmapcanvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/bufferedgraphicdevicebase.hxx,canvas/base/bufferedgraphicdevicebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/cachedprimitivebase.hxx,canvas/base/cachedprimitivebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvasbase.hxx,canvas/base/canvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvascustomspritebase.hxx,canvas/base/canvascustomspritebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvascustomspritehelper.hxx,canvas/base/canvascustomspritehelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/doublebitmapbase.hxx,canvas/base/doublebitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/floatbitmapbase.hxx,canvas/base/floatbitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/graphicdevicebase.hxx,canvas/base/graphicdevicebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/integerbitmapbase.hxx,canvas/base/integerbitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/sprite.hxx,canvas/base/sprite.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/spritecanvasbase.hxx,canvas/base/spritecanvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/spritesurface.hxx,canvas/base/spritesurface.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/canvastools.hxx,canvas/canvastools.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/canvastoolsdllapi.h,canvas/canvastoolsdllapi.h))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/debug.hxx,canvas/debug.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/elapsedtime.hxx,canvas/elapsedtime.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/parametricpolypolygon.hxx,canvas/parametricpolypolygon.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/prioritybooster.hxx,canvas/prioritybooster.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/propertysethelper.hxx,canvas/propertysethelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/bitmap.hxx,canvas/rendering/bitmap.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/icachedprimitive.hxx,canvas/rendering/icachedprimitive.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/icolorbuffer.hxx,canvas/rendering/icolorbuffer.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/irendermodule.hxx,canvas/rendering/irendermodule.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurface.hxx,canvas/rendering/isurface.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurfaceproxy.hxx,canvas/rendering/isurfaceproxy.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurfaceproxymanager.hxx,canvas/rendering/isurfaceproxymanager.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/spriteredrawmanager.hxx,canvas/spriteredrawmanager.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/vclwrapper.hxx,canvas/vclwrapper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/verbosetrace.hxx,canvas/verbosetrace.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/verifyinput.hxx,canvas/verifyinput.hxx))


# vim: set noet sw=4 ts=4:
