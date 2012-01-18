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
'*  short description : Invisible items on the standardbar
'*
'\******************************************************************************

testcase tStandardBar_4

    printlog( "Invisible items on the standardbar" )

    printlog( "Reset standardbar to defaults" )
    hResetStandardBar()

    gApplication = "WRITER"
    printlog( "Open one single Writer document" )
    hInitSingleDoc()

    kontext "DocumentWriter"
    DocumentWriter.maximize() ' Needed to avoid toolbar clipping on resolutions < 1024x768 or small windows.

    printlog( "Check item count on toolbar" )
    hStandardbarItemGetCount()

    printlog( "Load URL" )
    hStandardbarLoadUrl()

    printlog( "<NEW> dialog" )
    hStandardbarNewDialog()

    printlog( "<Save As>" )
    hStandardbarSaveAs()

    printlog( "Close all open documents" )
    hFileCloseAll()

endcase

    
