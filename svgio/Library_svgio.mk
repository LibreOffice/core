#/**************************************************************
# * 
# * Licensed to the Apache Software Foundation (ASF) under one
# * or more contributor license agreements.  See the NOTICE file
# * distributed with this work for additional information
# * regarding copyright ownership.  The ASF licenses this file
# * to you under the Apache License, Version 2.0 (the
# * "License"); you may not use this file except in compliance
# * with the License.  You may obtain a copy of the License at
# * 
# *   http://www.apache.org/licenses/LICENSE-2.0
# * 
# * Unless required by applicable law or agreed to in writing,
# * software distributed under the License is distributed on an
# * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# * KIND, either express or implied.  See the License for the
# * specific language governing permissions and limitations
# * under the License.
# * 
# *************************************************************/

$(eval $(call gb_Library_Library,svgio))

$(eval $(call gb_Library_set_componentfile,svgio,svgio/svgio))

$(eval $(call gb_Library_add_package_headers,svgio,svgio_inc))

$(eval $(call gb_Library_add_precompiled_header,svgio,$(SRCDIR)/svgio/inc/pch/precompiled_svgio))

$(eval $(call gb_Library_set_include,svgio,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svgio/inc \
    -I$(SRCDIR)/svgio/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,svgio,\
    $$(DEFS) \
    -DSVGIO_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,svgio,\
    basegfx \
    drawinglayer \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    stl \
    tl \
    sax \
    vcl \
    svt \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,svgio,\
    svgio/source/svgreader/svgcharacternode \
    svgio/source/svgreader/svgcirclenode \
    svgio/source/svgreader/svgclippathnode \
    svgio/source/svgreader/svgdocument \
    svgio/source/svgreader/svgdocumenthandler \
    svgio/source/svgreader/svgellipsenode \
    svgio/source/svgreader/svggnode \
    svgio/source/svgreader/svggradientnode \
    svgio/source/svgreader/svggradientstopnode \
    svgio/source/svgreader/svgimagenode \
    svgio/source/svgreader/svglinenode \
    svgio/source/svgreader/svgmarkernode \
    svgio/source/svgreader/svgmasknode \
    svgio/source/svgreader/svgnode \
    svgio/source/svgreader/svgpaint \
    svgio/source/svgreader/svgpathnode \
    svgio/source/svgreader/svgpatternnode \
    svgio/source/svgreader/svgpolynode \
    svgio/source/svgreader/svgrectnode \
    svgio/source/svgreader/svgstyleattributes \
    svgio/source/svgreader/svgstylenode \
    svgio/source/svgreader/svgsvgnode \
    svgio/source/svgreader/svgsymbolnode \
    svgio/source/svgreader/svgtextnode \
    svgio/source/svgreader/svgtitledescnode \
    svgio/source/svgreader/svgtoken \
    svgio/source/svgreader/svgtrefnode \
    svgio/source/svgreader/svgtools \
    svgio/source/svgreader/svgtextpathnode \
    svgio/source/svgreader/svgtspannode \
    svgio/source/svgreader/svgusenode \
    svgio/source/svguno/svguno \
    svgio/source/svguno/xsvgparser \
))

# vim: set noet sw=4 ts=4:
