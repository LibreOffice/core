# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sysui/productlist.mk

deb_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/deb)
deb_SRCDIR := $(SRCDIR)/sysui/desktop/debian

$(eval $(call gb_CustomTarget_CustomTarget,sysui/deb))

$(eval $(call gb_CustomTarget_register_targets,sysui/deb,\
$(foreach product,$(PRODUCTLIST),\
	$(product)-desktop-integration.tar.gz \
	$(product)/DEBIAN/control \
	$(product)/DEBIAN/postinst \
	$(product)/DEBIAN/postrm \
	$(product)/DEBIAN/prerm \
	$(product)$(PKGVERSIONSHORT)-debian-menus_$(PKGVERSION)-$(LIBO_VERSION_PATCH)_all.deb) \
))

$(deb_WORKDIR)/%-desktop-integration.tar.gz: $(deb_WORKDIR)/%$(PKGVERSIONSHORT)-debian-menus_$(PKGVERSION)-$(LIBO_VERSION_PATCH)_all.deb
	$(GNUTAR) -C $(deb_WORKDIR) -cf - $(notdir $<) | gzip > $@

$(deb_WORKDIR)/%/DEBIAN/postrm: $(deb_SRCDIR)/postrm
	cat $< | tr -d "\015" | \
		sed 's/%PREFIX/$(UNIXFILENAME.$*)/g' >> $@

$(deb_WORKDIR)/%/DEBIAN/postinst: $(deb_SRCDIR)/postinst
	cat $< | tr -d "\015" | \
		sed 's/%PREFIX/$(UNIXFILENAME.$*)/g' >> $@

$(deb_WORKDIR)/%/DEBIAN/prerm: $(deb_SRCDIR)/prerm
	cat $< | tr -d "\015" | \
		sed 's/%PREFIX/$(UNIXFILENAME.$*)/g' >> $@

$(deb_WORKDIR)/%/DEBIAN/control: $(deb_SRCDIR)/control $(call gb_CustomTarget_get_workdir,sysui/share)/%/create_tree.sh
	mkdir -p $(deb_WORKDIR)/$*/usr/lib/menu
	cd $(call gb_CustomTarget_get_workdir,sysui/share)/$* \
		&& DESTDIR=$(deb_WORKDIR)/$* \
		ICON_PREFIX=$(UNIXFILENAME.$*) \
		KDEMAINDIR=/usr \
		GNOMEDIR=/usr \
		./create_tree.sh
	sed $(deb_SRCDIR)/openoffice.org-debian-menus \
		-e 's/%PRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION)/' \
		-e 's/%PREFIX/$(UNIXFILENAME.$*)/' \
		-e 's/%ICONPREFIX/$(UNIXFILENAME.$*)/' \
		> $(deb_WORKDIR)/$*/usr/lib/menu/$*$(PKGVERSIONSHORT)
	echo "Package: $*$(PKGVERSIONSHORT)-debian-menus" >$@
	cat $< | tr -d "\015" | \
		sed 's/%productname/$(PRODUCTNAME.$*) $(PRODUCTVERSION)/' \
		>> $@
	echo "Version: $(PKGVERSION)-$(LIBO_VERSION_PATCH)" >>$@
	du -k -s $(deb_WORKDIR)/$* | awk -F ' ' '{ printf "Installed-Size: %s\n", $$1 ; }' >>$@

$(deb_WORKDIR)/%$(PKGVERSIONSHORT)-debian-menus_$(PKGVERSION)-$(LIBO_VERSION_PATCH)_all.deb: $(deb_WORKDIR)/%/DEBIAN/postrm $(deb_WORKDIR)/%/DEBIAN/postinst $(deb_WORKDIR)/%/DEBIAN/prerm $(deb_WORKDIR)/%/DEBIAN/control $(call gb_Library_get_target,getuid)

	chmod -R g-w $(deb_WORKDIR)/$*
	chmod a+rx $(deb_WORKDIR)/$*/DEBIAN \
		$(deb_WORKDIR)/$*/DEBIAN/pre* $(deb_WORKDIR)/$*/DEBIAN/post*
	chmod g-s $(deb_WORKDIR)/$*/DEBIAN
	LD_PRELOAD=$(call gb_Library_get_target,getuid) \
		   dpkg-deb --build $(deb_WORKDIR)/$* $@

# vim: set noet sw=4 ts=4:
