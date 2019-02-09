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

include $(SRCDIR)/solenv/inc/version.lst

$(eval $(call gb_Package_Package,solenv_versionlist,$(WORKDIR)/CustomTarget/solenv/inc))

$(eval $(call gb_Package_add_file,solenv_versionlist,inc/versionlist.hrc,versionlist.hrc))

$(WORKDIR)/CustomTarget/solenv/inc/versionlist.hrc : $(SRCDIR)/solenv/inc/version.lst
	mkdir -p $(dir $@)
	@echo "#define VERSION $(OOOBASEVERSIONMAJOR)" > $@
	@echo "#define SUBVERSION $(OOOBASEVERSIONMINOR)" >> $@
	@echo "#define MICROVERSION $(OOOBASEVERSIONMICRO)" >> $@
	@echo "#define VER_DAY $(OOOBASEVERSIONDAY)" >> $@
	@echo "#define VER_MONTH $(OOOBASEVERSIONMONTH)" >> $@
	@echo "#define VER_YEAR $(OOOBASEVERSIONYEAR)" >> $@

