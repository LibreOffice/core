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
# unixify windows paths
ifeq ($(OS),WNT)
override WORKDIR := $(shell cygpath -u $(WORKDIR))
override OUTDIR := $(shell cygpath -u $(OUTDIR))
override gb_REPOS := $(foreach repo,$(gb_REPOS),$(shell cygpath -u $(repo)))
endif

REPODIR := $(patsubst %/,%,$(dir $(firstword $(gb_REPOS))))

ifeq ($(filter setuplocal removelocal,$(MAKECMDGOALS)),)
ifneq ($(filter-out $(foreach repo,$(gb_REPOS),$(realpath $(repo))/%),$(realpath $(firstword $(MAKEFILE_LIST)))),)
$(eval $(call gb_Output_error,The initial makefile $(realpath $(firstword $(MAKEFILE_LIST))) is not in the repositories $(foreach repo,$(gb_REPOS),$(realpath $(repo))).,ALL))
endif
endif

# vim: set noet sw=4 ts=4:
