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


from .WebWizardConst import *
from ..common.IRenderer import IRenderer


# receives status calls from the status dialog which
# appears when the user clicks "create".
# allocates strings from the resources to
# display the current task status.
# (renders the state to resource strings)
class ProcessStatusRenderer(IRenderer):

    strings = {}

    def __init__(self, res):
        self.strings[TASK_EXPORT_DOCUMENTS] = res.resTaskExportDocs
        self.strings[TASK_EXPORT_PREPARE] = res.resTaskExportPrepare
        self.strings[TASK_GENERATE_COPY] = res.resTaskGenerateCopy
        self.strings[TASK_GENERATE_PREPARE] = res.resTaskGeneratePrepare
        self.strings[TASK_GENERATE_XSL] = res.resTaskGenerateXsl
        self.strings[TASK_PREPARE] = res.resTaskPrepare
        self.strings[LOCAL_PUBLISHER] = res.resTaskPublishLocal
        self.strings[ZIP_PUBLISHER] = res.resTaskPublishZip
        self.strings[FTP_PUBLISHER] = res.resTaskPublishFTP

        self.strings[TASK_PUBLISH_PREPARE] = res.resTaskPublishPrepare
        self.strings[TASK_FINISH] = res.resTaskFinish

    def render(self, object):
        return self.strings[object]

