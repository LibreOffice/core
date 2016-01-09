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

from .ProcessErrors import ProcessErrors
from .AbstractErrorHandler import AbstractErrorHandler
from .WebWizardConst import *

# used to interact error occurring when generating the
# web-site to the user.
# This class renders the different errors,
# replacing some strings from the resources with
# content of the given arguments, depending on the error
# that accured.
class ProcessErrorHandler(AbstractErrorHandler, ProcessErrors):

    FILENAME = "%FILENAME"
    URL = "%URL"
    ERROR = "%ERROR"
    resources = None

    def __init__(self, xmsf, peer, res):
        super(ProcessErrorHandler, self).__init__(xmsf, peer)
        self.resources = res

    def getMessageFor(self, ex, obj, ix, errType):
        if ix == ProcessErrors.ERROR_MKDIR:
            return self.resources.resErrDocExport.replace(obj.localFilename, self.FILENAME)
        elif ix == ProcessErrors.ERROR_EXPORT_MKDIR:
            return self.resources.resErrMkDir.replace(obj.localFilename, self.FILENAME)
        elif ix == ProcessErrors.ERROR_DOC_VALIDATE:
            return self.resources.resErrDocInfo.replace(obj.localFilename, self.FILENAME)
        elif ix == ProcessErrors.ERROR_EXPORT_IO:
            return self.resources.resErrExportIO.replace(obj.localFilename, self.FILENAME)
        elif ix == ProcessErrors.ERROR_EXPORT_SECURITY:
            return self.resources.resErrSecurity.replace(obj.localFilename, self.FILENAME)
        elif ix == ProcessErrors.ERROR_GENERATE_XSLT:
            return self.resources.resErrTOC
        elif ix == ProcessErrors.ERROR_GENERATE_COPY:
            return self.resources.resErrTOCMedia
        elif ix == ProcessErrors.ERROR_PUBLISH:
            return self.resources.resErrPublish.replace(self.URL, obj.cp_URL)
        elif (ix == ProcessErrors.ERROR_EXPORT or ix == ProcessErrors.ERROR_PUBLISH_MEDIA):
            return self.resources.resErrPublishMedia
        elif ix == ProcessErrors.ERROR_CLEANUP:
            return self.resources.resErrUnexpected
        else:
            return self.resources.resErrUnknown.replace("%ERROR", "{0}/{1}/{2!s}".format(ex.__class__.__name__, obj.__class__.__name__, ix))
