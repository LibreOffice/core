# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,ucbhelper_inc,$(SRCDIR)/ucbhelper/inc))

$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/activedatasink.hxx,ucbhelper/activedatasink.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/activedatastreamer.hxx,ucbhelper/activedatastreamer.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/cancelcommandexecution.hxx,ucbhelper/cancelcommandexecution.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/commandenvironment.hxx,ucbhelper/commandenvironment.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/content.hxx,ucbhelper/content.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contenthelper.hxx,ucbhelper/contenthelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contentidentifier.hxx,ucbhelper/contentidentifier.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contentinfo.hxx,ucbhelper/contentinfo.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/fd_inputstream.hxx,ucbhelper/fd_inputstream.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/fileidentifierconverter.hxx,ucbhelper/fileidentifierconverter.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/interactionrequest.hxx,ucbhelper/interactionrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/interceptedinteraction.hxx,ucbhelper/interceptedinteraction.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/macros.hxx,ucbhelper/macros.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/propertyvalueset.hxx,ucbhelper/propertyvalueset.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/providerhelper.hxx,ucbhelper/providerhelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/proxydecider.hxx,ucbhelper/proxydecider.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/registerucb.hxx,ucbhelper/registerucb.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultset.hxx,ucbhelper/resultset.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultsethelper.hxx,ucbhelper/resultsethelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultsetmetadata.hxx,ucbhelper/resultsetmetadata.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleauthenticationrequest.hxx,ucbhelper/simpleauthenticationrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simplecertificatevalidationrequest.hxx,ucbhelper/simplecertificatevalidationrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleinteractionrequest.hxx,ucbhelper/simpleinteractionrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleioerrorrequest.hxx,ucbhelper/simpleioerrorrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simplenameclashresolverequest.hxx,ucbhelper/simplenameclashresolverequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/std_inputstream.hxx,ucbhelper/std_inputstream.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/std_outputstream.hxx,ucbhelper/std_outputstream.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/ucbhelperdllapi.h,ucbhelper/ucbhelperdllapi.h))

# vim: set noet sw=4 ts=4:
