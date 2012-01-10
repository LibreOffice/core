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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,productregistration))

$(eval $(call gb_AllLangResTarget_set_reslocation,productregistration,svtools))

$(eval $(call gb_AllLangResTarget_add_srs,productregistration,\
    svt/productregistration \
))

$(eval $(call gb_SrsTarget_SrsTarget,svt/productregistration))

$(eval $(call gb_SrsTarget_set_include,svt/productregistration,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc/ \
    -I$(SRCDIR)/svtools/inc/svtools \
))

$(eval $(call gb_SrsTarget_add_files,svt/productregistration,\
    svtools/source/productregistration/registrationdlg.src \
))


