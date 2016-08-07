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



$(eval $(call gb_Package_Package,oox_generated,$(WORKDIR)/CustomTarget/oox/source/token))

$(eval $(call \
	gb_Package_add_customtarget,oox_generated,oox/source/token,SRCDIR))

$(eval $(call \
	gb_CustomTarget_add_dependencies,oox/source/token,\
		oox/source/token/namespaces.hxx.head \
		oox/source/token/namespaces.hxx.tail \
		oox/source/token/namespaces.txt \
		oox/source/token/namespaces.pl \
		oox/source/token/tokens.hxx.head \
		oox/source/token/tokens.hxx.tail \
		oox/source/token/tokens.txt \
		oox/source/token/tokens.pl \
		oox/source/token/properties.hxx.head \
		oox/source/token/properties.hxx.tail \
		oox/source/token/properties.txt \
		oox/source/token/properties.pl \
))

$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/namespaces.hxx,namespaces.hxx))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/namespaces.txt,namespaces.txt))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/properties.hxx,properties.hxx))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/tokens.hxx,tokens.hxx))

