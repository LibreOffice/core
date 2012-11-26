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
from wizards.common.ConfigGroup import *

'''
CGTopic means: Configuration Group Topic.
This object encapsulates a configuration group with topic information.
Since the topics gui conftrol uses its own data model, there is
also code here to convert from the data model to CGTopic object (the constructor)
and vice versa (setDataToRow method - used when loading the last session...)
'''

class CGTopic(ConfigGroup):

    cp_Index = int()
    cp_Topic = str()
    cp_Responsible = str()
    cp_Time = str()

    '''
    create a new CGTopic object with data from the given row.
    the row object is a PropertyValue array, as used
    by the TopicsControl's data model.
    @param row PropertyValue array as used by the TopicsControl data model.
    '''

    def __init__(self, row=None):
        if row is None:
            return
        num = row[0].Value
        CGTopic.cp_Index = int(row[0].Value[:-1])
        CGTopic.cp_Topic = row[1].Value
        CGTopic.cp_Responsible = row[2].Value
        CGTopic.cp_Time = row[3].Value

    '''
    copies the data in this CGTopic object
    to the given row.
    @param row the row object (PropertyValue array) to
    copy the data to.
    '''

    def setDataToRow(self, row):
        row[0].Value = "" + str(CGTopic.cp_Index) + "."
        row[1].Value = CGTopic.cp_Topic
        row[2].Value = CGTopic.cp_Responsible
        row[3].Value = CGTopic.cp_Time
