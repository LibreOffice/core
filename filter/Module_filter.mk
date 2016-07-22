#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Module_Module,filter))

$(eval $(call gb_Module_add_targets,filter,\
	Configuration_filter \
	CustomTarget_svg \
    Library_gie \
	Library_filterconfig \
	Library_flash \
    Library_icg \
	Library_msfilter \
	Library_odfflatxml \
	Library_pdffilter \
	Library_placeware \
	Library_storagefd \
	Library_svgfilter \
	Library_graphicfilter \
	Library_t602filter \
	Library_textfd \
	Library_xmlfa \
	Library_xmlfd \
	Library_xsltdlg \
	Library_xsltfilter \
	Package_docbook \
	Package_xhtml \
	Package_xslt \
))

$(eval $(call gb_Module_add_l10n_targets,filter,\
	AllLangResTarget_eps \
	AllLangResTarget_pdffilter \
	AllLangResTarget_t602filter \
	AllLangResTarget_xsltdlg \
	UIConfig_xsltdlg \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,filter,\
	Executable_svg2odf \
))
endif

$(eval $(call gb_Module_add_check_targets,filter,\
    CppunitTest_filter_xslt \
    CppunitTest_filter_priority \
    CppunitTest_filter_msfilter \
))

ifneq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_Module_add_check_targets,filter,\
    CppunitTest_filter_dxf_test \
    CppunitTest_filter_eps_test \
    CppunitTest_filter_met_test \
    CppunitTest_filter_pcd_test \
    CppunitTest_filter_pcx_test \
    CppunitTest_filter_pict_test \
    CppunitTest_filter_ppm_test \
    CppunitTest_filter_psd_test \
    CppunitTest_filter_ras_test \
    CppunitTest_filter_tiff_test \
    CppunitTest_filter_tga_test \
))
endif

# TODO
#$(eval $(call gb_Module_add_subsequentcheck_targets,filter,\
	JunitTest_filter_complex \
))

$(eval $(call gb_Module_add_slowcheck_targets,filter,\
    CppunitTest_filter_dialogs_test \
))

# vim: set noet sw=4 ts=4:
