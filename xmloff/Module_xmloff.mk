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



$(eval $(call gb_Module_Module,xmloff))

$(eval $(call gb_Module_add_targets,xmloff,\
    Library_xo \
    Library_xof \
    Package_dtd \
    Package_inc \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,xmloff,\
    JunitTest_xmloff_unoapi \
))

#todo: map file
#todo: xmlkywd.lib - built, but never used?!

# vim: set noet ts=4 sw=4:
