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



### FIXME: when we have a real ZipTarget mechanism in gbuild replace this mess
$(eval $(call gb_Package_Package,vcl_zip,))
$(eval $(call gb_Package_add_file,vcl_zip,bin/osxres.zip,$(WORKDIR)/Misc/osxres.zip))

/$(WORKDIR)/Misc/osxres.zip: $(SRCDIR)/vcl/aqua/source/res/MainMenu.nib/*.nib $(SRCDIR)/vcl/aqua/source/res/cursors/*.png
    cd $(SRCDIR)/vcl/aqua/source/res ; \
    mkdir -p $(dir $@) ; \
    zip $@ MainMenu.nib/*.nib cursors/*.png

# vim: set noet sw=4 ts=4:
