# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Module_Module,cross_tail_build))

$(eval $(call gb_Module_add_moduledirs,cross_tail_build,\
	autodoc \
	basegfx \
	binaryurp \
	$(call gb_Helper_optional,BOOST,boost) \
	bridges \
	cli_ure \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,DESKTOP,codemaker) \
	comphelper \
	cosv \
	cppu \
	cppuhelper \
	$(call gb_Helper_optional,DESKTOP,helpcompiler) \
	i18npool \
	i18nutil \
	idl \
	idlc \
	io \
	javaunohelper \
	jurt \
	jvmaccess \
	jvmfwk \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	o3tl \
	offapi \
	officecfg \
	oovbaapi \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	registry \
	remotebridges \
	ridljar \
	rsc \
	sal \
	salhelper \
	sax \
	shell \
	stoc \
	store \
	tools \
	ucbhelper \
	$(call gb_Helper_optional,UCPP,ucpp) \
	udkapi \
	udm \
	unoil \
	unotest \
	unotools \
	ure \
	xmlhelp \
	xmlreader \
	$(call gb_Helper_optional,ZLIB,zlib) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
