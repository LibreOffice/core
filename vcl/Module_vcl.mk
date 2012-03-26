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


$(eval $(call gb_Module_Module,vcl))

$(eval $(call gb_Module_add_targets,vcl,\
    AllLangResTarget_vcl \
    CustomTarget_afm_hash \
    Library_vcl \
    Package_inc \
    StaticLibrary_vclmain \
	Executable_ui-previewer \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_svp \
    Library_vclplug_gen \
    Library_desktop_detector \
    StaticLibrary_headless \
))

ifneq ($(ENABLE_GTK),)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk \
))
endif
ifneq ($(ENABLE_GTK3),)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk3 \
))
endif
ifneq ($(ENABLE_TDE),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_tde_moc \
    Executable_tdefilepicker \
    Library_vclplug_tde \
))
endif
ifneq ($(ENABLE_KDE),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kde_moc \
    Executable_kdefilepicker \
    Library_vclplug_kde \
))
endif
ifneq ($(ENABLE_KDE4),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kde4_moc \
    Library_vclplug_kde4 \
))
endif
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,vcl,\
    Zip_osxres \
))
endif

ifeq ($(GUIBASE),WIN)
$(eval $(call gb_Module_add_targets,vcl,\
    WinResTarget_vcl \
))
endif

# vim: set noet sw=4 ts=4:
