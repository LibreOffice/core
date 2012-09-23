# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

cli_ure_source_MAKEFILE := $(lastword $(MAKEFILE_LIST))

# FIXME move this to configure.in ?
cli_ure_CCNUMVER = $(shell PATH='$(PATH)' $(CXX) | $(gb_AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk)
cli_ure_CCNUMVER_GOOD = $(shell expr $(cli_ure_CCNUMVER) '>=' 001399999999)

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CustomTarget_CustomTarget,cli_ure/source))

$(call gb_CustomTarget_get_target,cli_ure/source) : \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/assembly.cs \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/native/assembly.cxx \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/assembly.cs

$(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/assembly.cs : \
	$(SRCDIR)/cli_ure/source/basetypes/assembly.cs \
	$(SRCDIR)/cli_ure/version/version.txt \
	$(cli_ure_source_MAKEFILE) \
	| $(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/.dir

$(call gb_CustomTarget_get_workdir,cli_ure/source)/native/assembly.cxx : \
	$(SRCDIR)/cli_ure/source/native/assembly.cxx \
	$(SRCDIR)/cli_ure/version/version.txt \
	$(cli_ure_source_MAKEFILE) \
	| $(call gb_CustomTarget_get_workdir,cli_ure/source)/native/.dir

$(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/assembly.cs : \
	$(SRCDIR)/cli_ure/source/ure/assembly.cs \
	$(SRCDIR)/cli_ure/version/version.txt \
	$(cli_ure_source_MAKEFILE) \
	| $(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/.dir

$(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/assembly.cs :
	$(GNUCOPY) $< $@.tmp && \
	echo '[assembly:System.Reflection.AssemblyVersion( "$(CLI_BASETYPES_NEW_VERSION)" )]' >> $@.tmp && \
	$(if $(cli_ure_CCNUMVER_GOOD),echo '[assembly:System.Reflection.AssemblyKeyFile( @"$(call gb_Helper_windows_path,$(OUTDIR)/bin/cliuno.snk)" )]' >> $@.tmp &&) \
	mv $@.tmp $@

# TODO use macros for this
$(call gb_CustomTarget_get_workdir,cli_ure/source)/native/assembly.cxx :
	$(GNUCOPY) $< $@.tmp && \
	echo '[assembly:System::Reflection::AssemblyVersion( "$(CLI_CPPUHELPER_NEW_VERSION)" )];' >> $@.tmp && \
	echo '[assembly:System::Reflection::AssemblyKeyFile( "$(call gb_Helper_windows_path,$(OUTDIR)/bin/cliuno.snk)" )];' >> $@.tmp && \
	mv $@.tmp $@

$(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/assembly.cs :
	$(GNUCOPY) $< $@.tmp && \
	echo '[assembly:System.Reflection.AssemblyVersion( "$(CLI_URE_NEW_VERSION)" )]' >> $@.tmp && \
	$(if $(cli_ure_CCNUMVER_GOOD),echo '[assembly:System.Reflection.AssemblyKeyFile( @"$(call gb_Helper_windows_path,$(OUTDIR)/bin/cliuno.snk)" )]' >> $@.tmp &&) \
	mv $@.tmp $@

# vim: set noet sw=4 ts=4:
