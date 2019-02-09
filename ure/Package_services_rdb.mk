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

$(eval $(call gb_Package_Package,ure_services_rdb,$(WORKDIR)/CustomTarget/ure/source))

$(eval $(call gb_Package_add_file,ure_services_rdb,xml/ure/services.rdb,services.rdb))

$(WORKDIR)/CustomTarget/ure/source/services.rdb : \
		$(SRCDIR)/ure/source/services.input \
		$(SOLARENV)/bin/packcomponents.xslt
	mkdir -p $(dir $@) && \
	$(gb_XSLTPROC) \
		--nonet \
		--stringparam prefix $(call gb_Helper_convert_native,$(OUTDIR)/xml/) \
		-o $(call gb_Helper_convert_native,$@) \
		$(call gb_Helper_convert_native,$(SOLARENV)/bin/packcomponents.xslt) \
		$(call gb_Helper_convert_native,$(SRCDIR)/ure/source/services.input)
