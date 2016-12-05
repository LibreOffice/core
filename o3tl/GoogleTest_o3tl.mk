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


$(eval $(call gb_GoogleTest_GoogleTest,o3tl_test))

$(eval $(call gb_GoogleTest_add_exception_objects,o3tl_test, \
	o3tl/qa/cow_wrapper_clients \
	o3tl/qa/main \
	o3tl/qa/test-cow_wrapper \
	o3tl/qa/test-vector_pool \
	o3tl/qa/test-heap_ptr \
	o3tl/qa/test-range \
))

$(eval $(call gb_GoogleTest_add_linked_libs,o3tl_test, \
	sal \
	$(gb_STDLIBS) \
))

# vim: set noet sw=4 ts=4:
