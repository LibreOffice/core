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
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGSession;

from .ConfiguredExporter import ConfiguredExporter

class ImpressHTMLExporter(ConfiguredExporter):

    SMALL_IMAGE = 512
    MEDIUM_IMAGE = 640
    LARGE_IMAGE = 800

    def export(source, targetDirectory, xmsf, task):

        # here set some filter specific properties.
        # other properties, which are not dependent on
        # user input are set through the exporter
        # configuration.

        session = self.getSession(source)

        self.props["Author"] = source.cp_Author
        self.props["Email"] = session.cp_GeneralInfo.cp_Email
        self.props["HomepageURL"] = self.getHomepageURL(session)
        self.props["UserText"] = self.source.cp_Title

        props[PropertyNames.PROPERTY_WIDTH] = self.getImageWidth(session)

        props["UseButtonSet"] = int(session.cp_Design.cp_IconSet)


        # now export
        return super(ImpressHTMLExporter, self).export(source, targetDirectory, xmsf, task)


    def getHomepageURL(session):
        return "../" + "../index.html" if (exporter.cp_OwnDirectory) else  "index.html"

    def getImageWidth(session):
        size = session.cp_Design.cp_OptimizeDisplaySize
        if (size ==  0):
            return self.SMALL_IMAGE
        elif (size ==  1):
            return self.MEDIUM_IMAGE
        elif (size ==  2):
            return self.LARGE_IMAGE
        return self.MEDIUM_IMAGE

    def getSession(doc):
        return doc.getSettings().cp_DefaultSession
