###############################################################
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
###############################################################



$(eval $(call gb_Library_Library,sb))

$(eval $(call gb_Library_add_package_headers,sb,basic_inc))

$(eval $(call gb_Library_add_precompiled_headers,sb,$(SRCDIR)/basic/inc/pch/precompiled_basic))

$(eval $(call gb_Library_set_componentfile,sb,basic/util/sb))

$(eval $(call gb_Library_add_api,sb,\
	offapi \
	oovbaapi \
	udkapi \
))

$(eval $(call gb_Library_add_defs,sb,\
        -DBASIC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_include,sb,\
	-I$(SRCDIR)/basic/inc \
	-I$(SRCDIR)/basic/inc/pch \
	-I$(SRCDIR)/basic/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,sb,\
        comphelper \
        cppu \
        cppuhelper \
        sal \
        salhelper \
        sot \
        svl \
        svt \
        tl \
        vcl \
        vos3 \
        stl \
        utl \
        xcr \
        $(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,sb,\
	oleaut32 \
	uwinapi \
))
endif

$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/basmgr/basmgr \
	basic/source/basmgr/basicmanagerrepository \
	basic/source/basmgr/vbahelper \
	basic/source/classes/sb \
	basic/source/classes/sbxmod \
	basic/source/classes/image \
	basic/source/classes/sbintern \
	basic/source/classes/sbunoobj \
	basic/source/classes/propacc \
	basic/source/classes/disas \
	basic/source/classes/errobject \
	basic/source/classes/eventatt \
	basic/source/comp/codegen \
	basic/source/comp/dim \
	basic/source/comp/exprtree \
	basic/source/comp/parser \
	basic/source/comp/sbcomp \
	basic/source/runtime/basrdll \
	basic/source/runtime/comenumwrapper \
	basic/source/runtime/inputbox \
	basic/source/runtime/runtime \
	basic/source/runtime/step0 \
	basic/source/runtime/step1 \
	basic/source/runtime/step2 \
	basic/source/runtime/iosys \
	basic/source/runtime/stdobj \
	basic/source/runtime/stdobj1 \
	basic/source/runtime/methods \
	basic/source/runtime/methods1 \
	basic/source/runtime/props \
	basic/source/runtime/ddectrl \
	basic/source/runtime/dllmgr \
	basic/source/runtime/sbdiagnose \
	basic/source/sbx/sbxarray \
	basic/source/uno/namecont \
	basic/source/uno/scriptcont \
	basic/source/uno/dlgcont \
	basic/source/uno/sbmodule \
	basic/source/uno/sbservices \
	basic/source/uno/modsizeexceeded \
))

$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/comp/buffer \
	basic/source/comp/exprgen \
	basic/source/comp/exprnode \
	basic/source/comp/io \
	basic/source/comp/loops \
	basic/source/comp/scanner \
	basic/source/comp/symtbl \
	basic/source/comp/token \
	basic/source/sbx/sbxbase \
	basic/source/sbx/sbxres \
	basic/source/sbx/sbxvalue \
	basic/source/sbx/sbxvals \
	basic/source/sbx/sbxvar \
	basic/source/sbx/sbxobj \
	basic/source/sbx/sbxcoll \
	basic/source/sbx/sbxexec \
	basic/source/sbx/sbxint \
	basic/source/sbx/sbxlng \
	basic/source/sbx/sbxsng \
	basic/source/sbx/sbxmstrm \
	basic/source/sbx/sbxdbl \
	basic/source/sbx/sbxcurr \
	basic/source/sbx/sbxdate \
	basic/source/sbx/sbxstr \
	basic/source/sbx/sbxbool \
	basic/source/sbx/sbxchar \
	basic/source/sbx/sbxbyte \
	basic/source/sbx/sbxuint \
	basic/source/sbx/sbxulng \
	basic/source/sbx/sbxform \
	basic/source/sbx/sbxscan \
	basic/source/sbx/sbxdec \
))


ifeq ($(GUI)$(COM)$(CPU),WNTMSCI)
$(eval $(call gb_Library_add_asmobjects,sb,\
	basic/source/runtime/wnt \
))
else ifeq ($(GUI)$(COM)$(CPU),WNTGCCI)
$(eval $(call gb_Library_add_asmobjects,sb,\
	basic/source/runtime/wnt-mingw \
))
endif

# vim: set noet sw=4 ts=4:
