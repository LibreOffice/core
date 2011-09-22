# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

import uno
import unohelper

from com.sun.star.task import XJobExecutor
from wizards.fax.FaxWizardDialogImpl import FaxWizardDialogImpl

# implement a UNO component by deriving from the standard unohelper.Base class
# and from the interface(s) you want to implement.
class CallWizard(unohelper.Base, XJobExecutor):
    def __init__(self, ctx):
        # store the component context for later use
        self.ctx = ctx

    def trigger(self, args):
        fw = FaxWizardDialogImpl(self.ctx.ServiceManager)
        fx.startWizard()

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()

g_ImplementationHelper.addImplementation( \
    CallWizard,                               # UNO object class
    "com.sun.star.wizards.fax.CallWizard", # implemenation name
    ("com.sun.star.task.Job",),)                 # list of implemented services
                                                 # (the only service)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
