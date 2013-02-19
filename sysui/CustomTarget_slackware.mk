# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

slack_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/slackware)
slack_SRCDIR := $(SRCDIR)/sysui/desktop/slackware

$(eval $(call gb_CustomTarget_CustomTarget,sysui/slackware))

$(eval $(call gb_CustomTarget_register_targets,sysui/slackware,\
	empty.tar \
$(foreach product,$(PRODUCTLIST),\
	$(product)/install/doinst.sh \
	$(product)/install/slack-desc \
	$(product)$(PRODUCTVERSION)-slackware-menus-$(PKGVERSION)-noarch-$(LIBO_VERSION_PATCH).tgz) \
))


$(slack_WORKDIR)/empty.tar:
	mkdir $(slack_WORKDIR)/empty
	$(GNUTAR) -C $(slack_WORKDIR)/empty --owner=root $(if $(filter-out MACOSX,$(OS_FOR_BUILD)),--group=root) --same-owner -cf $@ .

$(slack_WORKDIR)/%/install/doinst.sh: $(slack_SRCDIR)/update-script $(call gb_CustomTarget_get_workdir,sysui/share)/%/launcherlist
	echo "( cd etc; rm -rf $(UNIXFILENAME.$*) )" > $@
	echo "( cd etc; ln -snf /opt/$(UNIXFILENAME.$*) $(UNIXFILENAME.$*) )" >> $@
	echo "( cd usr/bin; rm -rf soffice )" >> $@
	echo "( cd usr/bin; ln -sf /etc/$(UNIXFILENAME.$*)/program/soffice soffice )" >> $@
	for i in `cat $(call gb_CustomTarget_get_workdir,sysui/share)/$*/launcherlist`; do \
	echo "(cd usr/share/applications; rm -rf $(UNIXFILENAME.$*)-$$i)" >> $@; \
	echo "(cd usr/share/applications; ln -sf /etc/$(UNIXFILENAME.$*)/share/xdg/$$i $(UNIXFILENAME.$*)-$$i)" >> $@; \
		done
	cat $< >> $@

$(slack_WORKDIR)/%/install/slack-desc: $(slack_SRCDIR)/slack-desc
	sed -e "s/PKGNAME/$*-slackware-menus/g" \
	-e "s/PKGVERSION/$(PKGVERSION)/g" \
	-e "s/LONGPRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION)/g" \
	-e "s/PRODUCTNAME/$(PRODUCTNAME.$*)/g" \
	-e "s/UNIXFILENAME/$(UNIXFILENAME.$*)/g" \
		$< > $@

$(slack_WORKDIR)/%$(PRODUCTVERSION)-slackware-menus-$(PKGVERSION)-noarch-$(LIBO_VERSION_PATCH).tgz: $(slack_WORKDIR)/empty.tar $(slack_WORKDIR)/%/install/doinst.sh $(slack_WORKDIR)/%/install/slack-desc $(call gb_CustomTarget_get_workdir,sysui/share)/%/create_tree.sh
	cd $(call gb_CustomTarget_get_workdir,sysui/share)/$* \
           && DESTDIR=$(slack_WORKDIR)/$* \
           ICON_PREFIX=$(UNIXFILENAME.$*) \
           KDEMAINDIR=/opt/kde \
           GNOMEDIR="" \
           ./create_tree.sh
	cp $(slack_WORKDIR)/empty.tar $@.tmp
	$(GNUTAR) -C $(slack_WORKDIR)/$* --owner=root $(if $(filter-out MACOSX,$(OS_FOR_BUILD)),--group=root) --same-owner \
		-rf $@.tmp install usr opt
	gzip < $@.tmp > $@

# vim: set noet sw=4 ts=4:
