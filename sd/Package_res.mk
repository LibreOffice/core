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



$(eval $(call gb_Package_Package,sd_res,$(SRCDIR)/sd/res))

$(eval $(call gb_Package_add_file,sd_res,pck/common.inc,webview/common.inc))
$(eval $(call gb_Package_add_file,sd_res,pck/common.pl,webview/common.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/edit.asp,webview/edit.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/edit.pl,webview/edit.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/editpic.asp,webview/editpic.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/editpic.pl,webview/editpic.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/index.pl,webview/index.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/poll.asp,webview/poll.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/poll.pl,webview/poll.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/savepic.asp,webview/savepic.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/savepic.pl,webview/savepic.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/show.asp,webview/show.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/show.pl,webview/show.pl))
$(eval $(call gb_Package_add_file,sd_res,pck/webcast.asp,webview/webview.asp))
$(eval $(call gb_Package_add_file,sd_res,pck/webcast.pl,webview/webview.pl))

$(eval $(call gb_Package_add_file,sd_res,pck/glas-blue.zip,buttons/glas-blue.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/glas-green.zip,buttons/glas-green.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/glas-red.zip,buttons/glas-red.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/round-gorilla.zip,buttons/round-gorilla.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/round-white.zip,buttons/round-white.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/simple.zip,buttons/simple.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/square-blue.zip,buttons/square-blue.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/square-gray.zip,buttons/square-gray.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/square-green.zip,buttons/square-green.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/square-red.zip,buttons/square-red.zip))
$(eval $(call gb_Package_add_file,sd_res,pck/square-yellow.zip,buttons/square-yellow.zip))
