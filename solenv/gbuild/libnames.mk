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

gb_Library_NAMESCHEMES := OOO PLAIN RT RTVER STL UNO UNOVER

gb_Library_OOOLIBS :=\
    avmedia \
    basegfx \
    cui \
    drawinglayer \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    editeng \
    lng \
    msfilter \
    msword \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
    textconversiondlgs \
    tk \
    tl \
    utl \
    vbahelper \
    vcl \
    xo \
    xof \

gb_Library_PLAINLIBS :=\
    icuuc \
    xml2 \

gb_Library_RTLIBS :=\
    comphelper \
    i18nisolang1 \
    i18nutil \
    ucbhelper \
    vos3 \

gb_Library_RTVERLIBS :=\
    cppuhelper \
    salhelper \

gb_Library_STLLIBS :=\
    stl \

gb_Library_UNOLIBS :=\
    fsstorage \
    hatchwindowfactory \
    passwordcontainer \
    productregistration \
    vbaswobj \

gb_Library_UNOVERLIBS :=\
    jvmfwk \
    cppu \
    sal \


gb_StaticLibrary_NAMESCHEMES := PLAIN

gb_StaticLibrary_PLAINLIBS :=\
    salcpprt \
    jpeglib \
    zlib \
    
# vim: set noet sw=4 ts=4:
