#*************************************************************************

# extend for JDK include (seems only needed in setsolar env?)
SOLARINC += $(JDKINCS)

OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

.PHONY : setuplocal removelocal
ifneq ($(gb_LOCALBUILDDIR),)
ifneq ($(wildcard $(gb_LOCALBUILDDIR)/SetupLocal.mk),)
include $(gb_LOCALBUILDDIR)/SetupLocal.mk
$(eval $(call gb_Output_info,Using local setup at $(gb_LOCALBUILDDIR).,ALL))
$(eval $(call gb_Output_info,gb_REPOS:=$(gb_REPOS),ALL))
$(eval $(call gb_Output_info,WORKDIR:=$(WORKDIR),ALL))
$(eval $(call gb_Output_info,OUTDIR:=$(OUTDIR),ALL))

setuplocal :
	$(eval, $(call gb_Output_error,$(gb_LOCALBUILDDIR) exists already.))

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

endif

removelocal :
	$(eval $(call gb_Output_warn,removing directory $(gb_LOCALBUILDDIR).,SYC))
	sleep 10
	rm -rf $(gb_LOCALBUILDDIR)

ifeq ($(strip $(gb_REPOS)),)
gb_REPOS := $(SOLARSRC)
endif
SRCDIR := $(firstword $(gb_REPOS))

# HACK
# unixify windoze paths
ifeq ($(OS),WNT)
SRCDIR := $(shell cygpath -u $(SRCDIR))
WORKDIR := $(shell cygpath -u $(WORKDIR))
OUTDIR := $(shell cygpath -u $(OUTDIR))
gb_REPOS := $(shell cygpath -u $(gb_REPOS))
endif

REPODIR := $(patsubst %/,%,$(dir $(firstword $(gb_REPOS))))

ifneq ($(MAKECMDGOALS),setuplocal)
ifneq ($(filter-out $(foreach repo,$(gb_REPOS),$(realpath $(repo))/%),$(realpath $(firstword $(MAKEFILE_LIST)))),)
$(eval $(call gb_Output_warn,The initial makefile $(realpath $(firstword $(MAKEFILE_LIST))) is not in the repositories $(foreach repo,$(gb_REPOS),$(realpath $(repo))).,ALL))
$(shell sleep 10)
endif
endif

# vim: set noet sw=4 ts=4:
