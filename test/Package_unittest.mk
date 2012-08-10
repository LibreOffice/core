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
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
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

$(eval $(call gb_Package_Package,test_unittest,$(SRCDIR)/test/user-template))

$(eval $(call gb_Package_add_file,test_unittest,unittest/registry/modifications.xcd,registry/modifications.xcd))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/sl.dic,user/wordbook/sl.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/en-US.dic,user/wordbook/en-US.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/technical.dic,user/wordbook/technical.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/wordbook/en-GB.dic,user/wordbook/en-GB.dic))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/gallery/empty_directory,user/gallery/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/dtd/empty_directory,user/dtd/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/psprint/fontmetric/empty_directory,user/psprint/fontmetric/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/psprint/driver/empty_directory,user/psprint/driver/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/template.bau,user/autotext/en-US/template.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/standard.bau,user/autotext/en-US/standard.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autotext/en-US/crdbus50.bau,user/autotext/en-US/crdbus50.bau))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/psetup.xpm,user/config/psetup.xpm))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/soffice.cfg/simpress/transitions.xml,user/config/soffice.cfg/simpress/transitions.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/soffice.cfg/simpress/transitions-ogl.xml,user/config/soffice.cfg/simpress/transitions-ogl.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/soffice.cfg/simpress/effects.xml,user/config/soffice.cfg/simpress/effects.xml))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/soffice.cfg/modules/empty_directory,user/config/soffice.cfg/modules/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/config/psetupl.xpm,user/config/psetupl.xpm))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/autocorr/empty_directory,user/autocorr/empty_directory))
$(eval $(call gb_Package_add_file,test_unittest,unittest/user/template/empty_directory,user/template/empty_directory))

# vim: set noet sw=4 ts=4:
