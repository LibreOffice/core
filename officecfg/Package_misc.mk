#***************************************************************
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#***************************************************************

$(eval $(call gb_Package_Package,officecfg_misc,$(SRCDIR)/officecfg))

$(eval $(call gb_Package_add_file,officecfg_misc,xml/oo-ldap.xcd.sample,registry/schema/oo-ldap.xcd.sample))
$(eval $(call gb_Package_add_file,officecfg_misc,xml/oo-ad-ldap.xcd.sample,registry/schema/oo-ad-ldap.xcd.sample))
$(eval $(call gb_Package_add_file,officecfg_misc,pck/oo-ldap-attr-map.properties,registry/schema/oo-ldap-attr-map.properties))
$(eval $(call gb_Package_add_file,officecfg_misc,pck/oo-org-map.properties,registry/schema/oo-org-map.properties))
$(eval $(call gb_Package_add_file,officecfg_misc,pck/oo-common.ldif,registry/schema/oo-common.ldif))
$(eval $(call gb_Package_add_file,officecfg_misc,pck/oo-common-ad.ldf,registry/schema/oo-common-ad.ldf))
$(eval $(call gb_Package_add_file,officecfg_misc,pck/oo-common.conf,registry/schema/oo-common.conf))

