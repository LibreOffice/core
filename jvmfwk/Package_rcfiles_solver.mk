# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(OS),WNT)
jvmfwk_Package_rcfiles_solver_LIBDIR := bin
else
jvmfwk_Package_rcfiles_solver_LIBDIR := lib
endif

$(eval $(call gb_Package_Package,jvmfwk_rcfiles_solver,$(SRCDIR)/jvmfwk))

$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/$(call gb_Helper_get_rcfile,jvmfwk3),source/jvmfwk3rc))
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/$(call gb_Helper_get_rcfile,sunjavaplugin),plugins/sunmajor/pluginlib/sunjavapluginrc))

ifeq ($(OS),AIX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_linux.xml))
else ifeq ($(OS),FREEBSD)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_freebsd.xml))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_linux.xml))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_macosx.xml))
else ifeq ($(OS),WNT)
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_wnt.xml))
else
$(eval $(call gb_Package_add_file,jvmfwk_rcfiles_solver,$(jvmfwk_Package_rcfiles_solver_LIBDIR)/javavendors.xml,distributions/OpenOfficeorg/javavendors_unx.xml))
endif

# vim:set noet sw=4 ts=4:
