###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



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
