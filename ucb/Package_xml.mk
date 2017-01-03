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



$(eval $(call gb_Package_Package,ucb_xml,$(SRCDIR)/ucb/source))

$(eval $(call gb_Package_add_file,ucb_xml,xml/cached.xml,cacher/cached.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/srtrs.xml,sorter/srtrs.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucb.xml,core/ucb.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpfile.xml,ucp/file/ucpfile.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpftp.xml,ucp/ftp/ucpftp.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpgio.xml,ucp/gio/ucpgio.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpgvfs.xml,ucp/gvfs/ucpgvfs.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucphier.xml,ucp/hierarchy/ucphier.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucppkg.xml,ucp/package/ucppkg.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucptdoc.xml,ucp/tdoc/ucptdoc.xml))
$(eval $(call gb_Package_add_file,ucb_xml,xml/ucpdav.xml,ucp/webdav/ucpdav.xml))
