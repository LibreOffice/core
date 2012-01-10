#**************************************************************
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
#**************************************************************



$(eval $(call gb_AllLangResTarget_AllLangResTarget,fwe))

$(eval $(call gb_AllLangResTarget_set_reslocation,fwe,framework))

$(eval $(call gb_AllLangResTarget_add_srs,fwe,\
    fwe/fwk_classes \
    fwe/fwk_services \
))


$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_classes))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_classes,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/classes \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_classes,\
    framework/source/classes/resource.src \
))

$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_services))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_services,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/services \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_services,\
    framework/source/services/fwk_services.src \
))
# vim: set noet sw=4 ts=4:
