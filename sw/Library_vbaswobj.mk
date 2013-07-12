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

$(eval $(call gb_Library_Library,vbaswobj))

$(eval $(call gb_Library_set_componentfile,vbaswobj,sw/util/vbaswobj))

$(eval $(call gb_Library_set_precompiled_header,vbaswobj,$(SRCDIR)/sw/inc/pch/precompiled_vbaswobj))

$(eval $(call gb_Library_set_include,vbaswobj,\
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_api,vbaswobj,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_use_libraries,vbaswobj,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    sal \
    sb \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    tk \
    tl \
    utl \
    vbahelper \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,vbaswobj,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,vbaswobj,\
    sw/source/ui/vba/service \
    sw/source/ui/vba/vbarange \
    sw/source/ui/vba/vbacell \
    sw/source/ui/vba/vbacells \
    sw/source/ui/vba/vbacolumn \
    sw/source/ui/vba/vbacolumns \
    sw/source/ui/vba/vbaframe \
    sw/source/ui/vba/vbaframes \
    sw/source/ui/vba/vbalistformat \
    sw/source/ui/vba/vbalistgallery \
    sw/source/ui/vba/vbalistgalleries \
    sw/source/ui/vba/vbalisthelper \
    sw/source/ui/vba/vbalistlevel \
    sw/source/ui/vba/vbalistlevels \
    sw/source/ui/vba/vbalisttemplate \
    sw/source/ui/vba/vbalisttemplates \
    sw/source/ui/vba/vbarevision \
    sw/source/ui/vba/vbarevisions \
    sw/source/ui/vba/vbarow \
    sw/source/ui/vba/vbarows \
    sw/source/ui/vba/vbadocument \
    sw/source/ui/vba/vbasections \
    sw/source/ui/vba/vbadialog \
    sw/source/ui/vba/vbawrapformat \
    sw/source/ui/vba/vbafont \
    sw/source/ui/vba/vbarangehelper \
    sw/source/ui/vba/vbaaddin \
    sw/source/ui/vba/vbaautotextentry \
    sw/source/ui/vba/vbadocumentproperties \
    sw/source/ui/vba/vbaeventshelper \
    sw/source/ui/vba/vbastyle \
    sw/source/ui/vba/vbapane \
    sw/source/ui/vba/vbaglobals \
    sw/source/ui/vba/vbatemplate \
    sw/source/ui/vba/vbaaddins \
    sw/source/ui/vba/vbaview \
    sw/source/ui/vba/vbaheaderfooter \
    sw/source/ui/vba/vbaheadersfooters \
    sw/source/ui/vba/vbaheaderfooterhelper \
    sw/source/ui/vba/vbabookmarks \
    sw/source/ui/vba/vbaoptions \
    sw/source/ui/vba/vbadialogs \
    sw/source/ui/vba/vbapalette \
    sw/source/ui/vba/vbaparagraph \
    sw/source/ui/vba/vbafind \
    sw/source/ui/vba/vbasection \
    sw/source/ui/vba/vbabookmark \
    sw/source/ui/vba/vbaapplication \
    sw/source/ui/vba/vbawindow \
    sw/source/ui/vba/vbareplacement \
    sw/source/ui/vba/vbatable \
    sw/source/ui/vba/vbatablehelper \
    sw/source/ui/vba/vbatabstop \
    sw/source/ui/vba/vbatabstops \
    sw/source/ui/vba/vbaselection \
    sw/source/ui/vba/vbasystem \
    sw/source/ui/vba/vbainformationhelper \
    sw/source/ui/vba/vbapagesetup \
    sw/source/ui/vba/vbafield \
    sw/source/ui/vba/vbatables \
    sw/source/ui/vba/vbatableofcontents \
    sw/source/ui/vba/vbatablesofcontents \
    sw/source/ui/vba/vbavariable \
    sw/source/ui/vba/vbadocuments \
    sw/source/ui/vba/vbaparagraphformat \
    sw/source/ui/vba/vbaborders \
    sw/source/ui/vba/vbavariables \
    sw/source/ui/vba/vbastyles \
    sw/source/ui/vba/vbapanes \
    sw/source/ui/vba/wordvbahelper \
))

# vim: set noet sw=4 ts=4:
