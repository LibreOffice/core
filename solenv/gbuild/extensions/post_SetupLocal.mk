#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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

# vim: set noet sw=4:
