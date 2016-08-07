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



$(eval $(call gb_Library_Library,msword))

$(eval $(call gb_Library_set_componentfile,msword,sw/util/msword))

$(eval $(call gb_Library_set_include,msword,\
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/inc/pch \
    -I$(SRCDIR)/sw/inc \
    -I$(WORKDIR)/inc/sw/sdi \
    -I$(WORKDIR)/Misc/sw/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/sw \
))

$(eval $(call gb_Library_add_api,msword,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,msword,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    i18nutil \
    msfilter \
    sal \
    sfx \
    sot \
    stl \
    svl \
    svt \
    svx \
    svx \
    svxcore \
    sw \
    ootk \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_external,msword,icuuc)

$(eval $(call gb_Library_add_exception_objects,msword,\
    sw/source/filter/rtf/rtffld \
    sw/source/filter/rtf/rtffly \
    sw/source/filter/rtf/rtfnum \
    sw/source/filter/rtf/rtftbl \
    sw/source/filter/rtf/swparrtf \
    sw/source/filter/ww8/rtfattributeoutput \
    sw/source/filter/ww8/rtfexport \
    sw/source/filter/ww8/rtfexportfilter \
    sw/source/filter/ww8/rtfimportfilter \
    sw/source/filter/ww8/rtfsdrexport \
    sw/source/filter/ww8/WW8FFData \
    sw/source/filter/ww8/WW8FibData \
    sw/source/filter/ww8/WW8Sttbf \
    sw/source/filter/ww8/WW8TableInfo \
    sw/source/filter/ww8/fields \
    sw/source/filter/ww8/styles \
    sw/source/filter/ww8/tracer \
    sw/source/filter/ww8/writerhelper \
    sw/source/filter/ww8/writerwordglue \
    sw/source/filter/ww8/wrtw8esh \
    sw/source/filter/ww8/wrtw8nds \
    sw/source/filter/ww8/wrtw8num \
    sw/source/filter/ww8/wrtw8sty \
    sw/source/filter/ww8/wrtww8 \
    sw/source/filter/ww8/wrtww8gr \
    sw/source/filter/ww8/ww8atr \
    sw/source/filter/ww8/ww8glsy \
    sw/source/filter/ww8/ww8graf \
    sw/source/filter/ww8/ww8graf2 \
    sw/source/filter/ww8/ww8par \
    sw/source/filter/ww8/ww8par2 \
    sw/source/filter/ww8/ww8par3 \
    sw/source/filter/ww8/ww8par4 \
    sw/source/filter/ww8/ww8par5 \
    sw/source/filter/ww8/ww8par6 \
    sw/source/filter/ww8/ww8scan \
))

# vim: set noet sw=4 ts=4:
