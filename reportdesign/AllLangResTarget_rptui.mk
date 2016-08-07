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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,rptui))

$(eval $(call gb_AllLangResTarget_set_reslocation,rptui,reportdesign))

$(eval $(call gb_AllLangResTarget_add_srs,rptui,reportdesign/rptui))

$(eval $(call gb_SrsTarget_SrsTarget,reportdesign/rptui))

$(eval $(call gb_SrsTarget_set_include,reportdesign/rptui,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(WORKDIR)/inc/reportdesign \
	-I$(SRCDIR)/reportdesign/source/ui/inc \
	-I$(SRCDIR)/reportdesign/inc \
))


$(eval $(call gb_SrsTarget_add_files,reportdesign/rptui,\
	reportdesign/source/ui/dlg/dlgpage.src	\
	reportdesign/source/ui/dlg/PageNumber.src	\
	reportdesign/source/ui/dlg/DateTime.src	\
	reportdesign/source/ui/dlg/CondFormat.src	\
	reportdesign/source/ui/dlg/Navigator.src	\
	reportdesign/source/ui/dlg/GroupsSorting.src \
	reportdesign/source/ui/inspection/inspection.src \
	reportdesign/source/ui/report/report.src \
))

# vim: set noet sw=4 ts=4:
