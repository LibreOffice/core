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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,pdffilter))

# this is ugly; why is it necessary to set this manually?
$(eval $(call gb_AllLangResTarget_set_reslocation,pdffilter,res))

$(eval $(call gb_AllLangResTarget_add_srs,pdffilter,\
	filter/pdffilter \
))

$(eval $(call gb_SrsTarget_SrsTarget,filter/pdffilter))

$(eval $(call gb_SrsTarget_set_include,filter/pdffilter,\
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_SrsTarget_add_files,filter/pdffilter,\
	filter/source/pdf/impdialog.src \
	filter/source/pdf/pdf.src \
))

# vim: set noet sw=4 ts=4:
