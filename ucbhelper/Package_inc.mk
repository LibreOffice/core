#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,ucbhelper_inc,$(SRCDIR)/ucbhelper/inc))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contentbroker.hxx,ucbhelper/contentbroker.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleauthenticationrequest.hxx,ucbhelper/simpleauthenticationrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/propertyvalueset.hxx,ucbhelper/propertyvalueset.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contentinfo.hxx,ucbhelper/contentinfo.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultsetmetadata.hxx,ucbhelper/resultsetmetadata.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contenthelper.hxx,ucbhelper/contenthelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/activedatasink.hxx,ucbhelper/activedatasink.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/commandenvironment.hxx,ucbhelper/commandenvironment.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/interceptedinteraction.hxx,ucbhelper/interceptedinteraction.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultset.hxx,ucbhelper/resultset.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/ucbhelperdllapi.h,ucbhelper/ucbhelperdllapi.h))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleinteractionrequest.hxx,ucbhelper/simpleinteractionrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simpleioerrorrequest.hxx,ucbhelper/simpleioerrorrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/interactionrequest.hxx,ucbhelper/interactionrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/activedatastreamer.hxx,ucbhelper/activedatastreamer.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/content.hxx,ucbhelper/content.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simplecertificatevalidationrequest.hxx,ucbhelper/simplecertificatevalidationrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/proxydecider.hxx,ucbhelper/proxydecider.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/fileidentifierconverter.hxx,ucbhelper/fileidentifierconverter.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/cancelcommandexecution.hxx,ucbhelper/cancelcommandexecution.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/commandenvironmentproxy.hxx,ucbhelper/commandenvironmentproxy.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/registerucb.hxx,ucbhelper/registerucb.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/resultsethelper.hxx,ucbhelper/resultsethelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/configurationkeys.hxx,ucbhelper/configurationkeys.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/contentidentifier.hxx,ucbhelper/contentidentifier.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/handleinteractionrequest.hxx,ucbhelper/handleinteractionrequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/providerhelper.hxx,ucbhelper/providerhelper.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/simplenameclashresolverequest.hxx,ucbhelper/simplenameclashresolverequest.hxx))
$(eval $(call gb_Package_add_file,ucbhelper_inc,inc/ucbhelper/macros.hxx,ucbhelper/macros.hxx))

# vim: set noet sw=4 ts=4:
