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

ExampleDataPilotSource
======================

This example shows how you can implement an external DataPilot
component.

Simple usage scenario:
======================
1. build and deploy the component
2. open a new spreadsheet document
3. select for example cell B:2
4. start the DataPilot dialog
   Data->DataPilot->Start...
5. select "External source/interface -> Ok
6. select the example DataPilot
   Use the drop down list and select the service
   "ExampleDataPilotSource" -> Ok
7. Place some number fields in the field areas
   For example:
   - select the "ones" field and drop it in the "Row Fields" area
   - repeat this step with the "tens" and "hundreds" field
   - select the "thousands" field and drop it in the "Column Fields"
     area.
   -> press Ok
8. you see a matrix with rows and columns for the selected fields
   and a value area with a sum up of all fields valid for this
   coordinate. For example cell F:21=1221 (ones=B:21=1, tens=c:21=2
   hundreds=D:21=2 and thousands=F:3=1)


