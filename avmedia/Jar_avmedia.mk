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



$(eval $(call gb_Jar_Jar,avmedia,SRCDIR))

$(eval $(call gb_Jar_add_jars,avmedia,\
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/jmf.jar \
))

$(eval $(call gb_Jar_set_packageroot,avmedia,*.class x11))

$(eval $(call gb_Jar_set_manifest,avmedia,$(SRCDIR)/avmedia/source/java/manifest))

$(eval $(call gb_Jar_add_sourcefiles,avmedia,\
	avmedia/source/java/Manager \
	avmedia/source/java/Player \
	avmedia/source/java/PlayerWindow \
	avmedia/source/java/WindowAdapter \
	avmedia/source/java/MediaUno \
	avmedia/source/java/FrameGrabber \
	avmedia/source/java/MediaUno \
	avmedia/source/x11/SystemWindowAdapter
))

$(eval $(call gb_Jar_set_componentfile,avmedia,avmedia/source/java/avmedia.jar,URE))

# vim: set noet sw=4 ts=4:
