# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************

ExampleDataPilotSource
======================

This example shows how you can implement an external DataPilot
component.

Simple usage scenario:
======================
1. build and deploay the component
2. open a new spreadsheet document
3. select for example cell B:2
4. start the DataPilot dialog
   Data->DataPilot->Start...
5. select "External source/interface -> Ok
6. select the example DataPilot 
   Use the the drop down list and select the service
   "ExampleDataPilotSource" -> Ok
7. Place some number fields in the field areas
   For example: 
   - select the "ones" field and drop it in the "Row Fields" area
   - repeat this step with the "tens" and "hundreds" field
   - select the "thousands" field and drop it in the "Column Fields" 
     area.
   -> press Ok
8. you see a matrix with rows and colums for the selected fields
   and a value area with a sum up of all fields valid for this 
   coordinate. For example cell F:21=1221 (ones=B:21=1, tens=c:21=2
   hundreds=D:21=2 and thousands=F:3=1)

     
