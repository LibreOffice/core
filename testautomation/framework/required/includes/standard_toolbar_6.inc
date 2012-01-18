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
'* short description : Open applications from New button on standardbar
'*
'\***************************************************************************

testcase tStandardBar_6

    printlog( "Open applications from New button on standardbar" )


    hUseImpressAutopilot( false )
    
    gApplication = "WRITER"
    
    printlog "Check the 'New'-menu-button in Standardbar in each application"
    printlog "- click on menu-button -> open a new document as same document-type"
    
    printlog "   - Writer"
    gApplication = "WRITER"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()
    
    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    hDestroyDocument()
    hDestroyDocument()
    
    printlog "   - Calc"
    gApplication = "CALC"
    hCreateDocument()
    
    kontext "standardbar"
    Neu.Click()
    
    kontext "documentcalc"
    DocumentCalc.TypeKeys( "Hallo" )
    
    hDestroyDocument()
    hDestroyDocument()
    
    printlog "   - Draw"
    gApplication = "DRAW"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()
    
    gMouseClick ( 50, 50 )
    
    hDestroyDocument()
    hDestroyDocument()
    
    printlog "   - Impress"
    gApplication = "IMPRESS"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()
    
    gMouseClick ( 50, 50 )
    
    hDestroyDocument()
    hDestroyDocument()
    
    printlog "   - HTML-document"
    gApplication = "HTML"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()

    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    hDestroyDocument()
    hDestroyDocument()
    
    printlog "   - Master-document"
    gApplication = "MASTERDOCUMENT"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()

    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )
    hDestroyDocument()
    
    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )
    hDestroyDocument()
    
    printlog "   - Math"
    gApplication = "MATH"
    hCreateDocument()

    kontext "standardbar"
    Neu.Click()
    
    SchreibenInMathdok "a over b"
    
    hDestroyDocument()
    hDestroyDocument()
    
endcase


