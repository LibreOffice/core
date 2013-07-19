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

$(eval $(call gb_Library_Library,sb))

$(eval $(call gb_Library_set_componentfile,sb,basic/util/sb))

$(eval $(call gb_Library_set_include,sb,\
	$$(INCLUDE) \
	-I$(SRCDIR)/basic/inc \
	-I$(SRCDIR)/basic/source/inc \
))

$(eval $(call gb_Library_use_external,sb,boost_headers))

$(eval $(call gb_Library_use_sdk_api,sb))
$(eval $(call gb_Library_use_api,sb,oovbaapi))

$(eval $(call gb_Library_add_defs,sb,\
	-DBASIC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,sb,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	i18nlangtag \
	sot \
	svl \
	svt \
	tl \
	utl \
	vcl \
	xmlscript \
	$(gb_UWINAPI) \
))

ifneq ($(DISABLE_SCRIPTING),TRUE)

$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/basmgr/basicmanagerrepository \
	basic/source/basmgr/basmgr \
	basic/source/basmgr/vbahelper \
	basic/source/classes/codecompletecache \
	basic/source/classes/errobject \
	basic/source/classes/eventatt \
	basic/source/classes/global \
	basic/source/classes/image \
	basic/source/classes/propacc \
	basic/source/classes/sb \
	basic/source/classes/sbintern \
	basic/source/classes/sbunoobj \
	basic/source/classes/sbxmod \
	basic/source/comp/basiccharclass \
	basic/source/comp/buffer \
	basic/source/comp/codegen \
	basic/source/comp/dim \
	basic/source/comp/exprtree \
	basic/source/comp/exprgen \
	basic/source/comp/exprnode \
	basic/source/comp/io \
	basic/source/comp/loops \
	basic/source/comp/parser \
	basic/source/comp/sbcomp \
	basic/source/comp/scanner \
	basic/source/comp/symtbl \
	basic/source/comp/token \
	basic/source/uno/dlgcont \
	basic/source/uno/modsizeexceeded \
	basic/source/uno/namecont \
	basic/source/uno/sbmodule \
	basic/source/uno/sbservices \
	basic/source/uno/scriptcont \
	basic/source/runtime/basrdll \
	basic/source/runtime/comenumwrapper \
	basic/source/runtime/ddectrl \
	basic/source/runtime/inputbox \
	basic/source/runtime/iosys \
	basic/source/runtime/props \
	basic/source/runtime/runtime \
	basic/source/runtime/sbdiagnose \
	basic/source/runtime/stdobj \
	basic/source/runtime/stdobj1 \
))

endif

$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/methods \
	basic/source/runtime/methods1 \
	basic/source/sbx/sbxarray \
	basic/source/sbx/sbxbool \
	basic/source/sbx/sbxbyte \
	basic/source/sbx/sbxchar \
	basic/source/sbx/sbxcoll \
	basic/source/sbx/sbxcurr \
	basic/source/sbx/sbxbase \
	basic/source/sbx/sbxdate \
	basic/source/sbx/sbxdbl \
	basic/source/sbx/sbxdec \
	basic/source/sbx/sbxexec \
	basic/source/sbx/sbxform \
	basic/source/sbx/sbxint \
	basic/source/sbx/sbxlng \
	basic/source/sbx/sbxobj \
	basic/source/sbx/sbxres \
	basic/source/sbx/sbxscan \
	basic/source/sbx/sbxsng \
	basic/source/sbx/sbxstr \
	basic/source/sbx/sbxuint \
	basic/source/sbx/sbxulng \
	basic/source/sbx/sbxvalue \
	basic/source/sbx/sbxvar \
))

# Uncomment the following line if DBG_TRACE_PROFILING is active in source/inc/sbtrace.hxx
# $(eval $(call gb_Library_use_libraries,sb,\
	canvastools \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,sb,\
	oleaut32 \
))
endif

ifeq ($(OS)$(CPUNAME),WNTINTEL)
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-x86 \
))
else
ifeq ($(OS)$(CPUNAME),WNTX86_64)
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-x64 \
))
else
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-none \
))
endif
endif
ifeq ($(OS)$(COM)$(CPUNAME),WNTMSCINTEL)
$(eval $(call gb_Library_add_asmobjects,sb,\
	basic/source/runtime/wnt-x86 \
))
endif
ifeq ($(OS)$(COM)$(CPUNAME),WNTGCCINTEL)
$(eval $(call gb_Library_add_asmobjects,sb,\
	basic/source/runtime/wnt-mingw \
))
endif

# Runtime dependency for unit-tests
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,sb)) :| \
	$(call gb_AllLangResTarget_get_target,sb)

# vim: set noet sw=4 ts=4:
