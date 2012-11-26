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



# this is an optional extension to gbuild
# it depends on scripts outside the gbuild directory
# nothing in the gbuild core should ever depend on it

.PHONY : packmodule cleanpackmodule

define gb_PackModule_setpackmodulecommand
ifeq ($$(words $(gb_Module_ALLMODULES)),1)
$$(eval $$(call gb_Output_announce,$$(strip $$(gb_Module_ALLMODULES)),$$(true),ZIP,5))
packmodule : COMMAND := $$(SOLARENV)/bin/packmodule $$(OUTDIR) $$(strip $$(gb_Module_ALLMODULES))
else
$$(eval $$(call gb_Output_announce,more than one module - creating no zipped package,$$(true),ZIP,5))
packmodule : COMMAND := true
endif
endef

packmodule : allandcheck deliverlog
	$(eval $(call gb_PackModule_setpackmodulecommand))
	$(COMMAND)

define gb_PackModule_setcleanpackmodulecommand
ifeq ($$(words $(gb_Module_ALLMODULES)),1)
$$(eval $$(call gb_Output_announce,$$(strip $$(gb_Module_ALLMODULES)),$$(false),ZIP,5))
cleanpackmodule : COMMAND := rm -f $$(OUTDIR)/zip/$$(strip $$(gb_Module_ALLMODULES)).zip
else
$$(eval $$(call gb_Output_announce,more than one module - deleting no zipped package,$$(false),ZIP,5))
cleanpackmodule : COMMAND := true
endif
endef

cleanpackmodule : clean
	$(eval $(call gb_PackModule_setcleanpackmodulecommand))
	$(COMMAND)
	
# vim: set noet ts=4 sw=4:
