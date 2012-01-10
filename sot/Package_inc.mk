#**************************************************************
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
#**************************************************************



$(eval $(call gb_Package_Package,sot_inc,$(SRCDIR)/sot/inc))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/absdev.hxx,sot/absdev.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/agg.hxx,sot/agg.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/clsids.hxx,sot/clsids.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/exchange.hxx,sot/exchange.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/factory.hxx,sot/factory.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/filelist.hxx,sot/filelist.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/formats.hxx,sot/formats.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/object.hxx,sot/object.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdata.hxx,sot/sotdata.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdllapi.h,sot/sotdllapi.h))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotref.hxx,sot/sotref.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/stg.hxx,sot/stg.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storage.hxx,sot/storage.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storinfo.hxx,sot/storinfo.hxx))
