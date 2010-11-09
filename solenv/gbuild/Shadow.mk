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

# returns the absolute shadow file path for the file
# $(1) absolute OUTDIR path
gb_Shadow__get_file = $(patsubst $(OUTDIR)%,$(SHADOWOUTDIR)%,$(1))

define gb_Shadow__deliver
mkdir -p $(dir $(2)) && cp -f $(1) $(2) && touch -r $(1) $(2)
endef

# copies the file from the younger source:
# - either the file at the same location in the shadowoutdir
# - or the workdir source
# $(1) target in the outdir (full path)
# $(2) source in the workdir (full path) or an empty string
ifneq ($(SHADOWOUTDIR),)
define gb_Shadow_copy
if [ $(2) ]; then $(call gb_Shadow__deliver,$(2),$(1)); else $(call gb_Shadow__deliver,$(call gb_Shadow__get_file,$(1)),$(1)); fi
endef

else
gb_Shadow_copy = $(call gb_Shadow__deliver,$(2),$(1))
endif

# vim: set noet sw=4 ts=4:
