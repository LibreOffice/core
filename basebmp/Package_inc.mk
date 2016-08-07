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



$(eval $(call gb_Package_Package,basebmp_inc,$(SRCDIR)/basebmp/inc))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessor.hxx,basebmp/accessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessoradapters.hxx,basebmp/accessoradapters.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessorfunctors.hxx,basebmp/accessorfunctors.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessortraits.hxx,basebmp/accessortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/bitmapdevice.hxx,basebmp/bitmapdevice.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/clippedlinerenderer.hxx,basebmp/clippedlinerenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/color.hxx,basebmp/color.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colorblendaccessoradapter.hxx,basebmp/colorblendaccessoradapter.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colormisc.hxx,basebmp/colormisc.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colortraits.hxx,basebmp/colortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/compositeiterator.hxx,basebmp/compositeiterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/debug.hxx,basebmp/debug.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/drawmodes.hxx,basebmp/drawmodes.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/endian.hxx,basebmp/endian.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/fillimage.hxx,basebmp/fillimage.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/genericcolorimageaccessor.hxx,basebmp/genericcolorimageaccessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/greylevelformats.hxx,basebmp/greylevelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/iteratortraits.hxx,basebmp/iteratortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/linerenderer.hxx,basebmp/linerenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/metafunctions.hxx,basebmp/metafunctions.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/nonstandarditerator.hxx,basebmp/nonstandarditerator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/packedpixeliterator.hxx,basebmp/packedpixeliterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/paletteformats.hxx,basebmp/paletteformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/paletteimageaccessor.hxx,basebmp/paletteimageaccessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/pixelformatadapters.hxx,basebmp/pixelformatadapters.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/pixeliterator.hxx,basebmp/pixeliterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/polypolygonrenderer.hxx,basebmp/polypolygonrenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/rgb24pixelformats.hxx,basebmp/rgb24pixelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/rgbmaskpixelformats.hxx,basebmp/rgbmaskpixelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/scaleimage.hxx,basebmp/scaleimage.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/scanlineformats.hxx,basebmp/scanlineformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/stridedarrayiterator.hxx,basebmp/stridedarrayiterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/tools.hxx,basebmp/tools.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/truecolormaskaccessor.hxx,basebmp/truecolormaskaccessor.hxx))

# vim: set noet sw=4 ts=4:
