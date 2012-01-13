'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'* short description : Impress Required Test Library (5)
'*
'\*****************************************************************

sub im_005_

    call tiFormatModifyLayout()

end sub

'*******************************************************************************

testcase tiFormatModifyLayout

    printlog "open application "
    Call  hNewDocument
    printlog( "Format/Page" )
    FormatPage
    Kontext "SeitenLayout"
    if ( SeitenLayout.exists( 2 ) ) then
        DialogTest ( SeitenLayout )
        printlog "cancel dialog 'Page Setup' "
        hCloseDialog( SeitenLayout, "Cancel" )
    else
        warnlog( "Dialog <SeitenLayout> did not open" )
    endif
    printlog "close application"
    Call hCloseDocument

endcase 'tiFormatModifyLayout
