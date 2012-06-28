#**************************************************************
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
#**************************************************************

The module stores GUI testing scripts. It's an eclipse project. Setup the project with the following steps:
1. Open eclipse and then click menu "File -> Import...".
2. Select "General -> Existing Projects into Workspace", click "Next", and then select the parent directory of this module as root directory.
3. Choose "testcommon" and "testscript", and then click "Finish".
4. Download JUnit-4.10.jar (Version MUST be higher than 4.10) into testscript/output/lib/junit.jar and then refresh the project.