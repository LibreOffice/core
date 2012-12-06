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

DESCRIPTION_OOO_MIN_VER=3.5.0
DESCRIPTION_DEP_NAME=OpenOffice.org $(DESCRIPTION_OOO_MIN_VER)

EXTENSION_ID=org.apache.openoffice.framework.statusbar.StatusWordCount
EXTENSION_VERSION=0.0.1

DESCRIPTION_PUBLISHER_URL=http://www.openoffice.org/api
DESCRIPTION_PUBLISHER_NAME=Apache OpenOffice
DESCRIPTION_DISPLAY_NAME=Word Count StatusBar Control

DESCRIPTION_UPDATE_BASEURL=http://www.openoffice.org/api/dummyupdate
DESCRIPTION_UPDATE_URL=http://www.openoffice.org/api/dummyupdate/update.xml
UPDATE_DOWNLOAD_BASE=http://www.openoffice.org/api/dummydownload/
RELEASE_NOTES=http://www.openoffice.org/api/dummy_release_notes

PROTOCOLHANDLER_IMPLNAME=org.apache.openoffice.framework.statusbar.comp.StatusWordCount.ProtocolHandler
PROTOCOLHANDLER_SERVICE_NAME=org.apache.openoffice.framework.statusbar.StatusWordCount.ProtocolHandler
PROTOCOLHANDLER_PROTOCOL=org.apache.openoffice.framework.statusbar.StatusWordCount

STATUSBARCONTROLLER_IMPLNAME=org.apache.openoffice.framework.statusbar.comp.StatusWordCount.StatusbarController
STATUSBARCONTROLLER_SERVICE_NAME=org.apache.openoffice.framework.statusbar.StatusWordCount.StatusbarController
