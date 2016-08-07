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



$(eval $(call gb_Package_Package,cppcanvas_inc,$(SRCDIR)/cppcanvas/inc))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/canvas.hxx,cppcanvas/canvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/sprite.hxx,cppcanvas/sprite.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/vclfactory.hxx,cppcanvas/vclfactory.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/font.hxx,cppcanvas/font.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/customsprite.hxx,cppcanvas/customsprite.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/canvasgraphic.hxx,cppcanvas/canvasgraphic.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/polypolygon.hxx,cppcanvas/polypolygon.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/color.hxx,cppcanvas/color.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/spritecanvas.hxx,cppcanvas/spritecanvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/text.hxx,cppcanvas/text.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/renderer.hxx,cppcanvas/renderer.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/basegfxfactory.hxx,cppcanvas/basegfxfactory.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/bitmap.hxx,cppcanvas/bitmap.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/bitmapcanvas.hxx,cppcanvas/bitmapcanvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/cppcanvasdllapi.h,cppcanvas/cppcanvasdllapi.h))

# vim: set noet sw=4 ts=4:
