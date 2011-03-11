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
'* short description : Procedures for the Clipboard-Export-Test
'*
'*******************************************************************
'*
' #1 Get_Position_Size_Attributes       :Getting values from the position and size dialog
' #1 Get_Colour_Attributes              :Geting Attributes from Area Dialog
' #1 Check_Position_Size_Attributes     :Pasting object and comparing attributes with the stored values
' #1 Check_Text_Formatting              :Checking text formatting of pasted object
' #1 Check_Colors_Borders_Attributes    :
' #1 SetKontextApplication              :-
' #1 New_Draw                           :-
' #1 New_Impress                        :-
' #1 New_Calc                           :-
' #1 New_Writer                         :-
' #1 Choose_Toapplication               :-

'*
'\*******************************************************************

Sub Get_Position_Size_Attributes

    Printlog "Get_Position_Size_Attributes Starts"

    FormatPositionAndSize

    sleep 1

    Kontext
    Select Case gApplication
    Case "WRITER"  : active.SetPage TabPositionAndSizeWriter
        Kontext "TabPositionAndSizeWriter"
    Case else      : active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
    End Select
    sleep 1

    printlog "Size-values"
    PositionWidth = Width.GetText
    PositionHeight = Height.GetText

    if KeepRatio.IsChecked then SetText.PositionKeepRatio = "TRUE"
        if ProtectPosition.IsChecked then SetText.PositionProtectPosition = "TRUE"
            if ProtectSize.IsChecked then SetText.PositionProtectSize = "TRUE"
                sleep 1

                Kontext
                Select Case gApplication
                Case "WRITER"  : active.SetPage TabPositionAndSizeWriter
                    Kontext "TabPositionAndSizeWriter"
                    TabPositionAndSizeWriter.OK
                Case else    :   active.SetPage TabPositionAndSize
                    Kontext "TabPositionAndSize"
                    TabPositionAndSize.OK
                End Select
                sleep 1
                SetKontextApplication
                sleep 3
                Printlog "   Get_Position_Size_Attributes Ends"

end sub 'Get_Position_Size_Attributes

'-------------------------------------------------------------------------------
sub Get_Colour_Attributes

    printlog "   Get_Colour_Attributes Starts"

    FormatArea
    Kontext "TabArea"
    Kontext
    Active.setpage TabArea
    Kontext "TabArea"


    printlog "Area"
    FillOptions.Select 2
    if FillOptions.GetSelIndex <> 2 then Errorlog "Couldnt click on 'Color' in FormatArea"
        AreaColourListValue = ColourList.GetSelText
        printlog "AreaColourListValue = " + AreaColourListValue
        sleep 1

        FillOptions.Select 3
        if FillOptions.GetSelIndex <> 3 then Errorlog "Couldnt click on .Gradient. in FormatArea"

            AreaGradientListValue = GradientList.GetSelText    'AreaGradientValue = "TRUE"
            If Automatic.IsVisible then
                If Automatic.IsChecked then AreaAutomaticValue = "TRUE"
                Else
                    If Increments.IsVisible then
                        AreaIncrementsValue = Increments.GetText
                    End if
                End if

                FillOptions.Select 4
                if FillOptions.GetSelIndex <> 4 then Errorlog "Couldnt click on 'Hatching' in FormatArea"

                    AreaHatchingListValue = HatchingList.GetSelText 'Then  = "TRUE"  ' lista
                    If BackgroundColour.IsVisible then
                        If BackgroundColour.IsChecked then
                            AreaBackgroundColourValue = BackgroundColour.GetText
                        End if
                    End if
                    If BackgroundColourList.IsVisible then
                        AreaBackgroundColourListValue = BackgroundColourList.GetSelText
                    End if

                    FillOptions.Select 5
                    if FillOptions.GetSelIndex <> 5 then Errorlog "Couldnt click on 'Hatching' in FormatArea"

                        AreaBitmapListValue = BitmapList.GetSelText

                        If Original.IsChecked then
                            AreaOriginalListValue = "TRUE"
                        Else
                            If Relative.IsChecked then
                                AreaRelativeValue = "TRUE"
                                AreaWidthValue = Width.GetText
                                AreaHeightValue = Height.GetText
                            Else
                                AreaWidthValue = Width.GetText
                                AreaHeightValue = Height.GetText
                            End if
                        End if

                        If Tile.IsChecked then AreaTileValue = "TRUE"
                            If Stretch.IsChecked then AreaStretchValue = "TRUE"
                                If Row.IsChecked then
                                    AreaRowValue = "TRUE"
                                    AreaOffsetValue = Offset.GetText
                                End if
                                If Column.IsChecked then
                                    AreaColumnValue = "TRUE"
                                    AreaOffsetValue = Offset.GetText
                                End if

                                sleep 1


                                Kontext "TabArea"
                                Kontext
                                Active.setpage TabSchatten
                                Kontext "TabSchatten"

                                If Anzeigen.IsChecked then
                                    ShadowUseShadowsValue = "TRUE"
                                    ShadowDistanceValue = Entfernung.GetText
                                    ShadowColorValue = Farbe.GetText
                                    ShadowTransparencyValue = Transparenz.GetText
                                end if

                                sleep 1


                                Kontext
                                Active.SetPage TabTransparenz
                                Kontext "TabTransparenz"
                                if KeineTransparenz.IsEnabled = FALSE then
                                    warnlog "   Couldnt check 'No transparency' in Area - Transparency'"
                                else
                                    KeineTransparenz.Check
                                end if

                                If KeineTransparenz.IsChecked then LineareTransparenz.Check
                                    If LineareTransparenz.IsChecked then
                                        TransparencyTransparencyValue = MFLinTransparenz.GetText
                                    End if


                                    Transparenzverlauf.Check
                                    If Transparenzverlauf.IsChecked then
                                        TransparencyGradientType = TransparenzverlaufTyp.GetItemText
                                        TransparencyMFZentrumX = MFZentrumX.GetText
                                        TransparencyMFZentrumY = MFZentrumY.GetText
                                        TransparencyMFWinkel = MFWinkel.GetText
                                        TransparencyMFRand = MFRand.GetText
                                        TransparencyMFStartwert = MFStartwert.GetText
                                        TransparencyMFEndwert = MFEndwert.GetText
                                    End if

                                    sleep 1


                                    Kontext
                                    Active.setpage TabFarben
                                    Kontext "TabFarben"

                                    ColorName = FarbName.GetText
                                    ColorColor = Farbe.GetItemText
                                    ColorModel = Farbmodell.GetSelText
                                    printlog "   The colormodel is: " + ColorModel

                                    if ColorModel = "RGB" then
                                        ColorR = R.GetText
                                        ColorG = G.GetText
                                        ColorB = B.GetText
                                    elseif ColorModel = "RVB" then
                                        ColorR = R.GetText
                                        ColorG = G.GetText
                                        ColorB = B.GetText
                                    else
                                        ColorC = C.GetText
                                        ColorM = M.GetText
                                        ColorY = Y.GetText
                                        ColorK = K.GetText
                                    endif

                                    Kontext
                                    Active.setpage TabFarbverlaeufe
                                    Kontext "TabFarbverlaeufe"

                                    GradientsType = Typ.GetItemText
                                    GradientsCenterX = ZentrumX.GetText
                                    GradientsCenterZ = ZentrumY.GetText
                                    GradientsAngle = Winkel.GetText
                                    GradientsBorder = Rand.GetText
                                    GradientsFromColor = Von.GetItemText
                                    GradientsFromValue = VonWert.GetText
                                    GradientsToColor = Bis.GetItemText
                                    GradientsToValue = BisWert.GetText


                                    Kontext
                                    Active.setpage TabSchraffuren
                                    Kontext "TabSchraffuren"

                                    HatchingDistance = Abstand.GetText
                                    HatchingAngle = Winkel.GetText
                                    'Definition
                                    HatchingLineType = Linientyp.GetSelText
                                    HatchingLineColor = Linienfarbe.GetSelText
                                    HatchingLineTable = Tabelle.GetItemText


                                    Kontext
                                    Active.setpage TabBitmap
                                    Kontext "TabBitmap"

                                    BitmapForeground = Vordergrund.GetSelText
                                    BitmapBackground = Hintergrund.GetSelText
                                    BitmapTable = Tabelle.GetSelText

                                    Kontext
                                    Active.setpage TabArea
                                    Kontext "TabArea"
                                    TabArea.Cancel
                                    printlog "   Get_Colour_Attributes ends"

end sub 'Get_Colour_Attributes

'-------------------------------------------------------------------------------
sub Check_Position_Size_Attributes

    printlog "   Check_Position_Size_Attributes Starts"

    sleep 1
    EditPaste
    sleep (5)

    FormatPositionAndSize

    sleep 1
    if gApplication = "WRITER" then
        Kontext "TabPositionAndSizeWriter"
        Kontext
        active.SetPage TabPositionAndSizeWriter
        Kontext
        active.SetPage TabPositionAndSizeWriter
        Kontext "TabPositionAndSizeWriter"
        sleep 1


        If LiberalMeasurement(PositionWidth, Width.GetText) <> TRUE then QaErrorLog "PositionWidth is: " + Width.GetText + " should be: '" + PositionWidth + "'. Possibly caused by i71631."
            If LiberalMeasurement(PositionHeight, Height.GetText) <> TRUE then QaErrorLog "PositionHeight is: " + Height.GetText + " should be: '" + PositionHeight + "'. Possibly caused by i71631."

                If PositionKeepRatio = "TRUE" then
                    If KeepRatio.IsChecked = "FALSE" then Warnlog "KeepRatio isnt checked"
                    End if

                    If PositionProtectPosition = "TRUE" then
                        If ProtectPosition.IsChecked = "FALSE" then Warnlog "ProtectPosition isnt checked"
                        End if

                        If PositionProtectSize = "TRUE" then
                            If ProtectSize.IsChecked = "FALSE" then Warnlog "ProtectPosition isnt checked"
                            End if
                            sleep 1

                            Kontext "TabPositionAndSizeWriter"
                            Kontext
                            Active.setpage TabPositionAndSizeWriter
                            Kontext "TabPositionAndSizeWriter"

                            TabPositionAndSizeWriter.OK

                        else
                            Kontext
                            active.SetPage TabPositionAndSize
                            Kontext "TabPositionAndSize"
                            sleep 1


                            If LiberalMeasurement(PositionWidth, Width.GetText) <> TRUE then QaErrorLog "PositionWidth is: " + Width.GetText + " should be: " + PositionWidth + "'. Possibly caused by i71631."
                                If LiberalMeasurement(PositionHeight, Height.GetText) <> TRUE then QaErrorLog "PositionHeight is: " + Height.GetText + " should be: " + PositionHeight + "'. Possibly caused by i71631."

                                    If PositionKeepRatio = "TRUE" then
                                        If KeepRatio.IsChecked = "FALSE" then Warnlog "KeepRatio isnt checked"
                                        End if

                                        If PositionProtectPosition = "TRUE" then
                                            If ProtectPosition.IsChecked = "FALSE" then Warnlog "ProtectPosition isnt checked"
                                            End if

                                            If PositionProtectSize = "TRUE" then
                                                If ProtectSize.IsChecked = "FALSE" then Warnlog "ProtectPosition isnt checked"
                                                End if
                                                sleep 1

                                                Kontext "TabPositionAndSize"
                                                Kontext
                                                Active.setpage TabPositionAndSize
                                                Kontext "TabPositionAndSize"

                                                TabPositionAndSize.OK
                                            endif

                                            SetKontextApplication
                                            sleep 3
                                            printlog "   Check_Position_Size_Attributes Ends"

