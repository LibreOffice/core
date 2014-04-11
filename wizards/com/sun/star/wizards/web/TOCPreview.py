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
from .Process import Process
from ..ui.event.Task import Task
from ..common.FileAccess import FileAccess
from ..common.Properties import Properties
from ..common.Desktop import Desktop
from ..common.UCB import UCB

# This class both copies necessary files to
# a temporary directory, generates a temporary TOC page,
# and opens the generated html document in a web browser,
# by default "index.html" (unchangeable).
# <br/>
# Since the files are both static and dynamic (some are always the same,
# while other change according to user choices)
# I divide this tasks to two: all necessary
# static files, which should not regularly update are copied upon
# instanciation.
# The TOC is generated in refresh(...);

class TOCPreview:

    def __init__(self, xmsf_, settings, res, tempDir_, _xFrame):
        self.xFrame = _xFrame
        self.xmsf = xmsf_
        self.resources = res
        self.fileAccess = FileAccess(self.xmsf)
        self.tempDir = tempDir_
        self.loadArgs = self.loadArgs(FileAccess.connectURLs(self.tempDir, "/index.html"))
        self.openHyperlink = Desktop.getDispatchURL(self.xmsf, ".uno:OpenHyperlink")
        self.xDispatch = Desktop.getDispatcher(self.xmsf, self.xFrame, "_top", self.openHyperlink)
        self.ucb = UCB(self.xmsf)

        Process.copyStaticImages(self.ucb, settings, self.tempDir)

    def refresh(self, settings):
        doc = settings.cp_DefaultSession.createDOM1()
        layout = settings.cp_DefaultSession.getLayout()
        task = Task("", "", 10000);
        Process.generate1(self.xmsf, layout, doc, self.fileAccess, self.tempDir, task)
        Process.copyLayoutFiles(self.ucb, self.fileAccess, settings, layout, self.tempDir)
        self.xDispatch.dispatch(self.openHyperlink, tuple(self.loadArgs))

    def loadArgs(self, url):
        props = Properties()
        props["URL"] = url
        return props.getProperties1()
