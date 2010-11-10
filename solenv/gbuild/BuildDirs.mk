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

# extend for JDK include (seems only needed in setsolar env?)
SOLARINC += $(JDKINCS)

OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

# if the goals do not include the targets install or uninstall we check if a
# local overlay build dir was requested. We are making an distiction between
# partitial and full builds. gb_LOCALPARTITIALBUILDDIR is only used here,
# later only gb_LOCALBUILDDIR is used.
ifeq ($(filter install uninstall,$(MAKECMDGOALS)),)
ifeq ($(gb_PARTITIALBUILD),$(true))
ifneq ($(gb_LOCALPARTITIALBUILDDIR),)
gb_LOCALBUILDDIR := $(gb_LOCALPARTITIALBUILDDIR)
OUTDIR := $(gb_LOCALBUILDDIR)/outdir/$(INPATH)
WORKDIR := $(gb_LOCALBUILDDIR)/workdir/$(INPATH)
else
gb_LOCALBUILDDIR :=
endif
else
ifneq ($(gb_LOCALBUILDDIR),)
OUTDIR := $(gb_LOCALBUILDDIR)/outdir/$(INPATH)
WORKDIR := $(gb_LOCALBUILDDIR)/workdir/$(INPATH)
endif
endif
endif

ifneq ($(gb_LOCALBUILDDIR),)
.PHONY : setuplocal
setuplocal :
    mkdir -p $(OUTDIR) $(WORKDIR)
    time rsync -a $(SOLARVERSION)/$(INPATH)/ $(OUTDIR)

endif

ifeq ($(strip $(gb_REPOS)),)
gb_REPOS := $(SOLARSRC)
endif
SRCDIR := $(firstword $(gb_REPOS))

# HACK
# unixify windoze paths
ifeq ($(OS),WNT)
WORKDIR := $(shell cygpath -u $(WORKDIR))
OUTDIR := $(shell cygpath -u $(OUTDIR))
gb_REPOS := $(shell cygpath -u $(gb_REPOS))
endif

REPODIR := $(patsubst %/,%,$(dir $(firstword $(gb_REPOS))))

# vim: set noet sw=4 ts=4:
