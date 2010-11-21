#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# this is an optional extension to gbuild
# it depends on scripts outside the gbuild directory
# nothing in the gbuild core should ever depend on it

define gb_PackModule_setpackmodulecommand
ifeq ($$(words $(gb_Module_ALLMODULES)),1)
$$(eval $$(call gb_Output_announce,$$(strip $$(gb_Module_ALLMODULES)),$$(true),ZIP,5))
packmodule : COMMAND := $$(SOLARENV)/bin/packmodule $$(OUTDIR) $$(strip $$(gb_Module_ALLMODULES)) $$(UPDMINOR)
else
$$(eval $$(call gb_Output_announce,more than one module - creating no zipped package,$$(true),ZIP,5))
packmodule : COMMAND := true
endif
endef

.PHONY : packmodule cleanpackmodule
packmodule : all
    $(eval $(call gb_PackModule_setpackmodulecommand))
    $(COMMAND)

# TODO: implement cleanpackmodule
# Should cleanpackmodule depend on clean and thus remove the build too?  That
# would be consistent with the current behaviour of packmodule.  Or should it
# only remove the packed module, but nothing else?  Or should packmodule have
# an order only dependency on all?  Then one could either pack whats there with
# "packmodule" (might fail on an incomplete build) or make sure everything is
# up-to-date with "packmodule all".


# vim: set noet ts=4 sw=4:
