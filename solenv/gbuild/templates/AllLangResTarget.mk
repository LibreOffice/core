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



# RES is the name of the resource; its file name will be RES-LOCALE.res
# MODULE is the name of the code module the makefile is located in

$(eval $(call gb_AllLangResTarget_AllLangResTarget,RES))

# declaration of images folder (e.g. default_images/MODULE/res)
$(eval $(call gb_AllLangResTarget_set_reslocation,RES,MODULE))

# declaration of a resource intermediate file (srs)
$(eval $(call gb_AllLangResTarget_add_srs,RES, RES/res))
$(eval $(call gb_SrsTarget_SrsTarget,RES/res))

# add any additional include paths here
$(eval $(call gb_SrsTarget_set_include,RES/res,\
	$$(INCLUDE) \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,RES/res,\
))

# vim: set noet sw=4 ts=4:
