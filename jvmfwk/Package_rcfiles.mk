# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,jvmfwk_rcfiles,$(SRCDIR)/jvmfwk))

# The below files (intended to be also used during the build) need to go into
# the same directory as dynamic libraries (either bin or lib):

ifeq ($(OS),WNT)
jvmfwk_Package_RCFILESDIR := bin
else
jvmfwk_Package_RCFILESDIR := lib
endif

$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(call gb_Helper_get_rcfile,$(jvmfwk_Package_RCFILESDIR)/sunjavaplugin),plugins/sunmajor/pluginlib/sunjavapluginrc))
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(call gb_Helper_get_rcfile,$(jvmfwk_Package_RCFILESDIR)/jvmfwk3),source/jvmfwk3rc))

ifneq ($(OS),WNT)
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
else ifeq ($(OS),WNT)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles,$(jvmfwk_Package_RCFILESDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_wnt.xml))
else
$(call gb_Output_error,Unsupported platform)
endif

# vim:set noet sw=4 ts=4:
