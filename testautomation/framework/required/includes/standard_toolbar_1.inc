'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

testcase tStandardBar_1

    printlog "Check the 'Standardbar' in all applications"

    gApplication = "WRITER"
    hNewDocument()
    kontext "DocumentWriter"
    DocumentWriter.maximize()
    hCloseDocument()

    Printlog " - WRITER: check if Standardbar exists"
    gApplication = "WRITER"
    hAccessStandardBar()

    Printlog " - CALC: check if Standardbar exists"
    gApplication = "CALC"
    hAccessStandardBar()

    Printlog " - IMPRESS: check if Standardbar exists"
    gApplication = "IMPRESS"
    hAccessStandardBar()

    Printlog " - DRAW: check if Standardbar exists"
    gApplication = "DRAW"
    hAccessStandardBar()

    Printlog " - HTML: check if Standardbar exists"
    gApplication = "HTML"
    hAccessStandardBar()

    Printlog " - MATH: check if Standardbar exists"
    gApplication = "MATH"
    hAccessStandardBar()

    Printlog " - GLOBALDOC: check if Standardbar exists"
    gApplication = "MASTERDOCUMENT"
    hAccessStandardBar()

    gApplication = "WRITER"


endcase


