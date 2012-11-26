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



.PHONY : setuplocal removelocal
ifneq ($(gb_LOCALBUILDDIR),)
ifneq ($(wildcard $(gb_LOCALBUILDDIR)/SetupLocal.mk),)
setuplocal :
	$(eval $(call gb_Output_error,$(gb_LOCALBUILDDIR) exists already.))

else

setuplocal :
	$(eval MODULE := $(firstword $(MODULE) $(lastword $(subst /, ,$(dir $(realpath $(firstword $(MAKEFILE_LIST))))))))
	$(eval modulerepo := $(patsubst %/$(MODULE),%,$(foreach repo,$(gb_REPOS),$(wildcard $(repo)/$(MODULE)))))
	$(eval $(call gb_Output_announce,setting up local build directory (module: $(MODULE)).,$(true),SYC,5))
	mkdir -p $(gb_LOCALBUILDDIR)/srcdir $(gb_LOCALBUILDDIR)/workdir $(gb_LOCALBUILDDIR)/outdir
	rsync --archive --exclude 'workdir/**' $(SOLARVERSION)/$(INPATH)/ $(gb_LOCALBUILDDIR)/outdir
	cp $(modulerepo)/Repository.mk $(gb_LOCALBUILDDIR)/srcdir/Repository.mk
	cp $(modulerepo)/RepositoryFixes.mk $(gb_LOCALBUILDDIR)/srcdir/RepositoryFixes.mk
	rsync --archive $(modulerepo)/$(MODULE)/ $(gb_LOCALBUILDDIR)/srcdir/$(MODULE)
	echo "gb_REPOS := $(gb_LOCALBUILDDIR)/srcdir $(filter-out $(patsubst %/$(MODULE),%,$(foreach repo,$(gb_REPOS),$(wildcard $(repo)/$(MODULE)))),$(gb_REPOS))" > $(gb_LOCALBUILDDIR)/SetupLocal.mk
	echo "#original gb_REPOS was $(gb_REPOS)" >> $(gb_LOCALBUILDDIR)/SetupLocal.mk
	echo "OUTDIR := $(gb_LOCALBUILDDIR)/outdir" >> $(gb_LOCALBUILDDIR)/SetupLocal.mk
	echo "#original OUTDIR was $(OUTDIR)" >> $(gb_LOCALBUILDDIR)/SetupLocal.mk
	echo "WORKDIR := $(gb_LOCALBUILDDIR)/workdir" >> $(gb_LOCALBUILDDIR)/SetupLocal.mk
	echo "#original WORKDIR was $(WORKDIR)" >> $(gb_LOCALBUILDDIR)/SetupLocal.mk

endif

removelocal :
	$(eval $(call gb_Output_warn,removing directory $(gb_LOCALBUILDDIR).,SYC))
	sleep 10
	rm -rf $(gb_LOCALBUILDDIR)

else
setuplocal:
	$(eval $(call gb_Output_error,setuplocal: gb_LOCALBUILDDIR is not set.))

removelocal:
	$(eval $(call gb_Output_error,setuplocal: gb_LOCALBUILDDIR is not set.))

endif

# vim: set noet sw=4 ts=4:
