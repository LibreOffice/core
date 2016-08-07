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



$(eval $(call gb_Zip_Zip,basicsrvdepot,$(SRCDIR)/wizards/source/depot))

$(eval $(call gb_Zip_add_files,basicsrvdepot,\
	CommonLang.xba \
	Currency.xba \
	Depot.xba \
	dialog.xlb \
	Dialog2.xdl \
	Dialog3.xdl \
	Dialog4.xdl \
	Internet.xba \
	Lang_de.xba \
	Lang_en.xba \
	Lang_es.xba \
	Lang_fr.xba \
	Lang_it.xba \
	Lang_ja.xba \
	Lang_ko.xba \
	Lang_sv.xba \
	Lang_tw.xba \
	Lang_zh.xba \
	script.xlb \
	tools.xba \
))

# vim: set noet sw=4 ts=4:
