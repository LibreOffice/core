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

sub CreatePathList

    Dim i as Integer, iNum as Integer
    Dim sType as String, sVario as String, sList as String
    Dim lsInterim ( 50 ) as String
    '///create the comparison list for path-options
    '///+[TesttoolPath]\framework\options\input\paths_[LanguageCode].txt
    sList = ConvertPath ( gTesttoolPath + "framework\optional\input\options\paths_" + iSprache + ".txt" )
    call hNewDocument()
    ToolsOptions
    hToolsOptions ( "StarOffice", "Paths" )

    for i=1 to Typ.GetItemCount

        Kontext "TabPfade"

        if i=1 then
            Typ.TypeKeys "<Down>"
            Typ.TypeKeys "<Home>"
        else
            Typ.TypeKeys "<Down>"
        end if

        sType = Typ.GetSelText

        if Bearbeiten.IsEnabled then
            Bearbeiten.Click

            Kontext "OeffnenDlg"

            if OeffnenDlg.Exists then
                sVario = 1
                iNum = 1
                OeffnenDlg.Cancel
            end if

            Kontext "PfadeAuswaehlen"
            if PfadeAuswaehlen.Exists then
                sVario = 2
                iNum = Pfade.GetItemCount
                PfadeAuswaehlen.Cancel
            end if

        else
            sVario = 0
            iNum = 0
        end if

        ListAppend ( lsInterim(), sType + ";" + sVario + ";" + iNum )
    next i

    if bAsianLan = FALSE then
        Warnlog "The file for comparison does not exists. The file will be written!"
        Warnlog "Please check : " + sList
        ListWrite ( lsInterim(), sList )
    else
        Warnlog "The file for comparison does not exists. The file will be written!"
        Warnlog "Please check : " + sList
        ListWrite ( lsInterim(), sList, "utf8" )
    end if

end sub

'*******************************************************************************

sub Make3D

    '///test with 3D-objects when 3D-options are changed ( view page )
    gApplication = "IMPRESS"
    call hNewDocument()
    WL_SD_Wuerfel
    Sleep 1

    Kontext "Documentimpress"
    DocumentImpress.MouseDown 50, 50
    DocumentImpress.MouseMove 30, 60
    DocumentImpress.MouseUp 30, 60
    Sleep 3
    call hCloseDocument()
    gApplication = "WRITER"

end sub

'*******************************************************************************

sub DeleteColor( cColorName as String )
    'Deletes a color by name. The color is selected in the listbox
    'and should - if it exists - be visible in the entryfield above the list.
    'If this is not the case the color probably not exists and thus
    'cannot be deleted.
    'Remember i18n, only use this sub for colors you created yourself!
    dim iItems as Integer     'Number of listed colors
    dim i as Integer          'counter
    dim bExists as Boolean    'TRUE if color has been successfully deleted
    dim iPos as Integer       'Position of the deleted color
    dim sColor as string

    printlog "DeleteColor:: - Trying to delete color: '" + cColorName + "'"
    Kontext "TabFarben"
    bExists = FALSE
    iItems = Farbe.getItemCount
    i = 0
    while ((i<iItems) AND (NOT bExists))
        sColor = Farbe.getItemText(i+1)
        if (sColor = cColorName) then
            Loeschen.Click
            Kontext "Active"
            if Active.Exists then
                printlog active.getText
                Active.Yes
            else
                qaErrorlog "No warning about deleting the color."
            endif
            printlog " - successfully deleted color @ " + (i+1) + "/" + iItems
            bExists = TRUE
        else
            inc (i)
        endif
'        if FarbName.getText = cColorName then
    wend
    iPos = i+1

    if bExists then
        'The userdefined colors usually are appended to the list. If they are
        'somewhere else this might be a bug.
        if iItems <> iPos then
            warnlog  "   The color was not located at the end of the list."
            printlog "   The Order of the list might be corrupted"
        end if
    else
        'Inform that the color did not exist. This usually is perfectly ok.
        printlog "   (The color was not deleted, it was not found)"
    endif
end sub

'*******************************************************************************

sub modifyColorRGB_PGUP( iColor as Integer )
    'The current color's values are set to maximum (255) for RGB
    printlog( "modifyColorRGB_PGUP:: - change the color by pressing PAGE UP in RGB listboxes." )
    
    Kontext "TabFarben"
    Farbe.Select(iColor)
    R.TypeKeys("<PageUp>")
    G.TypeKeys("<PageUp>")
    B.TypeKeys("<PageUp>")
    printlog("modifyColorRGB_PGUP:: Press 'modify'")
    Aendern.Click()
    Sleep (1)
end sub

'*******************************************************************************

sub createNewColor( aColor() as String )
    ' INPUT : array: (1): Name; (2): Red value; (3): Green value; (4): Blue value
    ' OUTPUT: 
    'The desired color is selected by name and created.
    'If it already exists, there is a problem
    printlog( "createNewColor:: Adding a color to the list: '" + aColor(1) + "'"
    Kontext "TabFarben"
    FarbName.setText(aColor(1))
    R.SetText(aColor(2))
    G.SetText(aColor(3))
    B.SetText(aColor(4))
    Sleep(1)
    Hinzufuegen.Click
    Sleep(1)
    
    Kontext "DuplicateNameWarning"
    if DuplicateNameWarning.Exists then
        warnlog "createNewColor:: Color already exists."
        DuplicateNameWarning.OK
        Kontext "NameDlg"
        if NameDlg.Exists then
            printlog "createNewColor:: Naming dialog shown. Good, cancelling"
            NameDlg.Cancel
        else
            warnlog "createNewColor:: Naming dialog didn't came up."
        end if
    else
        printlog "createNewColor:: New color has been created"
    end if
end sub

'*******************************************************************************

sub getColorRGB( aColor() as String )
    ' INPUT : array with index 1-4 that will get deleted
    ' OUTPUT: array: (1): Name; (2): Red value; (3): Green value; (4): Blue value
    'A RGB color always has four attributes:
    'The name and the three RGB values (0...255)
    printlog( "GetColorRGB:: Determining the current color" )
    FarbModell.Select(1)
    
    aColor(1) = FarbName.GetText()
    aColor(2) = R.GetText()
    aColor(3) = G.GetText()
    aColor(4) = B.GetText()
    
    printlog( "GetColorRGB:: N = " & aColor(1) )
    printlog( "GetColorRGB:: R = " & aColor(2) )
    printlog( "GetColorRGB:: G = " & aColor(3) )
    printlog( "GetColorRGB:: B = " & aColor(4) )
end sub

'*******************************************************************************

sub compareTwoColorsRGB( aColor() as String )
    ' INPUT : array: (1): Name; (2): Red value; (3): Green value; (4): Blue value
    '         and a selected color
    ' OUTPUT: 
    'Comparision of two colors. Tested values are:
    'Name and three RGB values
    'aColor is the expected value, bColor is the current color
    dim i as Integer
    dim bColor(4) as String

    printlog( "compareTwoColorsRGB:: Compare saved default color with the current" )
    call getColorRGB(bColor()) 
    for i = 1 to 4
        if aColor(i) <> bColor(i) then
            warnlog( "compareTwoColorsRGB:: " & "(" & i & ") Expected: '" _
            & aColor( i ) & "' found: '" & bColor(i) & "'" )
        else
            printlog( "compareTwoColorsRGB:: " & "(" & i & ") OK" )
        endif
    next i
end sub

'*******************************************************************************

sub ModifyColorRGB( aColor() as String )
    'A RGB color always has four attributes:
    'The name and the three RGB values (0...255)

    Kontext "TabFarben"
    FarbName.SetText( aColor( 1 ) )
    FarbModell.Select( 1 )
    R.SetText( aColor( 2 ) )
    G.SetText( aColor( 3 ) )
    B.SetText( aColor( 4 ) )
    Aendern.Click
end sub

