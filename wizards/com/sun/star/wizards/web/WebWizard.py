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
import traceback

from .WWD_Events import WWD_Events

# The last class in the WebWizard Dialog class hirarchy.
# Has no functionality, is just nice to have it instanciated.
class WebWizard(WWD_Events):

    def __init__(self, xmsf):
        super(WebWizard, self).__init__(xmsf)

    @classmethod
    def main(self):
        #Call the wizard remotely(see README)
        try:
            ConnectStr = \
                "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
            xLocMSF = Desktop.connect(ConnectStr)
            ww = WebWizard(xLocMSF)
            ww.show()
            ww.cleanup()
        except Exception as e:
            print ("Wizard failure exception " + str(type(e)) +
                   " message " + str(e) + " args " + str(e.args) +
                   traceback.format_exc())
