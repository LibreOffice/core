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
#***********************************************************************/

PRJ = ..
PRJNAME = postprocess
TARGET = packcomponents

.INCLUDE: settings.mk

#TODO (copied from scp2/source/ooo/makefile.mk):
.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(ENABLE_GTK)" != ""
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
GTK_TWO_FOUR=$(shell @+-$(PKG_CONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo YES)
.END

my_components = \
    abp \
    adabasui \
    analysis \
    animcore \
    avmedia \
    basctl \
    basprov \
    bib \
    cached1 \
    calc \
    canvasfactory \
    chartcontroller \
    chartmodel \
    charttools \
    chartview \
    component/comphelper/util/comphelp \
    component/framework/util/fwk \
    component/framework/util/fwl \
    component/framework/util/fwm \
    component/vbahelper/util/msforms \
    component/sfx2/util/sfx \
    component/sot/util/sot \
    component/svl/source/fsstor/fsstorage \
    component/svl/source/passwordcontainer/passwordcontainer \
    component/svl/util/svl \
    component/svtools/source/hatchwindow/hatchwindowfactory \
    component/svtools/source/productregistration/productregistration.uno \
    component/svtools/util/svt \
    component/svx/util/svx \
    component/svx/util/svxcore \
    component/svx/util/textconversiondlgs \
    component/sw/util/msword \
    component/sw/util/sw \
    component/sw/util/swd \
    component/sw/util/vbaswobj \
    component/toolkit/util/tk \
    component/unoxml/source/rdf/unordf \
    component/unoxml/source/service/unoxml \
    component/xmloff/source/transform/xof \
    component/xmloff/util/xo \
    configmgr \
    ctl \
    date \
    dba \
    dbase \
    dbaxml \
    dbmm \
    dbp \
    dbpool2 \
    dbtools \
    dbu \
    deployment \
    deploymentgui \
    dlgprov \
    embobj \
    evtatt \
    fastsax \
    fileacc \
    filterconfig1 \
    flash \
    flat \
    for \
    fpicker \
    fps_office \
    frm \
    guesslang \
    hwp \
    hyphen \
    i18npool \
    i18nsearch \
    lng \
    lnth \
    localebe1 \
    log \
    mcnttype \
    migrationoo2 \
    msfilter \
    mysql \
    odbc \
    offacc \
    oooimprovecore \
    oox \
    package2 \
    pcr \
    pdffilter \
    placeware \
    preload \
    protocolhandler \
    pythonloader \
    pythonscript \
    res \
    rpt \
    rptui \
    rptxml \
    sax \
    sb \
    sc \
    scd \
    scn \
    scriptframe \
    sd \
    sdbc2 \
    sdbt \
    sdd \
    simplecanvas \
    slideshow \
    sm \
    smd \
    solver \
    spell \
    spl \
    srtrs1 \
    stringresource \
    svgfilter \
    syssh \
    t602filter \
    tvhlp1 \
    ucb1 \
    ucpchelp1 \
    ucpexpand1 \
    ucpext \
    ucpfile1 \
    ucpftp1 \
    ucphier1 \
    ucppkg1 \
    ucptdoc1 \
    updatefeed \
    updchk \
    updchk.uno \
    utl \
    uui \
    vbaobj \
    vbaevents \
    vcl \
    vclcanvas \
    wpft \
    writerfilter \
    xcr \
    xmlfa \
    xmlfd \
    xmx \
    xsltdlg \
    xsltfilter \
    xstor

.IF "$(OS)" != "MACOSX"
my_components += splash
.END

.IF "$(BUILD_SPECIAL)" != ""
my_components += oooimprovement
.END

.IF "$(DISABLE_ATL)" == ""
my_components += emboleobj
.END

.IF "$(DISABLE_NEON)" != "TRUE"
my_components += ucpdav1
.END

.IF "$(ENABLE_CAIRO)" == "TRUE"
my_components += cairocanvas
.END

.IF "$(ENABLE_GCONF)" != ""
my_components += gconfbe1
.END

.IF "$(ENABLE_GIO)" != ""
my_components += ucpgio
.END

.IF "$(ENABLE_GNOMEVFS)" != ""
my_components += ucpgvfs
.END

.IF "$(ENABLE_KAB)" != ""
my_components += kab1
.END

.IF "$(ENABLE_KDE)" != ""
my_components += kdebe1
.END

.IF "$(ENABLE_KDE4)" != ""
my_components += kde4be1
.END

.IF "$(ENABLE_OGL)" == "TRUE"
my_components += ogltrans
.END

.IF "$(ENABLE_SVCTAGS)" == "YES"
my_components += productregistration.jar
.END

.IF "$(SOLAR_JAVA)" == "TRUE"
my_components += \
    LuceneHelpWrapper \
    ScriptFramework \
    ScriptProviderForBeanShell \
    ScriptProviderForJava \
    ScriptProviderForJavaScript \
    XMergeBridge \
    XSLTFilter.jar \
    XSLTValidate \
    agenda \
    fax \
    form \
    hsqldb \
    jdbc \
    letter \
    query \
    report \
    table \
    web
.END

.IF "$(WITH_BINFILTER)" != "NO"
my_components += \
    bf_migratefilter \
    bindet
.END

.IF "$(WITH_LDAP)" == "YES"
my_components += ldapbe2
.END

.IF "$(WITH_MOZILLA)" != "NO"
my_components += \
    xmlsecurity \
    xsec_fw \
    xsec_xmlsec \
    pl
.END

.IF "$(OS)" == "MACOSX"
my_components += \
    MacOSXSpell \
    avmediaQuickTime \
    fps_aqua \
    macab1 \
    macbe1
.END

.IF "$(OS)" == "WNT"
my_components += \
    ado \
    dnd \
    dtrans \
    fop \
    fps \
    ftransl \
    java_uno_accessbridge \
    smplmail \
    sysdtrans \
    wininetbe1
.END

.IF "$(OS)" == "WNT" && "$(DISABLE_ATL)" == ""
my_components += \
    emser \
    oleautobridge
.END

.IF "$(OS)" == "WNT" && "$(ENABLE_DIRECTX)" != ""
my_components += \
    avmediawin \
    directx9canvas \
    gdipluscanvas
.END

.IF "$(OS)" == "WNT" && "$(ENABLE_DIRECTX)" != "" && "$(USE_DIRECTX5)" != ""
my_components += directx5canvas
.END

.IF "$(OS)" == "LINUX" || "$(OS)" == "NETBSD" || \
    ("$(OS)" == "SOLARIS" && "$(CPU)" == "S") || "$(OS)" == "WNT"
my_components += adabas
.END

.IF "$(OS)" != "MACOSX" && "$(SYSTEM_MOZILLA)" != "YES" && \
    "$(WITH_MOZILLA)" != "NO"
my_components += mozab
.ELSE
my_components += mozbootstrap
.END

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT"
my_components += desktopbe1
.END

.IF "$(GTK_TWO_FOUR)" != ""
my_components += fps_gnome
.END

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(ENABLE_KDE4)" != ""
my_components += fps_kde4
.END

.IF "$(OS)" != "WNT"
my_components += cmdmail
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_EVOAB2)" != ""
my_components += evoab
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_GSTREAMER)" != ""
my_components += avmediagstreamer
.END

.IF "$(OS)" != "WNT" && "$(SOLAR_JAVA)" == "TRUE"
my_components += avmedia.jar
.END

my_ooo_components = mailmerge

.INCLUDE: target.mk

ALLTAR : $(MISC)/services.rdb $(MISC)/ooo-services.rdb

$(MISC)/services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/services.input $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/services.input

$(MISC)/services.input : makefile.mk
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/ooo-services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/ooo-services.input \
        $(my_ooo_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/ooo-services.input

$(MISC)/ooo-services.input : makefile.mk
    echo '<list>' \
        '$(my_ooo_components:^"<filename>":+".component</filename>")' \
        '</list>' > $@