end sub 'Check_Position_Size_Attributes

'----------------------------------------------------------------------------------------------------
sub Check_Text_Formatting

    printlog "   Check_Text_Formatting Starts"
    sleep (1)
    EditPaste
    sleep (3)

    select case (gApplication)
    case "DRAW"    : hTypeKeys "<RETURN>"
        hTypeKeys "<SHIFT HOME>" 'Mark the whole text'
    case "IMPRESS" : hTypeKeys "<RETURN>"
        hTypeKeys "<SHIFT HOME>" 'Mark the whole text'
    case "WRITER"  : EditSelectAll  'Since there is currently only (should only be) one object (the text) in the document.
    case "CALC"    : hTypeKeys "<MOD1 HOME>"
        '                           hTypeKeys "<SHIFT HOME>" 'Mark the whole text'
    case else      : warnlog "   the test has not been configured for this application ( '" + gApplication + "' ). Please notify testwriter."
    end select
    sleep (2)

    Kontext "TextObjectbar"
    if TextObjectbar.Exists = FALSE then
        ViewToolbarsTextFormatting
        sleep (1)
    endif
    Printlog "   - Check if the Fontname is still the same."
    if Schriftart.GetSelText <> Text1 then warnlog "   Fontname should be: " + Text1 + " but was: " + Schriftart.GetSelText
        sleep (2)
        Printlog "   - Check if the Text still has the same Size."
        if Schriftgroesse.GetSelText <> Text2 then warnlog "   Fontsize should be: " + Text2 + " but was: " + Schriftgroesse.GetSelText

            Kontext "TextObjectbar"
            Printlog "   - Check if the Font-attribute is set to Bold."
            if Fett.GetState(2) <> Text4 then warnlog "   Bold should be: " + Text4 + " but was: " + Fett.GetState(2)

                Kontext "TextObjectbar"
                Printlog "   - Font attribute cursive"
                if Kursiv.GetState(2) <> Text5 then warnlog "   Italic should be: " + Text5 + " but was: " + Kursiv.GetState(2)

                    Kontext "TextObjectbar"
                    Printlog "   - Check if the Font-attribute is set to Underlined"
                    sleep (2)
                    if Unterstrichen.GetState(2) <> Text6 then warnlog "   Underline should be: " + Text6 + " but was: " + Unterstrichen.GetState(2)

                        Kontext "TextObjectbar"
                        if (gApplication) = "CALC" then
                            kontext "FormatObjectBar"
                            Printlog "   - Check if the Font-attribute is set to Justified"
                            if Block.GetState(2) <> Text7 then warnlog "   Justified should be: " + Text7 + " but was: " + Blocksatz.GetState(2)
                            else
                                if Blocksatz.GetState(2) <> Text7 then warnlog "   Justified should be: " + Text7 + " but was: " + Blocksatz.GetState(2)
                                endif
                                SetKontextApplication
                                printlog "   Check_Text_Formatting Ends"

end sub 'Check_Text_Formatting

'-------------------------------------------------------------------------------

