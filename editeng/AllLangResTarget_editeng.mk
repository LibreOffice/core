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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,editeng))

$(eval $(call gb_AllLangResTarget_set_reslocation,editeng,svx))

$(eval $(call gb_AllLangResTarget_add_srs,editeng, editeng/res))
$(eval $(call gb_SrsTarget_SrsTarget,editeng/res))

$(eval $(call gb_SrsTarget_set_include,editeng/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/editeng/inc \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,editeng/res,\
    editeng/source/accessibility/accessibility.src \
    editeng/source/editeng/editeng.src \
    editeng/source/items/page.src \
    editeng/source/items/svxitems.src \
    editeng/source/misc/lingu.src \
    editeng/source/outliner/outliner.src \
))

# vim: set noet sw=4 ts=4:
