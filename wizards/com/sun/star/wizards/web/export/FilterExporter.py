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

from .AbstractExporter import AbstractExporter
from ...common.Properties import Properties

from com.sun.star.io import IOException

# An exporter which is configured with a filter name, and
# uses the specified filter to export documents.
class FilterExporter(AbstractExporter):

    filterName = ""
    props = Properties()

    def __init__(self, exporter_):
        super(FilterExporter, self).__init__(exporter_)
        self.filterName = self.getArgument("Filter", exporter_)

    # (non-Javadoc)
    # @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
    def export(self, source, target, xmsf, task):
        result = True
        document = None

        try:
            document = self.openDocument(source, xmsf)
            task.advance(True)
            self.storeToURL1(document, target, self.filterName, self.props.getProperties1())
            task.advance(True)
        except IOException as iox:
            iox.printStackTrace(System.err)
            result = False
            raise iox
        finally:
            self.closeDocument(document, xmsf)
            self.calcFileSize(source, target, xmsf)
            task.advance(True);
        return result
