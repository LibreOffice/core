'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : owner : wolfram.garten@oracle.com
'*
'* short description : import presentations with shapes in .ppt format
'*
'************************************************************************
'*
' #1  t_import_shapes     ' Load all shapes with .ppt format one time
'*
'\***********************************************************************

testcase t_import_shapes
    dim i as integer
    dim iOldState as integer
    dim iDocuments as integer
    dim lDocuments(100) as string
    dim sPage as string

    iOldState = hSetMacroSecurity ( 0 )
    iDocuments = GetFileList(convertPath(gTesttoolPath + "graphics/required/input/shapes"),"*.ppt",lDocuments())
    for i = 1 to iDocuments
        printlog "(" + i + "/" + iDocuments + "): " + lDocuments(i)
        printlog "------------------------------------------------------"
        hFileOpen(lDocuments(i))
        
        ' check if the document is writable
        if fIsDocumentWritable = false then
            ' make the document writable and check if it's succesfull 
            if fMakeDocumentWritable = false then
                warnlog "The document can't be make writeable. Test stopped."
                goto endsub
            endif
        endif

        kontext "DocumentImpress"
        DocumentImpress.typeKeys("<home>")
        while (sPage <> DocumentImpress.StatusGetText(DocumentImpress.StatusGetItemID(6)))
            printlog "------------"
            sleep 1
            DocumentImpress.typeKeys("<tab>")
            sleep 1 ' loop while empty: break after 10 minutes
            DocumentImpress.typeKeys("<tab>")
            FormatPositionAndSize
                kontext
                active.setpage TabPositionAndSize
                kontext "TabPositionAndSize"
                printlog "w: '" + Width.getText + "'; h: '" + Height.getText + "'"
            TabPositionAndSize.cancel
            kontext "DocumentImpress"
            DocumentImpress.typeKeys("<F2>")
            editSelectAll
            editCopy
            printlog getClipboard
            sPage = DocumentImpress.StatusGetText(DocumentImpress.StatusGetItemID(6))
            printlog sPage
            DocumentImpress.typeKeys("<escape><pageDown>")
            sleep 5
        wend
        hCloseDocument
    next i
hSetMacroSecurity ( iOldState )
endcase

