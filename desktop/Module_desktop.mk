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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

# FIXME: Okay, so this is ugly hack, because
# include $(dir $(realpath $(firstword $(MAKEFILE_LIST))))Pagein.mk
# does not work from tail_build. I think I should just move Pagein.mk
# into gbuild proper...
#
# On the other side, I wonder how many of our gbuild classes /
# implementations would break when using with multiple repos as they
# were originally intended, i.e., with different root dirs.
include $(SRCDIR)/desktop/Pagein.mk

$(eval $(call gb_Module_Module,desktop))

$(eval $(call gb_Module_add_targets,desktop,\
    AllLangResTarget_deployment \
    AllLangResTarget_deploymentgui \
    AllLangResTarget_dkt \
    Executable_soffice.bin \
    Executable_unopkg.bin \
    Library_deployment \
    Library_deploymentgui \
    Library_deploymentmisc \
    Library_migrationoo2 \
    Library_migrationoo3 \
    Library_offacc \
    Library_sofficeapp \
    Library_spl \
    Library_unopkgapp \
    Package_branding \
    Package_intro \
    Package_scripts \
    Pagein_calc \
    Pagein_common \
    Pagein_draw \
    Pagein_impress \
    Pagein_writer \
    Zip_brand \
    Zip_brand_dev \
    Zip_shell \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_guistdio \
    Executable_loader2 \
    Executable_quickstart \
    Executable_sbase \
    Executable_scalc \
    Executable_sdraw \
    Executable_simpress \
    Executable_smath \
    Executable_soffice \
    Executable_sweb \
    Executable_swriter \
    Executable_unoinfo \
    Executable_unopkg \
    Executable_unopkg.com \
    Package_manifest \
    Package_rcfile \
    WinResTarget_quickstart \
    WinResTarget_sbase \
    WinResTarget_scalc \
    WinResTarget_sdraw \
    WinResTarget_simpress \
    WinResTarget_soffice \
    WinResTarget_smath \
    WinResTarget_sweb \
    WinResTarget_swriter \
))

else

ifeq ($(OS),MACOSX)

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_officeloader \
))

else

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_oosplash \
    Library_spl_unx \
))

endif

endif

# vim: set ts=4 sw=4 et:
