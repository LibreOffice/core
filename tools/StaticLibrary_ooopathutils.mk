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



$(eval $(call gb_StaticLibrary_StaticLibrary,ooopathutils))

$(eval $(call gb_StaticLibrary_add_package_headers,ooopathutils,tools_inc))

$(eval $(call gb_StaticLibrary_add_exception_objects,ooopathutils,\
	tools/source/misc/pathutils \
))


# HACK for now
# We really should fix the clients of this to link against the static library
# Instead of this evil linking of an object from $(OUTDIR)
define StaticLibrary_ooopathutils_hack
$(call gb_StaticLibrary_get_target,ooopathutils) : $(OUTDIR)/lib/$(1)
$$(eval $$(call gb_Deliver_add_deliverable,$(OUTDIR)/lib/$(1),$(call gb_CxxObject_get_target,tools/source/misc/pathutils)))

$(OUTDIR)/lib/$(1) : $(call gb_CxxObject_get_target,tools/source/misc/pathutils)
	$$(call gb_Deliver_deliver,$$<,$$@)

endef

ifeq ($(OS),WNT)
$(eval $(call StaticLibrary_ooopathutils_hack,pathutils-obj.obj))
else
$(eval $(call StaticLibrary_ooopathutils_hack,pathutils-obj.o))
endif
# vim: set noet sw=4 ts=4:
