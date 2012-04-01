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

$(eval $(call gb_Module_Module,officecfg))

$(eval $(call gb_Module_add_targets,officecfg,\
    CustomTarget_registry \
    Package_cppheader \
    Package_misc \
    Package_tools \
    Configuration_officecfg \
))

# this does not work, subsequenttest s a runtime dep, that is it postpone the run of the unittest, not the build of it
# and that test need a bunch of include that are not there and there is no rule to teach make about the
# dependency of these generated headers
# disabled for now
# The cppheader test depends on comphelper so can only be a subsequentcheck:
#$(eval $(call gb_Module_add_subsequentcheck_targets,officecfg,\
#    CppunitTest_officecfg_cppheader_test \
#))

# vim: set noet sw=4 ts=4:
