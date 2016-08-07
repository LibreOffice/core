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



$(eval $(call gb_UnoApiTarget_UnoApiTarget,types))

$(eval $(call gb_UnoApiTarget_add_merge_rdbfiles,types,\
    udkapi \
    offapi \
))

$(eval $(call gb_UnoApiTarget_add_reference_rdbfile,types,\
    offapi/type_reference/types \
))

# ugly hack for now!
statistic : $(gb_Helper_MISC)/api_statistic

$(gb_Helper_MISC)/api_statistic: $(call gb_UnoApiTarget_get_target,types)
	$(PERL) util/checknewapi.pl $(call gb_UnoApiTarget_get_target,types) $(UNOAPI_REFERENCE_types) "$(RSCREVISION)" "$(gb_UnoApiTarget_REGVIEWTARGET)" > $@

# vim: set noet sw=4 ts=4:
