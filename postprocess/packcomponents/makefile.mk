#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    acceptor \
    binaryurp \
    bootstrap \
    connector \
    introspection \
    invocadapt \
    invocation \
    javaloader \
    javavm \
    juh \
    namingservice \
    proxyfac \
    reflection \
    stocservices \
    streams \
    textinstream \
    textoutstream \
    uuresolver \
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
    component/cui/util/cui \
    component/drawinglayer/drawinglayer \
    component/framework/util/fwk \
    component/framework/util/fwl \
    component/framework/util/fwm \
    component/vbahelper/util/msforms \
    component/sfx2/util/sfx \
    component/sot/util/sot \
    component/svgio/svgio \
    component/svl/source/fsstor/fsstorage \
    component/svl/source/passwordcontainer/passwordcontainer \
    component/svl/util/svl \
    component/svtools/source/hatchwindow/hatchwindowfactory \
    component/svtools/util/svt \
    component/svx/util/svx \
    component/svx/util/svxcore \
    component/svx/util/textconversiondlgs \
    component/sw/util/msword \
    component/sw/util/sw \
    component/sw/util/swd \
    component/sw/util/vbaswobj \
    component/toolkit/util/ootk \
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
    i18npool \
    i18nsearch \
    lng \
    lnth \
    localebe1 \
    log \
    mailmerge \
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
    spl \
    srtrs1 \
    stringresource \
    svgfilter \
    syssh \
    t602filter \
    tvhlp1 \
    ucb1 \
    ucpchelp1 \
    ucpdav1 \
    ucpexpand1 \
    ucpext \
    ucpfile1 \
    ucpftp1 \
    ucphier1 \
    ucppkg1 \
    ucptdoc1 \
    updatefeed \
    updchk \
    utl \
    uui \
    vbaevents \
    vbaobj \
    vclcanvas \
    writerfilter \
    xcr \
    xmlfa \
    xmlfd \
    xmx \
    xsltdlg \
    xstor \
    PresentationMinimizer \
    PresenterScreen

.IF "$(OS)" == "MACOSX"
my_components += component/vcl/vcl.macosx
.ELSE
.IF "$(OS)" == "WNT" || "$(OS)" == "OS2"
my_components += component/vcl/vcl.windows
.ELSE
my_components += component/vcl/vcl.unx
.ENDIF
.ENDIF

.IF "$(ENABLE_ONLINE_UPDATE)"=="YES"
my_components += updchk.uno
.END

.IF "$(ENABLE_OOOIMPROVEMENT)" != ""
my_components += oooimprovement
.END

.IF "$(ENABLE_COINMP)"=="YES"
my_components += solver
.END

.IF "$(DISABLE_SAXON)" == ""
my_components += XSLTFilter.jar \
    xsltfilter
.END

.IF "$(ENABLE_HYPHEN)" == "YES"
my_components += hyphen
.END

.IF "$(ENABLE_HUNSPELL)" == "YES"
my_components += spell
.END

.IF "$(SYSTEM_LIBWPD)" == "YES"
my_components +=     wpft
.END

.IF "$(DISABLE_ATL)" == ""
my_components += emboleobj
.END

.IF "$(ENABLE_CAIRO_CANVAS)" == "TRUE"
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

.IF "$(ENABLE_OPENGL)" == "TRUE"
my_components += ogltrans
.END

.IF "$(SOLAR_JAVA)" == "TRUE"
my_components += \
    LuceneHelpWrapper \
    ScriptFramework \
    ScriptProviderForJava \
    XMergeBridge \
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
.IF "$(ENABLE_BEANSHELL)" == "YES"
my_components += ScriptProviderForBeanShell
.END
.IF "$(ENABLE_JAVASCRIPT)" == "YES"
my_components += ScriptProviderForJavaScript
.END
.END

.IF "$(WITH_LDAP)" == "YES"
my_components += ldapbe2
.END

.IF "$(ENABLE_NSS_MODULE)" != "NO"
my_components += \
    xmlsecurity \
    xsec_fw \
    xsec_xmlsec
.END

.IF "$(OS)" == "MACOSX"
my_components += \
    avmediaQuickTime \
    fps_aqua \
    macab1 \
    macbe1 \
    MacOSXSpell
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
    sysmail \
    sysdtrans \
    winaccessibility \
    wininetbe1
.END

.IF "$(OS)" == "OS2"
my_components += \
    dtrans \
    sysdtrans
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

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(OS)" != "OS2"
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
my_components += avmediagst
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
