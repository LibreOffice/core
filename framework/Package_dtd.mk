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



$(eval $(call gb_Package_Package,framework_dtd,$(SRCDIR)/framework/dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/accelerator.dtd,accelerator.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/event.dtd,event.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/groupuinames.dtd,groupuinames.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/image.dtd,image.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/menubar.dtd,menubar.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/statusbar.dtd,statusbar.dtd))
$(eval $(call gb_Package_add_file,framework_dtd,bin/toolbar.dtd,toolbar.dtd))
