#***************************************************************
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#***************************************************************

$(eval $(call gb_Module_Module,filter))

$(eval $(call gb_Module_add_targets,filter,\
	Configuration_filter \
	Jar_XSLTFilter \
	Jar_XSLTValidate \
	Library_PptImporter \
	Library_egi \
	Library_eme \
	Library_epb \
	Library_epg \
	Library_epp \
	Library_eps \
	Library_ept \
	Library_era \
	Library_eti \
	Library_exp \
	Library_filterconfig \
	Library_filtertracer \
	Library_flash \
	Library_icd \
	Library_icg \
	Library_idx \
	Library_ime \
	Library_ipb \
	Library_ipd \
	Library_ips \
	Library_ipt \
	Library_ipx \
	Library_ira \
	Library_itg \
	Library_iti \
	Library_msfilter \
	Library_odfflatxml \
	Library_pdffilter \
	Library_placeware \
	Library_svgfilter \
	Library_t602filter \
	Library_xmlfa \
	Library_xmlfd \
	Library_xsltdlg \
	Library_xsltfilter \
	Package_inc \
	Package_docbook \
	Package_filter_generated \
	Package_xslt \
))

# TODO
#$(eval $(call gb_Module_add_subsequentcheck_targets,filter,\
	JunitTest_filter_complex \
))

# vim: set noet sw=4 ts=4:
