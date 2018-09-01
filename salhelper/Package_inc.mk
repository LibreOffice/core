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



$(eval $(call gb_Package_Package,salhelper_inc,$(SRCDIR)/salhelper/inc))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/condition.hxx,salhelper/condition.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/dynload.hxx,salhelper/dynload.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/future.hxx,salhelper/future.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/futurequeue.hxx,salhelper/futurequeue.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/monitor.hxx,salhelper/monitor.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/queue.hxx,salhelper/queue.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/refobj.hxx,salhelper/refobj.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/salhelperdllapi.h,salhelper/salhelperdllapi.h))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/simplereferenceobject.hxx,salhelper/simplereferenceobject.hxx))
$(eval $(call gb_Package_add_file,salhelper_inc,inc/salhelper/singletonref.hxx,salhelper/singletonref.hxx))
