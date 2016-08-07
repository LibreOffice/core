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



$(eval $(call gb_Library_Library,xcr))

$(eval $(call gb_Library_add_package_headers,xcr,xmlscript_inc))

$(eval $(call gb_Library_set_componentfile,xcr,xmlscript/util/xcr))

$(eval $(call gb_Library_add_api,xcr,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,xcr,\
	-I$(SRCDIR)/xmlscript/source/inc \
	-I$(SRCDIR)/xmlscript/inc/xmlscript \
	-I$(SRCDIR)/xmlscript/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,xcr,\
	-DXCR_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,xcr,\
	sal \
	cppu \
	cppuhelper \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,xcr,\
	xmlscript/source/xmldlg_imexp/xmldlg_impmodels \
	xmlscript/source/xmldlg_imexp/xmldlg_import \
	xmlscript/source/xmldlg_imexp/xmldlg_addfunc \
	xmlscript/source/xmldlg_imexp/xmldlg_export \
	xmlscript/source/xmldlg_imexp/xmldlg_expmodels \
	xmlscript/source/misc/unoservices \
	xmlscript/source/xml_helper/xml_byteseq \
	xmlscript/source/xml_helper/xml_impctx \
	xmlscript/source/xml_helper/xml_element \
	xmlscript/source/xmlflat_imexp/xmlbas_import \
	xmlscript/source/xmlflat_imexp/xmlbas_export \
	xmlscript/source/xmllib_imexp/xmllib_import \
	xmlscript/source/xmllib_imexp/xmllib_export \
	xmlscript/source/xmlmod_imexp/xmlmod_import \
	xmlscript/source/xmlmod_imexp/xmlmod_export \
))

# vim: set noet sw=4 ts=4:
