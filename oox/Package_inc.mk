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



$(eval $(call gb_Package_Package,oox_inc,$(SRCDIR)/oox/inc))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/dllapi.h,oox/dllapi.h))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterbase.hxx,oox/core/filterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterdetect.hxx,oox/core/filterdetect.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/relations.hxx,oox/core/relations.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/xmlfilterbase.hxx,oox/core/xmlfilterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/chartconverter.hxx,oox/drawingml/chart/chartconverter.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/table/tablestylelist.hxx,oox/drawingml/table/tablestylelist.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binarystreambase.hxx,oox/helper/binarystreambase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/helper.hxx,oox/helper/helper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refmap.hxx,oox/helper/refmap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refvector.hxx,oox/helper/refvector.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/storagebase.hxx,oox/helper/storagebase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/zipstorage.hxx,oox/helper/zipstorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/vbaproject.hxx,oox/ole/vbaproject.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmldrawing.hxx,oox/vml/vmldrawing.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlshape.hxx,oox/vml/vmlshape.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/xls/excelvbaproject.hxx,oox/xls/excelvbaproject.hxx))
