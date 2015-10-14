# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sysui/productlist.mk

rpm_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/rpm)
rpm_SRCDIR := $(SRCDIR)/sysui/desktop

RPMDISTROS := freedesktop

define rpm_register_target
$(call gb_CustomTarget_get_target,sysui/rpm): $(rpm_WORKDIR)/$(1)/$(1)$(PKGVERSIONSHORT)-$(2)-menus-$(PKGVERSION).noarch.rpm

$(rpm_WORKDIR)/$(1)-desktop-integration.tar.gz: $(rpm_WORKDIR)/$(1)/$(1)$(PKGVERSIONSHORT)-$(2)-menus-$(PKGVERSION).noarch.rpm
$(rpm_WORKDIR)/$(1)/$(1)$(PKGVERSIONSHORT)-$(2)-menus-$(PKGVERSION).noarch.rpm : \
		$(rpm_SRCDIR)/$(2)/$(2)-menus.spec \
		$(call gb_CustomTarget_get_workdir,sysui/share)/$(1)/create_tree.sh \
		| $(rpm_WORKDIR)/$(1)/.dir
	$(call gb_Helper_print_on_error,\
		$(RPM) -bb $$< \
			--buildroot $(rpm_WORKDIR)/$(1)/$(2) \
			--define "_builddir $(call gb_CustomTarget_get_workdir,sysui/share)/$(1)" \
			--define "_rpmdir $(rpm_WORKDIR)/$(1)" \
			--define "_rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" \
			--define "productname $(PRODUCTNAME.$(1))" \
			--define "pkgprefix $(1)$(PKGVERSIONSHORT)" \
			--define "unixfilename $(UNIXFILENAME.$(1))" \
			--define "productversion $(PRODUCTVERSION)" \
			--define "iconprefix $(UNIXFILENAME.$(1))" \
			--define "version $(PKGVERSION)" \
			--define "__debug_install_post %nil" \
		, $$@.log \
	)
endef

$(eval $(call gb_CustomTarget_CustomTarget,sysui/rpm))


$(foreach product,$(PRODUCTLIST),\
$(foreach distro,$(RPMDISTROS),\
$(eval $(call rpm_register_target,$(product),$(distro)))))


$(eval $(call gb_CustomTarget_register_targets,sysui/rpm,\
$(foreach product,$(PRODUCTLIST),\
	$(product)-desktop-integration.tar.gz) \
))

$(rpm_WORKDIR)/%-desktop-integration.tar.gz:
	$(GNUTAR) -C $(rpm_WORKDIR)/$* -cf - $(foreach distro,$(RPMDISTROS),$*$(PKGVERSIONSHORT)-$(distro)-menus-$(PKGVERSION).noarch.rpm) | gzip > $@

# vim: set noet sw=4 ts=4:
