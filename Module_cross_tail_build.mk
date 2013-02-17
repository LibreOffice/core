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
	bridges \
	cli_ure \
	codemaker \
	comphelper \
	cosv \
	cppu \
	cppuhelper \
	i18npool \
	i18nutil \
	idl \
	idlc \
	io \
	javaunohelper \
	jurt \
	jvmaccess \
	jvmfwk \
	o3tl \
	offapi \
	officecfg \
	oovbaapi \
	registry \
	remotebridges \
	ridljar \
	rsc \
	sal \
	salhelper \
	sax \
	setup_native \
	shell \
	stoc \
	store \
	tools \
	ucbhelper \
	udkapi \
	udm \
	unoil \
	unotest \
	unotools \
	ure \
	xmlreader \
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional_for_host,DESKTOP, \
		helpcompiler \
		l10ntools \
		xmlhelp \
	) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	$(call gb_Helper_optional,UCPP,ucpp) \
	$(call gb_Helper_optional,ZLIB,zlib) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
