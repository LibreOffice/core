# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# MODULE is the name of the module the makefile is located in

# declare a library
# LIB is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,msforms))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,msforms,vbahelper/util/msforms,services))

$(eval $(call gb_Library_use_external,msforms,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,msforms,vbahelper/inc/pch/precompiled_msforms))

$(eval $(call gb_Library_use_api,msforms,\
	udkapi \
	offapi \
	oovbaapi \
))

# add libraries to be linked to LIB; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_use_libraries,msforms,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sb \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    tk \
    tl \
    vbahelper \
    vcl \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,msforms,\
    vbahelper/source/msforms/vbabutton \
    vbahelper/source/msforms/vbacheckbox \
    vbahelper/source/msforms/vbacombobox \
    vbahelper/source/msforms/vbacontrol \
    vbahelper/source/msforms/vbacontrols \
    vbahelper/source/msforms/vbaframe \
    vbahelper/source/msforms/vbaimage \
    vbahelper/source/msforms/vbalabel \
    vbahelper/source/msforms/vbalistbox \
    vbahelper/source/msforms/vbalistcontrolhelper \
    vbahelper/source/msforms/vbamultipage \
    vbahelper/source/msforms/vbanewfont \
    vbahelper/source/msforms/vbapages \
    vbahelper/source/msforms/vbaprogressbar \
    vbahelper/source/msforms/vbaradiobutton \
    vbahelper/source/msforms/vbascrollbar \
    vbahelper/source/msforms/vbaspinbutton \
    vbahelper/source/msforms/vbasystemaxcontrol \
    vbahelper/source/msforms/vbatextbox \
    vbahelper/source/msforms/vbatogglebutton \
    vbahelper/source/msforms/vbauserform \
))

# vim: set noet sw=4 ts=4:
