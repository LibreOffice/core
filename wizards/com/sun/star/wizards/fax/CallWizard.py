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
import unohelper
import traceback

from .FaxWizardDialogImpl import FaxWizardDialogImpl

from com.sun.star.task import XJobExecutor

# implement a UNO component by deriving from the standard unohelper.Base class
# and from the interface(s) you want to implement.
class CallWizard(unohelper.Base, XJobExecutor):
    def __init__(self, ctx):
        # store the component context for later use
        self.ctx = ctx

    def trigger(self, args):
        try:
            fw = FaxWizardDialogImpl(self.ctx.ServiceManager)
            fw.startWizard(self.ctx.ServiceManager)
        except Exception as e:
            print ("Wizard failure exception " + str(type(e)) +
                   " message " + str(e) + " args " + str(e.args) +
                   traceback.format_exc())

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()

g_ImplementationHelper.addImplementation( \
    CallWizard,                               # UNO object class
    "com.sun.star.wizards.fax.CallWizard", # implemenation name
    ("com.sun.star.task.Job",),)                 # list of implemented services
                                                 # (the only service)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
