#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
from common.ConfigGroup import ConfigGroup

'''
A Class which describes the publishing arguments
in a session.
Each session can contain different publishers, which are configured
through such a CGPublish object.
'''

class CGPublish(ConfigGroup):

    cp_Publish = bool()
    cp_URL = str()
    cp_Username = str()
    password = str()

    def setURL(self, path):
        try:
            self.cp_URL = (self.root).getFileAccess().getURL(path)
            self.overwriteApproved = False
        except Exception, ex:
            ex.printStackTrace()

    def getURL(self):
        try:
            return (self.root).getFileAccess().getPath(self.cp_URL, None)
        except Exception, e:
            e.printStackTrace()
            return ""

    def ftpURL(self):
        pass
        #COMMENTED
        #return "ftp://" + self.cp_Username + ((self.password != None and self.password.length() > 0) ? ":" + self.password : "") + "@" + self.cp_URL.substring(7)
