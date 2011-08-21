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

# this package contains stuff that is used during the build

$(eval $(call gb_Package_Package,officecfg_tools,$(SRCDIR)/officecfg))

$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/alllang.xsl,util/alllang.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/schema_val.xsl,util/schema_val.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/schema_trim.xsl,util/schema_trim.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/resource.xsl,util/resource.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/sanity.xsl,util/sanity.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/data_val.xsl,util/data_val.xsl))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/processing/delcomment.sed,util/delcomment.sed))

$(eval $(call gb_Package_add_file,officecfg_tools,xml/registry/data.dtd,registry/data.dtd))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/registry/component-update.dtd,registry/component-update.dtd))
$(eval $(call gb_Package_add_file,officecfg_tools,xml/registry/component-schema.dtd,registry/component-schema.dtd))

# vim: set noet sw=4 ts=4:
