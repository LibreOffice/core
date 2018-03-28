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



$(eval $(call gb_Package_Package,cosv_inc,$(SRCDIR)/cosv/inc/cosv))

$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/file.hxx,file.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/bstream.hxx,bstream.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/ploc_dir.hxx,ploc_dir.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/openclose.hxx,openclose.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/comfunc.hxx,comfunc.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/datetime.hxx,datetime.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/mbstream.hxx,mbstream.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_precomp.h,csv_precomp.h))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_ostream.hxx,csv_ostream.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/streamstr.hxx,streamstr.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/x.hxx,x.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/stringdata.hxx,stringdata.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/commandline.hxx,commandline.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/str_types.hxx,str_types.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/plocroot.hxx,plocroot.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/persist.hxx,persist.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/ploc.hxx,ploc.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_env.hxx,csv_env.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/string.hxx,string.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/comdline.hxx,comdline.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/dirchain.hxx,dirchain.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/std_outp.hxx,std_outp.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/tpltools.hxx,tpl/tpltools.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/range.hxx,tpl/range.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/dyn.hxx,tpl/dyn.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/funcall.hxx,tpl/funcall.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/swelist.hxx,tpl/swelist.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/vvector.hxx,tpl/vvector.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/processor.hxx,tpl/processor.hxx))