sub Check_Colors_Borders_Attributes

    printlog "   Check_Colors_Borders_Attributes Starts"

    FormatArea

    Kontext "TabArea"
    Kontext
    Active.setpage TabArea
    Kontext "TabArea"

    printlog "Area"
    FillOptions.Select 2
    if FillOptions.GetSelIndex <> 2 then Errorlog "Couldnt click on 'Color' in FormatArea"

        If AreaColourListValue <> ColourList.GetSelText then
            WarnLog "   Area ColourList doesn't have the right Colour selected."
            printlog "   AreaColourListValue = " + AreaColourListValue + " should be = " + ColourList.GetSelText
        end if

        sleep 1

        FillOptions.Select 3
        if FillOptions.GetSelIndex <> 3 then Errorlog "Couldnt click on 'Gradient.' in FormatArea"

            If AreaGradientListValue <> GradientList.GetSelText then WarnLog "Area GradientList has changed it's value."
                If Automatic.IsVisible then
                    If AreaAutomaticValue = "TRUE" then
                        If Automatic.IsChecked then
                            Printlog "   Area Automatic Value is correct"
                        Else
                            WarnLog "   Area Automatic Value wasnt checked. It should have been."
                        End if
                    End if
                Else
                    If Increments.IsVisible then
                        If AreaIncrementsValue <> Increments.GetText then WarnLog "Area Increments has changed."
                        End if
                    End if

                    FillOptions.Select 4
                    if FillOptions.GetSelIndex <> 4 then Errorlog "Couldnt click on 'Hatching' in FormatArea"

                        If AreaHatchingListValue <> HatchingList.GetSelText then Errorlog "Area HatchingList has changed it's value."
                            If BackgroundColour.IsVisible then
                                If BackgroundColour.IsChecked then
                                    If AreaBackgroundColourValue <> BackgroundColour.GetText then Errorlog "Area Background Color has changed it's value."
                                    end if
                                end if
                                If BackgroundColourList.IsVisible then
                                    If AreaBackgroundColourListValue <> BackgroundColourList.GetSelText then Errorlog "Area Background ColourList has changed it's value."
                                    End if

                                    FillOptions.Select 5
                                    if FillOptions.GetSelIndex <> 5 then Errorlog "Couldnt click on 'Color' in FormatArea"

                                        If BitmapList.IsVisible then
                                            If AreaBitmapListValue <> BitmapList.GetSelText then Errorlog "Area Bitmap List has changed it's value."
                                            else
                                                warnlog "   No Area-Bitmap-List visible!"
                                            end if
                                            If Original.IsChecked = "TRUE" then
                                                If AreaOriginalListValue <> "TRUE" then Errorlog "Original List has changed it's value."
                                                Else
                                                    If Relative.IsChecked = "TRUE" then
                                                        If AreaRelativeValue <> "TRUE" then Errorlog "Area Relative has changed it's value." = "TRUE"
                                                            If AreaWidthValue <> Width.GetText then Errorlog "AreaWidthValue has changed."
                                                                If AreaHeightValue <> Height.GetText then Errorlog "AreaHeightValue has changed."
                                                                Else
                                                                    If AreaWidthValue <> Width.GetText then Errorlog "AreaWidthValue has changed."
                                                                        If AreaHeightValue <> Height.GetText then Errorlog "AreaHeightValue has changed."
                                                                        End if
                                                                    End if

                                                                    If Tile.IsChecked then
                                                                        If AreaTileValue <> "TRUE" then Errorlog "AreaTileValue has changed."
                                                                        End if

                                                                        If Stretch.IsChecked then
                                                                            If AreaStretchValue <> "TRUE" then Errorlog "AreaStretchValue has changed."
                                                                            End if

                                                                            If Row.IsChecked then
                                                                                If AreaRowValue <> "TRUE" then Errorlog "AreRowValue has changed."
                                                                                    If AreaOffsetValue <> Offset.GetText then Errorlog "AreaOffsetValue has changed."
                                                                                    End if

                                                                                    If Column.IsChecked then
                                                                                        If AreaColumnValue <> "TRUE" then Errorlog "AreaColumnValue has changed."
                                                                                            If AreaOffsetValue <> Offset.GetText then Errorlog "AreaOffsetValue has changed."
                                                                                            End if

                                                                                            sleep 1

                                                                                            Kontext "TabArea"
                                                                                            Kontext
                                                                                            Active.setpage TabSchatten
                                                                                            Kontext "TabSchatten"

                                                                                            If Anzeigen.IsChecked then
                                                                                                ShadowUseShadowsValue = "TRUE"
                                                                                                ShadowDistanceValue = Entfernung.GetText
                                                                                                ShadowColorValue = Farbe.GetText
                                                                                                ShadowTransparencyValue = Transparenz.GetText
                                                                                            end if

                                                                                            sleep 1

                                                                                            Kontext
                                                                                            Active.SetPage TabTransparenz
                                                                                            Kontext "TabTransparenz"

                                                                                            if KeineTransparenz.IsEnabled = FALSE then
                                                                                                warnlog "   Couldnt check 'No transparency' in Area - Transparency'"
                                                                                            else
                                                                                                KeineTransparenz.Check
                                                                                            end if

                                                                                            If KeineTransparenz.IsChecked then LineareTransparenz.Check
                                                                                                If LineareTransparenz.IsChecked then
                                                                                                    TransparencyTransparencyValue = MFLinTransparenz.GetText
                                                                                                end if

                                                                                                Transparenzverlauf.Check
                                                                                                If Transparenzverlauf.IsChecked then
                                                                                                    TransparencyGradientType = TransparenzverlaufTyp.GetItemText
                                                                                                    TransparencyMFZentrumX = MFZentrumX.GetText
                                                                                                    TransparencyMFZentrumY = MFZentrumY.GetText
                                                                                                    TransparencyMFWinkel = MFWinkel.GetText
                                                                                                    TransparencyMFRand = MFRand.GetText
                                                                                                    TransparencyMFStartwert = MFStartwert.GetText
                                                                                                    TransparencyMFEndwert = MFEndwert.GetText
                                                                                                end if

                                                                                                sleep 1

                                                                                                Kontext
                                                                                                Active.setpage TabFarben
                                                                                                Kontext "TabFarben"
                                                                                                ColorName = FarbName.GetText
                                                                                                ColorColor = Farbe.GetItemText
                                                                                                ColorModel = Farbmodell.GetSelText
                                                                                                printlog "   " + ColorModel

                                                                                                                if ColorC <> C.GetText then Warnlog "Wrong C-Color. Should be: " + ColorC + " but was: " + C.GetText
                                                                                                                    if ColorM <> M.GetText then Warnlog "Wrong M-Color. Should be: " + ColorM + " but was: " + M.GetText
                                                                                                                        if ColorY <> Y.GetText then Warnlog "Wrong J-Color. Should be: " + ColorY + " but was: " + Y.GetText
                                                                                                                            if ColorK <> K.GetText then Warnlog "Wrong N-Color. Should be: " + ColorK + " but was: " + K.GetText

                                                                                                                                                Kontext
                                                                                                                                                Active.setpage TabFarbverlaeufe
                                                                                                                                                Kontext "TabFarbverlaeufe"

                                                                                                                                                GradientsType = Typ.GetItemText
                                                                                                                                                GradientsCenterX = ZentrumX.GetText
                                                                                                                                                GradientsCenterZ = ZentrumY.GetText
                                                                                                                                                GradientsAngle = Winkel.GetText
                                                                                                                                                GradientsBorder = Rand.GetText
                                                                                                                                                GradientsFromColor = Von.GetItemText
                                                                                                                                                GradientsFromValue = VonWert.GetText
                                                                                                                                                GradientsToColor = Bis.GetItemText
                                                                                                                                                GradientsToValue = BisWert.GetText

                                                                                                                                                Kontext
                                                                                                                                                Active.setpage TabSchraffuren
                                                                                                                                                Kontext "TabSchraffuren"

                                                                                                                                                HatchingDistance = Abstand.GetText
                                                                                                                                                HatchingAngle = Winkel.GetText
                                                                                                                                                'Definition
                                                                                                                                                HatchingLineType = Linientyp.GetSelText
                                                                                                                                                HatchingLineColor = Linienfarbe.GetSelText
                                                                                                                                                HatchingLineTable = Tabelle.GetItemText

                                                                                                                                                Kontext
                                                                                                                                                Active.setpage TabBitmap
                                                                                                                                                Kontext "TabBitmap"

                                                                                                                                                BitmapForeground = Vordergrund.GetSelText
                                                                                                                                                BitmapBackground = Hintergrund.GetSelText
                                                                                                                                                BitmapTable = Tabelle.GetSelText

                                                                                                                                                Kontext
                                                                                                                                                Active.setpage TabArea
                                                                                                                                                Kontext "TabArea"
                                                                                                                                                TabArea.Cancel

                                                                                                                                                SetKontextApplication

                                                                                                                                                printlog "Check_Colors_Borders_Attributes Ends"

                                                                                                                                                Printlog "End of one application"


end sub 'Check_Colors_Borders_Attributes
   '---------------------------------- Set the Kontext to the current Application ----------------------------------'
sub SetKontextApplication

    sleep 1
    Select Case gApplication
    Case "DRAW"
        Kontext "DocumentDraw"
        '         Printlog "gApplication / Kontext is now: DocumentDraw"
        sleep 1
    Case "IMPRESS"
        Kontext "DocumentImpress"
        '         Printlog "gApplication / Kontext is now: DocumentImpress"
        sleep 1
    Case "WRITER"
        Kontext "DocumentWriter"
        '         Printlog "gApplication / Kontext is now: DocumentWriter"
        sleep 1
    Case "CALC"
        Kontext "DocumentCalc"
        '         Printlog "gApplication / Kontext is now: DocumentCalc"
        sleep 1
    end select
    sleep 1

end sub 'SetKontextApplication

'------------------------- Part with "create new windows" ---------------------
sub New_Draw

    gApplication = "DRAW"
    ToApp = "DRAW"
    printlog "to app has changed to: " + gApplication
    Call hNewDocument
    SetKontextApplication

end sub 'New_Draw

'-------------------------------------------------------------------------------
sub New_Impress

    printlog "Copied object from application: " + FromApp2
    gApplication = "IMPRESS"
    ToApp = "IMPRESS"
    printlog "to app has changed to: " + gApplication
    Call hNewDocument
    SetKontextApplication

end sub 'New_Impress

'-------------------------------------------------------------------------------
sub New_Calc

    printlog "Copied object from application: " + FromApp2
    gApplication = "CALC"
    ToApp = "CALC"
    printlog "to app has changed to: " + gApplication
    Call hNewDocument
    SetKontextApplication

end sub 'New_Calc

'-------------------------------------------------------------------------------
sub New_Writer

    printlog "Copied object from application: " + FromApp2
    gApplication = "WRITER"
    ToApp = "WRITER"
    printlog "to app has changed to: " + gApplication
    Call hNewDocument
    SetKontextApplication

end sub 'New_Writer

'-------------------------------------------------------------------------------
testcase Choose_Toapplication

    For ToAppCounter = 1 to 4

        Select case ToAppCounter
        case "1"   :   Call hCloseDocument
            printlog "to app: " + gApplication  ' Since the first application is Draw by default, we'll skip that one.
            ToApp = "DRAW"
        case "2"   :   Call hCloseDocument
            gApplication = "IMPRESS"
            printlog "to app: " + gApplication
            ToApp = "IMPRESS"
        case "3"   :   Call hCloseDocument
            gApplication = "CALC"
            printlog "to app: " + gApplication
            ToApp = "CALC"
        case "4"   :   Call hCloseDocument
            gApplication = "WRITER"
            printlog "to app: " + gApplication
            ToApp = "WRITER"
        case else  :   goto endsub
        End select
    next ToAppCounter
    Call hNewDocument
    SetKontextApplication

endcase 'Choose_Toapplication

'--------------------------------------------------------------------------------------------------------
