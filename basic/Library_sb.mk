# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
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
#*************************************************************************

$(eval $(call gb_Library_Library,sb))

$(eval $(call gb_Library_add_package_headers,sb,basic_inc))

$(eval $(call gb_Library_add_precompiled_header,sb,$(SRCDIR)/basic/inc/pch/precompiled_basic))

$(eval $(call gb_Library_set_componentfile,sb,basic/util/sb))

$(eval $(call gb_Library_set_include,sb,\
	$$(INCLUDE) \
	-I$(realpath $(SRCDIR)/basic/inc) \
	-I$(realpath $(SRCDIR)/basic/inc/pch) \
	-I$(realpath $(SRCDIR)/basic/source/inc) \
))

$(eval $(call gb_Library_add_api,sb,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_add_defs,sb,\
	-DBASIC_DLLIMPLEMENTATION \
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
	utl \
	vcl \
	xcr \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/basmgr/basicmanagerrepository \
	basic/source/basmgr/basmgr \
	basic/source/basmgr/vbahelper \
	basic/source/classes/disas \
	basic/source/classes/errobject \
	basic/source/classes/eventatt \
	basic/source/classes/image \
	basic/source/classes/propacc \
	basic/source/classes/sb \
	basic/source/classes/sbintern \
	basic/source/classes/sbunoobj \
	basic/source/classes/sbxmod \
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
	basic/source/runtime/methods \
	basic/source/runtime/methods1 \
	basic/source/runtime/props \
	basic/source/runtime/runtime \
	basic/source/runtime/sbdiagnose \
	basic/source/runtime/stdobj \
	basic/source/runtime/stdobj1 \
	basic/source/runtime/step0 \
	basic/source/runtime/step1 \
	basic/source/runtime/step2 \
	basic/source/sbx/sbxarray \
	basic/source/sbx/sbxbase \
	basic/source/sbx/sbxbool \
	basic/source/sbx/sbxbyte \
	basic/source/sbx/sbxchar \
	basic/source/sbx/sbxcoll \
	basic/source/sbx/sbxcurr \
	basic/source/sbx/sbxdate \
	basic/source/sbx/sbxdbl \
	basic/source/sbx/sbxdec \
	basic/source/sbx/sbxexec \
	basic/source/sbx/sbxform \
	basic/source/sbx/sbxint \
	basic/source/sbx/sbxlng \
	basic/source/sbx/sbxmstrm \
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
# $(eval $(call gb_Library_add_linked_libs,sb,\
	canvastools \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,sb,\
	uwinapi \
	oleaut32 \
))
endif

ifeq ($(GUI)$(CPU),WNTI)
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-x86 \
))
else
ifeq ($(GUI)$(CPU),WNTX)
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-x64 \
))
else
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/dllmgr-none \
))
endif
endif
ifeq ($(GUI)$(COM)$(CPU),WNTMSCI)
$(eval $(call gb_Library_add_asmobjects,sb,\
	basic/source/runtime/wnt-x86 \
))
endif
ifeq ($(GUI)$(COM)$(CPU),WNTGCCI)
$(eval $(call gb_Library_add_exception_objects,sb,\
	basic/source/runtime/wnt-mingw \
))
endif

# vim: set noet sw=4 ts=4:
