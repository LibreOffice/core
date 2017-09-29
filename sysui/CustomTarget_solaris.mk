# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sysui/productlist.mk

solaris_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/solaris)
solaris_SRCDIR := $(SRCDIR)/sysui/desktop/solaris

$(eval $(call gb_CustomTarget_CustomTarget,sysui/solaris))

$(eval $(call gb_CustomTarget_register_targets,sysui/solaris,\
$(foreach product,$(PRODUCTLIST),\
	$(product)/pkginfo \
	$(product)/depend \
	$(product)/mailcap \
	$(product)/copyright \
	$(product)/postinstall \
	$(product)/postremove \
	$(product)/prototype \
	$(product)-desktop-integration.tar.gz) \
))


$(solaris_WORKDIR)/%/pkginfo: $(solaris_SRCDIR)/pkginfo
	cat $< | tr -d "\015" | sed -e "s/%PRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION)/g"  -e "s/%pkgprefix/$*/g" > $@

$(solaris_WORKDIR)/%/depend: $(solaris_SRCDIR)/depend
	cat $< | tr -d "\015" | sed -e "s/%PRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION)/g"  -e "s/%pkgprefix/$*/g" > $@

$(solaris_WORKDIR)/%/mailcap: $(solaris_SRCDIR)/mailcap
	cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$*)/g" > $@

$(solaris_WORKDIR)/%/copyright: $(solaris_SRCDIR)/copyright
	cat $< | tr -d "\015" > $@

$(solaris_WORKDIR)/%/postinstall: $(solaris_SRCDIR)/postinstall
	cat $< | tr -d "\015" > $@

$(solaris_WORKDIR)/%/postremove: $(solaris_SRCDIR)/postremove
	cat $< | tr -d "\015" > $@

$(solaris_WORKDIR)/%/prototype: $(solaris_SRCDIR)/prototype
	cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$*)/g" -e "s_%SOURCE_$(call gb_CustomTarget_get_workdir,sysui/share)/$*_g" -e "s/%ICONPREFIX/$(UNIXFILENAME.$*)/g" > $@

$(solaris_WORKDIR)/%-desktop-integration.tar.gz: $(solaris_WORKDIR)/%/copyright $(solaris_WORKDIR)/%/pkginfo $(solaris_WORKDIR)/%/depend $(solaris_WORKDIR)/%/mailcap $(solaris_WORKDIR)/%/postinstall $(solaris_WORKDIR)/%/postremove $(solaris_WORKDIR)/%/prototype $(call gb_CustomTarget_get_workdir,sysui/share)/%/openoffice.org.xml
#	pkgmk -l 1073741824 -r $(solaris_WORKDIR) -f $(solaris_WORKDIR)/$*/prototype -o -d $(solaris_WORKDIR) ARCH=all VERSION=$(PKGVERSION.$*)
	$(GNUTAR) -cf - -C $(solaris_WORKDIR) $* | gzip > $@

# vim: set noet sw=4 ts=4:
