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



$(eval $(call gb_StaticLibrary_StaticLibrary,vclmain))

$(eval $(call gb_StaticLibrary_set_include,vclmain,\
	$$(INCLUDE) \
	-I$(SRCDIR)/vcl/inc \
	-I$(SRCDIR)/vcl/inc/pch \
	-I$(SRCDIR)/solenv/inc \
	-I$(OUTDIR)/inc/offuh \
	-I$(OUTDIR)/inc/stl \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,vclmain,\
	vcl/source/salmain/salmain \
))

# HACK for now
# We really should fix the clients of this to link against the static library
# Instead of this evil linking of an object from $(OUTDIR)
define StaticLibrary_salmain_hack
$(call gb_StaticLibrary_get_target,vclmain) : $(OUTDIR)/lib/$(1)
$$(eval $$(call gb_Deliver_add_deliverable,$(OUTDIR)/lib/$(1),$(call gb_CxxObject_get_target,vcl/source/salmain/salmain)))

$(OUTDIR)/lib/$(1) : $(call gb_CxxObject_get_target,vcl/source/salmain/salmain)
	$$(call gb_Deliver_deliver,$$<,$$@)

endef

ifeq ($(OS),WNT)
$(eval $(call StaticLibrary_salmain_hack,salmain.obj))
else
$(eval $(call StaticLibrary_salmain_hack,salmain.o))
endif

# vim: set noet sw=4 ts=4:
