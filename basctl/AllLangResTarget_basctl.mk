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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,basctl))

$(eval $(call gb_AllLangResTarget_add_srs,basctl,basctl/res))

$(eval $(call gb_SrsTarget_SrsTarget,basctl/res))

$(eval $(call gb_SrsTarget_set_include,basctl/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/source/inc \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,basctl/res,\
	basctl/source/basicide/basidesh.src \
	basctl/source/basicide/macrodlg.src \
	basctl/source/basicide/moptions.src \
	basctl/source/basicide/moduldlg.src \
	basctl/source/basicide/objdlg.src \
	basctl/source/basicide/brkdlg.src \
	basctl/source/basicide/basicprint.src \
	basctl/source/dlged/dlgresid.src \
	basctl/source/dlged/managelang.src \
))

# vim: set noet sw=4 ts=4:
