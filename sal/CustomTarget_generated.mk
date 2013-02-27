# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,sal/generated))

sal_DIR := $(call gb_CustomTarget_get_workdir,sal/generated)

$(call gb_CustomTarget_get_target,sal/generated) : \
	$(sal_DIR)/sal/udkversion.h \
	$(if $(filter-out $(COM),MSC),$(sal_DIR)/sal/typesizes.h)

$(sal_DIR)/sal/typesizes.h :| $(sal_DIR)/sal/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	cp $(SRCDIR)/config_host/config_typesizes.h $@

include $(SRCDIR)/solenv/inc/udkversion.mk

$(sal_DIR)/sal/udkversion.h :| $(sal_DIR)/sal/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo '#ifndef _SAL_UDKVERSION_H_'           >  $@
	echo '#define _SAL_UDKVERSION_H_'           >> $@
	echo ''                                     >> $@
	echo '#define SAL_UDK_MAJOR "$(UDK_MAJOR)"' >> $@
	echo '#define SAL_UDK_MINOR "$(UDK_MINOR)"' >> $@
	echo '#define SAL_UDK_MICRO "$(UDK_MICRO)"' >> $@
	echo ''                                     >> $@
	echo '#endif'                               >> $@

# vim: set noet sw=4 ts=4:
