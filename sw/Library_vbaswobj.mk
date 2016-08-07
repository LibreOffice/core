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



$(eval $(call gb_Library_Library,vbaswobj))

$(eval $(call gb_Library_set_componentfile,vbaswobj,sw/util/vbaswobj))

$(eval $(call gb_Library_set_include,vbaswobj,\
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/inc/pch \
    -I$(SRCDIR)/sw/inc \
    -I$(WORKDIR)/Misc/sw/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/ \
))

$(eval $(call gb_Library_add_api,vbaswobj,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_Library_add_linked_libs,vbaswobj,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    sal \
    sb \
    sfx \
    stl \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    ootk \
    tl \
    utl \
    vbahelper \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vbaswobj,\
    sw/source/ui/vba/service \
    sw/source/ui/vba/vbadocument \
    sw/source/ui/vba/vbasections \
    sw/source/ui/vba/vbadialog \
    sw/source/ui/vba/vbawrapformat \
    sw/source/ui/vba/vbafont \
    sw/source/ui/vba/vbaheaderfooterhelper \
    sw/source/ui/vba/vbarangehelper \
    sw/source/ui/vba/vbaaddin \
    sw/source/ui/vba/vbaautotextentry \
    sw/source/ui/vba/vbarange \
    sw/source/ui/vba/vbadocumentproperties \
    sw/source/ui/vba/vbaeventshelper \
    sw/source/ui/vba/vbastyle \
    sw/source/ui/vba/vbapane \
    sw/source/ui/vba/vbaglobals \
    sw/source/ui/vba/vbatemplate \
    sw/source/ui/vba/vbaaddins \
    sw/source/ui/vba/vbaview \
    sw/source/ui/vba/vbaheaderfooter \
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
    sw/source/ui/vba/vbaselection \
    sw/source/ui/vba/vbasystem \
    sw/source/ui/vba/vbainformationhelper \
    sw/source/ui/vba/vbapagesetup \
    sw/source/ui/vba/vbafield \
    sw/source/ui/vba/vbatables \
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
