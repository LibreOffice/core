# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Tools we need to build for cross-compiling
gb_BUILD_TOOLS = \
	$(foreach executable, \
		bestreversemap \
		cfgex \
		cpp \
		cppumaker \
		gencoll_rule \
		genconv_dict \
		gendict \
		genindex_data \
		helpex \
		idxdict \
		javamaker \
		makedepend \
		propex \
		rsc \
		saxparser \
		stringex \
		svidl \
		transex3 \
		treex \
		uiex \
		ulfex \
		unoidl-check \
		unoidl-write \
		xrmex \
		$(call gb_Helper_optional_for_host,DESKTOP, \
			gengal \
			HelpIndexer \
			HelpLinker \
		) \
	,$(call gb_Executable_get_runtime_dependencies,$(executable))) \
	$(foreach executable, \
		xsltproc \
	,$(call gb_ExternalExecutable_get_dependencies,$(executable))) \
	$(call gb_Helper_optional_for_host,DESKTOP,Package_xmlhelp_xsl) \

# vim: set noet sw=4 ts=4:
