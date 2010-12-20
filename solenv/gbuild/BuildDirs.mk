#*************************************************************************

# extend for JDK include (seems only needed in setsolar env?)
SOLARINC += $(JDKINCS)

OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

# Override for SetupLocal
ifneq ($(and $(gb_LOCALBUILDDIR),$(wildcard $(gb_LOCALBUILDDIR)/SetupLocal.mk)),)
include $(gb_LOCALBUILDDIR)/SetupLocal.mk
endif

ifeq ($(strip $(gb_REPOS)),)
gb_REPOS := $(SOLARSRC)
endif

# HACK
# unixify windoze paths
ifeq ($(OS),WNT)
override WORKDIR := $(shell cygpath -u $(WORKDIR))
override OUTDIR := $(shell cygpath -u $(OUTDIR))
override gb_REPOS := $(shell cygpath -u $(gb_REPOS))
endif

REPODIR := $(patsubst %/,%,$(dir $(firstword $(gb_REPOS))))

ifeq ($(filter setuplocal removelocal,$(MAKECMDGOALS)),)
ifneq ($(filter-out $(foreach repo,$(gb_REPOS),$(realpath $(repo))/%),$(realpath $(firstword $(MAKEFILE_LIST)))),)
$(eval $(call gb_Output_error,The initial makefile $(realpath $(firstword $(MAKEFILE_LIST))) is not in the repositories $(foreach repo,$(gb_REPOS),$(realpath $(repo))).,ALL))
endif
endif

# vim: set noet sw=4 ts=4:
