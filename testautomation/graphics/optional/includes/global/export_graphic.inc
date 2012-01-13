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

*****

function hInsertGraphicsToNewFile( sOutputFile as string )
    sleep (1)
    if ( FileExists( sOutputFile ) ) then
        hFileReOpen( "" )
        Call hGrafikEinfuegen( sOutputFile )
    else
        warnlog( "File not saved: " & sOutputFile )
    endif

end function

'*******************************************************************************

function hFileReOpen( cFile as string ) as boolean

    dim iDocumentCount as integer : iDocumentCount = getDocumentCount
    hFileReopen() = false

    hCloseDocument()

    if ( cFile = "" ) then
        hNewDocument()
    else
        if ( FileExists( cFile ) ) then
            hFileOpen( cFile )
        else
            warnlog( "hFileReOpen(): File does not exist: " & cFile )
        endif
    endif

    if ( getDocumentCount = iDocumentCount ) then hFileReOpen() = true

end function

'*******************************************************************************

function hWaitForOutputFile( sOutputFile as string ) as boolean

    const FILE_WRITE_TIMEOUT = 30
    dim iWait as integer : iWait = 0

    for iWait = 1 to FILE_WRITE_TIMEOUT

        ' File found
        if ( FileExists( sOutputFile ) ) then
            hWaitForOutputFile() = true
            exit function
        endif

        ' Timeout
        if ( iWait = FILE_WRITE_TIMEOUT ) then
            hWaitForOutputFile() = false
            exit function
        endif

        wait( 1000 )

    next iWait

end function
