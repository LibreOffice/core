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



$(eval $(call gb_Module_Module,wizards))

$(eval $(call gb_Module_add_targets,wizards,\
	AllLangResTarget_cal \
	AllLangResTarget_dbw \
	AllLangResTarget_eur \
	AllLangResTarget_imp \
	AllLangResTarget_tpl \
	AllLangResTarget_wwz \
	AllLangResTarget_wzi \
	Jar_commonwizards \
	Jar_agenda \
	Jar_fax \
	Jar_form \
	Jar_letter \
	Jar_query \
	Jar_report \
	Jar_reportbuilder \
	Jar_table \
	Jar_web \
	Zip_depot \
	Zip_euro \
	Zip_form \
	Zip_gimmicks \
	Zip_import \
	Zip_launcher \
	Zip_schedule \
	Zip_share \
	Zip_standard \
	Zip_template \
	Zip_tools \
	Zip_tutorials \
	Zip_usr \
	Zip_web \
))

# vim: set noet sw=4 ts=4:
