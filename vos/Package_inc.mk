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



$(eval $(call gb_Package_Package,vos_inc,$(SRCDIR)/vos/inc/vos))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/conditn.hxx,conditn.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/connectn.hxx,connectn.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/diagnose.hxx,diagnose.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/execabl.hxx,execabl.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/istream.hxx,istream.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/macros.hxx,macros.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/module.hxx,module.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/mutex.hxx,mutex.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/object.hxx,object.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/pipe.hxx,pipe.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/process.hxx,process.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/ref.hxx,ref.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/ref.inl,ref.inl))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/refernce.hxx,refernce.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/refobj.hxx,refobj.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/refobj.inl,refobj.inl))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/runnable.hxx,runnable.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/security.hxx,security.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/signal.hxx,signal.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/socket.hxx,socket.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/stream.hxx,stream.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/thread.hxx,thread.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/timer.hxx,timer.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/types.hxx,types.hxx))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/vosdllapi.h,vosdllapi.h))
$(eval $(call gb_Package_add_file,vos_inc,inc/vos/xception.hxx,xception.hxx))

