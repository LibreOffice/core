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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,analysis))

$(eval $(call gb_AllLangResTarget_set_reslocation,analysis,scaddins))

$(eval $(call gb_AllLangResTarget_add_srs,analysis,\
	analysis/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,analysis/res))

$(eval $(call gb_SrsTarget_set_include,analysis/res,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(SRCDIR)/scaddins/source/analysis \
))

$(eval $(call gb_SrsTarget_add_files,analysis/res,\
	scaddins/source/analysis/analysis.src \
	scaddins/source/analysis/analysis_deffuncnames.src \
	scaddins/source/analysis/analysis_funcnames.src \
))
