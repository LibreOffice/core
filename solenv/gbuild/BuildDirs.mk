# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

# The entire gbuild operates in unix paths, and then when calling the
# native tools, converts them back to the Windows native paths.
#
# The path overwriting below is EVIL, because after the recent changes when
# every module (even build.pl-based) is routed through gbuild, the modules
# using build.pl-based build will get OUTDIR in the cygwin format.
#
# To undo this, we explicitly set these vars to native Windows paths in
# build.pl again, otherwise we would have potentially different behavior in
# build.pl-based modules when you build them from the toplevel (using make
# all), and from the module itself (using cd module ; build )
ifeq ($(OS_FOR_BUILD),WNT)
override WORKDIR := $(shell cygpath -u $(WORKDIR))
override OUTDIR := $(shell cygpath -u $(OUTDIR))
override OUTDIR_FOR_BUILD := $(shell cygpath -u $(OUTDIR_FOR_BUILD))
override SRCDIR := $(shell cygpath -u $(SRCDIR))
endif

# vim: set noet sw=4:
