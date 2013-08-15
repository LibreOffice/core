# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,test_unittest,$(SRCDIR)/test/user-template))

$(eval $(call gb_Package_add_file,test_unittest,unittest/registry/modifications.xcd,registry/modifications.xcd))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/sl.dic,user/wordbook/sl.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/en-US.dic,user/wordbook/en-US.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/technical.dic,user/wordbook/technical.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/en-GB.dic,user/wordbook/en-GB.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/template.bau,user/autotext/en-US/template.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/standard.bau,user/autotext/en-US/standard.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/crdbus50.bau,user/autotext/en-US/crdbus50.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/psetup.xpm,user/config/psetup.xpm))
$(eval $(call gb_Package_add_file,test_unittest,unittest/install/share/config/soffice.cfg/simpress/transitions.xml,user/config/soffice.cfg/simpress/transitions.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/soffice.cfg/simpress/transitions-ogl.xml,user/config/soffice.cfg/simpress/transitions-ogl.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/install/share/config/soffice.cfg/simpress/effects.xml,user/config/soffice.cfg/simpress/effects.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/install/share/config/soffice.cfg/simpress/layoutlist.xml,user/config/soffice.cfg/simpress/layoutlist.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/psetupl.xpm,user/config/psetupl.xpm))

# vim: set noet sw=4 ts=4:
