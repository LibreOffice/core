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



$(eval $(call gb_Package_Package,xmloff_dtd,$(SRCDIR)/xmloff/dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/Blocklist.dtd,Blocklist.dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/chart.mod,chart.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/datastyl.mod,datastyl.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/defs.mod,defs.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/drawing.mod,drawing.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/dtypes.mod,dtypes.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/form.mod,form.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/meta.mod,meta.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/nmspace.mod,nmspace.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/office.dtd,office.dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/office.mod,office.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/openoffice-2.0-schema.rng,openoffice-2.0-schema.rng))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/script.mod,script.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/settings.mod,settings.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/style.mod,style.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/table.mod,table.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/text.mod,text.mod))
