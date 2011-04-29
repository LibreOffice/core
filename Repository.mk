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

$(eval $(call gb_Helper_register_repository,SRCDIR))


$(eval $(call gb_Helper_register_executables,NONE, \
    bmp \
    bmpsum \
    g2g \
    mkunroll \
    rscdep \
    so_checksum \
))

$(eval $(call gb_Helper_register_executables,OOO, \
    spadmin.bin \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    adabasui \
    avmedia \
    basegfx \
    canvastools \
    chartcontroller \
    chartmodel \
    charttools \
    chartview \
    cppcanvas \
    cui \
    dba \
    dbaxml \
    dbmm \
    dbtools \
    dbu \
    doctok \
    drawinglayer \
    editeng \
    for \
    forui \
    frm \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    i18npaper \
    lng \
    lwpft \
    msfilter \
    msword \
    oox \
    ooxml \
    qstart_gtk \
    resourcemodel \
    rpt \
    rptui \
    rptxml \
    sax \
    sb \
    sc \
    scd \
    scfilt \
    scui \
    sd \
    sdbt \
    sdd \
    sdfilt \
    sdui \
    sfx \
    sm \
    smd \
    sot \
    spa \
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
    unordf \
    unoxml \
    utl \
    vbahelper \
    vcl \
    writerfilter \
    xcr \
    xo \
    xof \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    xml2 \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	test \
    cppunit \
    crypto \
    fileacc \
    icui18n \
    icule \
    icuuc \
    rdf \
    ssl \
    xslt \
))


$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    ucbhelper \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    fsstorage \
    hatchwindowfactory \
    lomenubar \
    msforms \
    OGLTrans \
    passwordcontainer \
    productregistration \
    slideshow \
    vbaobj \
    vbaswobj \
    writerfilter_uno \
))


$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    sal \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    crypto \
    libeay32 \
    ssleay32 \
    jpeglib \
    ooopathutils \
    salcpprt \
    ssl \
    toolshelpers \
    vclmain \
    zlib \
))

# vim: set noet sw=4 ts=4:
