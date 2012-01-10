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



$(eval $(call gb_Package_Package,toolkit_source,$(SRCDIR)/toolkit/source))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/bin.hxx,layout/core/bin.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/box-base.hxx,layout/core/box-base.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/box.hxx,layout/core/box.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/container.hxx,layout/core/container.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/dialogbuttonhbox.hxx,layout/core/dialogbuttonhbox.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/factory.hxx,layout/core/factory.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/flow.hxx,layout/core/flow.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/helper.hxx,layout/core/helper.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/import.hxx,layout/core/import.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/localized-string.hxx,layout/core/localized-string.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/precompiled_xmlscript.hxx,layout/core/precompiled_xmlscript.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/proplist.hxx,layout/core/proplist.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/root.hxx,layout/core/root.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/table.hxx,layout/core/table.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/timer.hxx,layout/core/timer.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/translate.hxx,layout/core/translate.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/vcl.hxx,layout/core/vcl.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/vcl/wrapper.hxx,layout/vcl/wrapper.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/toolkit/awt/vclxdialog.hxx,awt/vclxdialog.hxx))
