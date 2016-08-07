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



$(eval $(call gb_Package_Package,linguistic_inc,$(SRCDIR)/linguistic/inc))

# add files to the package
# SOURCE_RELPATH/file.ext is the name of the source file (relative to the root give above)
# DESTINATION_RELPATH/file.ext is the destination file (relative to $(OUTDIR))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngdllapi.h,linguistic/lngdllapi.h))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/hyphdta.hxx,linguistic/hyphdta.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngprophelp.hxx,linguistic/lngprophelp.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/lngprops.hxx,linguistic/lngprops.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/misc.hxx,linguistic/misc.hxx))
$(eval $(call gb_Package_add_file,linguistic_inc,inc/linguistic/spelldta.hxx,linguistic/spelldta.hxx))

# vim: set noet sw=4 ts=4:
