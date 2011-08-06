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
'* Owner : wolfram.garten@oracle.com
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
