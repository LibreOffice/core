# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,components))

$(eval $(call gb_Library_set_plugin_for,components,cppuhelper))

$(eval $(call gb_Library_add_generated_exception_objects,components,\
    CustomTarget/static/component_maps \
))

# ENABLE_WASM_STRIP_ACCESSIBILITY:
#
# This little block (without comments) makes the UNO API unoapploader
# component to be linked into the executables - not by being added
# to those single executables individually, but by using
# - originally - the lines
#    $(eval $(call gb_Library_use_libraries,acc,\
#        $(gb_CPPU_ENV)_uno \
#    ))
# to do that. This extends the sources added to the accessibility
# module - so everything linked *against* accessibility *will*
# also be linked against the needed $(gb_CPPU_ENV)_uno statically.
#
# This is a nice *trick*, but has some caveats:
# - You *need to know* that acc <-> accessibility is used in the
#   build system for accessibility defines (as shortcut?)
# - Any executable/target that does *not* link against 'acc'
#   will fail (happr searching for why that is..)
# - If you remove accessibility - as I did - you get cryptic
#   messages and link errors - not a single executable will link
#   anymore - sigh
# - you *need to know* enough about the build system - e.g. that
#   you *can* add modules to any lib at *any* place, here. for
#   example
#
# I think for the futue and for an universally usable static
# linking the lib would need to be adfded to *each* target
# individually.
#
# For now, I have just identified another commonly linked-against
# libray to use for this 'trick', the framework one...
$(eval $(call gb_Library_use_libraries,fwk,\
    $(gb_CPPU_ENV)_uno \
))

# vim: set noet sw=4 ts=4:
