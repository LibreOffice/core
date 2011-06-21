#*************************************************************************
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s): Caolán McNamara <caolanm@redhat.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
#*************************************************************************

#please make generic modifications to unxgcc.mk or linux.mk
gb_CPUDEFS += -DARM32
gb_COMPILERDEFAULTOPTFLAGS := -Os
gb_CXXFLAGS += -fno-omit-frame-pointer
gb_CFLAGS += -fno-omit-frame-pointer

include $(GBUILDDIR)/platform/unxgcc.mk

# Set the proper enirotment variable so that our BUILD platform
# build-time shared libraries are found.
ifeq ($(OS_FOR_BUILD),MACOSX)
gb_Helper_set_ld_path := DYLD_LIBRARY_PATH=$(OUTDIR_FOR_BUILD)/lib
else
gb_Helper_set_ld_path := LD_LIBRARY_PATH=$(OUTDIR_FOR_BUILD)/lib
endif

# UnoApiTarget

gb_UnoApiTarget_IDLCTARGET := $(OUTDIR_FOR_BUILD)/bin/idlc
gb_UnoApiTarget_IDLCCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_IDLCTARGET)
gb_UnoApiTarget_REGMERGETARGET := $(OUTDIR_FOR_BUILD)/bin/regmerge
gb_UnoApiTarget_REGMERGECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGMERGETARGET)
gb_UnoApiTarget_REGCOMPARETARGET := $(OUTDIR_FOR_BUILD)/bin/regcompare
gb_UnoApiTarget_REGCOMPARECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGCOMPARETARGET)
gb_UnoApiTarget_CPPUMAKERTARGET := $(OUTDIR_FOR_BUILD)/bin/cppumaker
gb_UnoApiTarget_CPPUMAKERCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_CPPUMAKERTARGET)
gb_UnoApiTarget_REGVIEWTARGET := $(OUTDIR_FOR_BUILD)/bin/regview
gb_UnoApiTarget_REGVIEWCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGVIEWTARGET)

# vim: set noet sw=4:
