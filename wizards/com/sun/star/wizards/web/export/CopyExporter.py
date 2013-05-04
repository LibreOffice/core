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

from .AbstractExporter import AbstractExporter
from ...common.FileAccess import FileAccess

class CopyExporter(AbstractExporter):

    # @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
    def export(self, source, target, xmsf, task):
        try:
            task.advance(True)

            # this will open the document, and calculate the pages/slides number
            # in it.
            if (self.exporter.cp_PageType > 0):
                self.closeDocument(self.openDocument(source, xmsf), xmsf)

            task.advance(True)

            newTarget = FileAccess.connectURLs(
                    FileAccess.getParentDir(target), source.urlFilename)

            b = self.getFileAccess(xmsf).copy(source.cp_URL, newTarget)

            task.advance(True)

            self.calcFileSize(source, newTarget, xmsf)

            return b
        except Exception:
            traceback.print_exc()
            return False
