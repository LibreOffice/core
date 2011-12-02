#   -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#  
#   Version: MPL 1.1 / GPLv3+ / LGPLv3+
#  
#   The contents of this file are subject to the Mozilla Public License Version
#   1.1 (the "License"); you may not use this file except in compliance with
#   the License or as specified alternatively below. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
#  
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#  
#   Major Contributor(s):
#   [ Copyright (C) 2011 Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (initial developer) ]
#  
#   All Rights Reserved.
#  
#   For minor contributions see the git repository.
#  
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#   the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
#   in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
#   instead of those above.

.PHONY: id tags docs distro-pack-install install fetch clean-host clean-build bootstrap

id:
	@create-ids

tags:
	@create-tags

docs:
	@mkdocs.sh $(SRCDIR)/docs $(SOLARENV)/inc/doxygen.cfg

distro-pack-install: install
	$(SRCDIR)/bin/distro-install-clean-up
	$(SRCDIR)/bin/distro-install-desktop-integration
	$(SRCDIR)/bin/distro-install-sdk
	$(SRCDIR)/bin/distro-install-file-lists

$(SRCDIR)/src.downloaded: $(SRCDIR)/ooo.lst $(SRCDIR)/download
	$(if $(filter YES,$(DO_FETCH_TARBALLS)),@$(SRCDIR)/download $(SRCDIR)/ooo.lst)
	@touch $@

fetch: $(SRCDIR)/src.downloaded

# fixme: can we prevent these exports in the first place?
$(SRCDIR)/Env.Host.sh: autogen.lastrun configure.in ooo.lst.in set_soenv.in
	$(if $(filter reconfigure,$(gb_SourceEnvAndRecurse_STAGE)),$(SRCDIR)/autogen.sh,@echo "cannot reconfigure from within solarenv" && rm -f $(SRCDIR)/Env.Host.sh && exit 2)

$(WORKDIR)/bootstrap:
	@cd $(SRCDIR) && ./bootstrap
	@mkdir -p $(dir $@) && touch $@

bootstrap: $(WORKDIR)/bootstrap

ifeq ($(strip $(gb_PARTIALBUILD)),)

clean-host:
	rm -rf $(SRCDIR)/*/$(INPATH)
	rm -rf install

clean-build:
	if [ -f $(SRCDIR)/dmake/Makefile ] ; then $(GNUMAKE) -C $(SRCDIR)/dmake clean; fi
	rm -f solenv/*/bin/dmake*
	rm -rf $(SRCDIR)/*/$(INPATH_FOR_BUILD)

distclean: clean
	if [ -f $(SRCDIR)/dmake/Makefile ] ; then $(GNUMAKE) -C $(SRCDIR)/dmake clean; fi
	cd $(SRCDIR) && rm -rf Env.Host.sh aclocal.m4 autogen.lastrun autom4te.cache \
	bin/repo-list build_env config.log config.status configure \
	desktop/scripts/soffice.sh ooo.lst post_download post_download.log \
	set_soenv set_soenv.last set_soenv.stamp src.downloaded warn

install: build
	echo "Installing in $(INSTALLDIR)..." && \
	ooinstall "$(INSTALLDIR)" && \
	echo "" && \
	echo "Installation finished, you can now execute:" && \
	echo "$(INSTALLDIR)/program/soffice"

endif


# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
