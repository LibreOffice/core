# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Package_Package,jvmfwk_rcfiles,$(SRCDIR)/jvmfwk))

# The below files (intended to be also used during the build) need to go into
# the same directory as dynamic libraries (either bin or lib):

ifeq ($(GUI),WNT)
jvmfwk_Package_RCFILESDIR := bin
else
jvmfwk_Package_RCFILESDIR := lib
endif

$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(call gb_Helper_get_rcfile,$(jvmfwk_Package_RCFILESDIR)/sunjavaplugin),plugins/sunmajor/pluginlib/sunjavapluginrc))
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(call gb_Helper_get_rcfile,$(jvmfwk_Package_RCFILESDIR)/jvmfwk3),source/jvmfwk3rc))

ifeq ($(GUI),UNX)
ifeq ($(OS),FREEBSD)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_freebsd.xml))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_macosx.xml))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_linux.xml))
else ifeq ($(OS),AIX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_linux.xml))
else
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_unx.xml))
endif
else ifeq ($(GUI),WNT)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_wnt.xml))
else
$(call gb_Output_error,Unsupported platform)
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
