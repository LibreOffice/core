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

gb_REPOSITORYDIRNAME := SRCDIR


gb_Executable_UREBIN := \

gb_Executable_SDK := \

gb_Executable_OOO := \

gb_Executable_BRAND := \

# not installed
gb_Executable_NONE := \
    mkunroll \
    rscdep \
    so_checksum \
    sspretty \
    bmp \
    bmpsum \
    g2g \

gb_Library_OOOLIBS :=\
    avmedia \
    basegfx \
    cui \
    drawinglayer \
    eggtray \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    editeng \
    lng \
    msfilter \
    msword \
    qstart_gtk \
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
    xcr \

gb_Library_PLAINLIBS_URE :=\
    xml2 \

gb_Library_PLAINLIBS_OOO :=\
    icuuc \


# => OOOLIB
gb_Library_RTLIBS :=\
    comphelper \
    i18nisolang1 \
    i18nutil \
    ucbhelper \
    vos3 \

# => URELIB
gb_Library_RTVERLIBS :=\
    cppuhelper \
    salhelper \

# => URELIB
gb_Library_STLLIBS :=\
    stl \

gb_Library_UNOLIBS_URE := \

gb_Library_UNOLIBS_OOO := \
    fsstorage \
    hatchwindowfactory \
    passwordcontainer \
    productregistration \
    vbaswobj \


# => URELIB
gb_Library_UNOVERLIBS :=\
    cppu \
    jvmfwk \
    sal \

gb_StaticLibrary_PLAINLIBS :=\
    jpeglib \
    ooopathutils \
    salcpprt \
    zlib \
    
# vim: set noet sw=4 ts=4:
