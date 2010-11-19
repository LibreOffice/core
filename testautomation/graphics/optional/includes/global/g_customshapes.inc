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
'* short description : Customshapes-test
'*
'*********************************************************************
' #1 tCustomshapes
' #1 tFormatPaintbrush
'\********************************************************************
testcase tCustomshapes

   dim counter as Integer
   dim MakeInto3Dcounter as Integer

   printlog "Create new document "
   Call hNewDocument
   Call sSelectEmptyLayout
    if gApplication = "DRAW" then
        Kontext "Documentdraw"
    else '(Impress)
        Kontext "DocumentImpress"
    endif
    kontext "Toolbar"
    sleep 1
    printlog "From the toolbar: Insert three objects: "
    printlog "1st: insert a Smiley. "
    kontext "Toolbar"
     if Toolbar.Exists then
        if Toolbar.IsVisible then
           sleep 1
           try
              SymbolShapes.TearOff
           catch
              warnlog "bug for GH from FHA; .tearoff doesnt tell success"
           endcatch
           sleep 1
           kontext "SymbolShapes"
           SymbolShapesSmiley.Click
           sleep 1

           gMouseDown (40,40)
           gMouseMove (40,40,60,60)
           gMouseUp (60,60)

           sleep 1
           kontext "SymbolShapes"
	   SymbolShapes.Close

           printlog "Unmark all objects "
           hTypeKeys "<ESCAPE>"

           printlog "Mark the Smiley "
           hTypeKeys "<TAB>"

           printlog "Rename the object to 'First'."
           hOpenContextMenu
           sleep 2
           if hMenuFindSelect(27027, true, 15) = false then
              Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
              Call hCloseDocument
              Goto Endsub
           endif
           kontext "NameDlgObject"
           NameField.SetText "First"
           NameDlgObject.OK

           printlog "Open the Context-menu and set the description of the object to 'First'"
           hOpenContextMenu
           sleep (2)
           if hMenuFindSelect(27033, true, 14) = false then
              Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
              Call hCloseDocument
              Goto Endsub
           endif
           kontext "DescriptionObject"
	   DescriptionTitle.SetText "FirstTitle"
	   DescriptionText.SetText "FirstText"
	   DescriptionObject.Ok
        else
           warnlog "the Toolbar wasnt visible."
        endif
     else
        warnlog "the Toolbar didnt exist."
     endif
     gMouseClick 90, 90

     printlog "2nd: Insert a Triangle "
     kontext "Toolbar"
     if Toolbar.Exists then
        if Toolbar.IsVisible then
           try
              BasicShapes.TearOff
           catch
              warnlog "bug for GH from FHA; .tearoff doesnt tell success"
           endcatch
           kontext "BasicShapes"
           sleep 1
           IsoscelesTriangle.click
           sleep 1

           gMouseDown (45,33)
           gMouseMove (45,33,55,42)
           gMouseUp (55,42)
           sleep 1
           kontext "BasicShapes"
	   BasicShapes.Close
           sleep 1

           printlog "Unmark all objects "
           hTypeKeys "<ESCAPE>"
           printlog "Mark the Triangle "
           hTypeKeys "<TAB>", 2

           printlog "Rename the object into 'Second'. "
           hOpenContextMenu
           sleep 2
           if hMenuFindSelect(27027, true, 15) = false then
              Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
              Call hCloseDocument
             Goto Endsub
           endif

           sleep 1
           kontext "NameDlgObject"
           NameField.SetText "Second"
           NameDlgObject.OK

           printlog "Open the Context-menu and set the description of the object to 'SecondTitle' and 'SecondText'"
           hOpenContextMenu
           sleep (2)
           if hMenuFindSelect(27033, true, 14) = false then
              Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
              Call hCloseDocument
              Goto Endsub
           endif
           kontext "DescriptionObject"
	   DescriptionTitle.SetText "SecondTitle"
	   DescriptionText.SetText "SecondText"
	   DescriptionObject.Ok
	else
           warnlog "the Toolbar wasnt visible."
        endif
     else
        warnlog "the Toolbar didnt exist."
     endif
     gMouseClick 90, 90

     printlog "3rd: Insert Thinking-cloud."
     kontext "Toolbar"
     if Toolbar.Exists then
        if Toolbar.IsVisible then
           sleep 1
           try
              CalloutShapes.TearOff  
           catch
              warnlog "bug for GH from FHA; .tearoff doesnt tell success"
           endcatch
           kontext "Callouts"
           sleep 1
           Cloud.click
           sleep 1

           gMouseDown (1,1)
           gMouseMove (1,1,35,30)
           gMouseUp (35,30)
           sleep 1

           kontext "Callouts"
	   Callouts.Close
	   sleep 1

           printlog "Unmark all objects "
           hTypeKeys "<ESCAPE>"
           printlog "Mark the cloud "
           hTypeKeys "<TAB>", 3

           printlog "Rename the object to 'Third'. "
           hOpenContextMenu
           sleep 2
           if hMenuFindSelect(27027, true, 15) = false then
              Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
              Call hCloseDocument
              Goto Endsub
           endif
           Kontext "NameDlgObject"
           NameField.SetText "Third"
           NameDlgObject.OK

           printlog "Open the Context-menu and set the description of the object to 'First'"
           hOpenContextMenu
           sleep (2)
           if hMenuFindSelect(27033, true, 14) = false then
              Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
              Call hCloseDocument
              Goto Endsub
           endif
           kontext "DescriptionObject"
	   DescriptionTitle.SetText "ThirdTitle"
	   DescriptionText.SetText "ThirdText"
	   DescriptionObject.Ok
	else
           warnlog "the Toolbar wasnt visible."
        endif
     else
        warnlog "the Toolbar didnt exist."
     endif
     gMouseClick 90, 90

     printlog "Change all objects into 3D. "
     if gApplication = "DRAW" then
        Kontext "Documentdraw"
     else '(Impress)
        Kontext "DocumentImpress"
     endif
     For MakeInto3Dcounter = 1 to 3
         hTypeKeys "<TAB>", MakeInto3Dcounter
         kontext "Toolbar"
         if Toolbar.Exists then
             if Toolbar.IsVisible then
                 if ExtrusionOnOff.Exists then 
                     ExtrusionOnOff.Click
                     sleep (3)
                 else
                     sleep (1)
                     Toolbar.OpenContextMenu ' enable forms button in menuebar
                     sleep (1)
                     hMenuselectNr (1)
                     sleep (1)
                     if hMenuFindSelect(1024, true, 25) = false then
                         Warnlog "Context-Menu-entry `Extrusion On/Off` was not found. Therefore the test ends."
                         Call hCloseDocument
                         Goto Endsub
                     endif
                     sleep (1)
                     kontext "Toolbar"
                     ExtrusionOnOff.Click
                     sleep (3)
                 endif
	         Kontext "ExtrusionObjectBar"
                 if ExtrusionObjectBar.Exists then
                     if ExtrusionObjectBar.IsVisible then
                         printlog "   Objects got converted into Extrusion-objects, correct."
                     else
                         warnlog "   No Extrusion-toolbar visible. Eventually a problem making objects into Extrusion-Objects."
                     endif
                 endif
             endif
         else
             warnlog "   the 'Toolbar' -Toolbar didnt exist."
         endif
         sleep (1)
         hTypeKeys "<ESCAPE>"
     Next MakeInto3Dcounter

     printlog "See if the objects still are Custom Shapes "
     printlog "We do so by moving the lower part of the the thinking-cloud. "
     gMouseClick 90, 1
     hTypeKeys "<TAB><TAB><TAB>"

     hTypeKeys "<MOD1 TAB>" ,9
     if gApplication = "IMPRESS" then
        hTypeKeys "<RIGHT>" ,86
        hTypeKeys "<UP>", 48
     else
        hTypeKeys "<RIGHT>" ,131
        hTypeKeys "<UP>" ,53
     endif
     sleep 1

     gMouseClick 90, 1
     hTypeKeys "<TAB>"

     printlog "Check the name of the selected object (should be 'First') "
     hOpenContextMenu
     sleep 2
     'Choose rename.
     if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif

     sleep 1
     kontext "NameDlgObject"
     if NameField.GetText <> "First" then
        warnlog "   Wrong object. Should have been the first, but was the " + NameField.GetText
     else
        Printlog "   The first object still had the correct name, good."
     endif
     NameDlgObject.OK

     printlog "Open the Context-menu and check if the description of the object is right"
     hOpenContextMenu
     sleep (2)
     'Choose "Description"
     if hMenuFindSelect(27033, true, 14) = false then
        Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif
     kontext "DescriptionObject"
     if DescriptionTitle.GetText <> "FirstTitle" then 
         Warnlog "   The description-title for the first object were not 'FirstTitle', but: " + DescriptionTitle.GetText
     endif
     if DescriptionText.GetText <> "FirstText" then 
         Warnlog "   The description-text for the first object were not 'FirstText', but: " + DescriptionText.GetText
     endif
     DescriptionObject.Ok

     Kontext "ExtrusionObjectBar"
     if ExtrusionObjectBar.Exists then
        if ExtrusionObjectBar.IsVisible then
           printlog "   The first object got converted into a 3D-object (Extrusion-Object), correct."
        else
           warnlog "   No Extrusion-Object-toolbar visible. Eventually a problem making the object into 3D."
        endif
     endif

     gMouseClick 90, 1
     hTypeKeys "<TAB><TAB>"
     printlog "Check the name of the selected object (should be 'Second') "
     hOpenContextMenu
     sleep 2
     if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "   Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif

     sleep 1
     kontext "NameDlgObject"
     if NameField.GetText <> "Second" then
        warnlog "   Wrong object. Should have been the second, but was the " + NameField.GetText
     else
        Printlog "   The second object still had the correct name, good."
     endif
     NameDlgObject.OK

     printlog "Open the Context-menu and check if the description of the object is right"
     hOpenContextMenu
     sleep (2)
     'Choose "Description"
     if hMenuFindSelect(27033, true, 14) = false then
        Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif
     kontext "DescriptionObject"
     if DescriptionTitle.GetText <> "SecondTitle" then
        Warnlog "   The description-title for the second object were not 'SecondTitle', but: " + DescriptionTitle.GetText
     else
        printlog "   The description-title for the second object were correct"
     endif
     if DescriptionText.GetText <> "SecondText" then
        Warnlog "   The description-text for the second object were not 'SecondText', but: " + DescriptionText.GetText
     else
        printlog "   The description-text for the second object were correct"
     endif
     DescriptionObject.Ok

     Kontext "ExtrusionObjectBar"
     if ExtrusionObjectBar.Exists then
        if ExtrusionObjectBar.IsVisible then
           printlog "   The first second got converted into 3D-object (Extrusion-Object), correct."
        else
           warnlog "   No Extrusion-Object-toolbar visible. Eventually a problem making the object into 3D."
        endif
     endif

     gMouseClick 90, 1
     hTypeKeys "<TAB><TAB><TAB>"
     printlog "Check the name of the selected object (should be Third) "
     hOpenContextMenu
     sleep 2
     'Choose rename.
     if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif

     sleep 1
     kontext "NameDlgObject"
     if NameField.GetText <> "Third" then
        warnlog "   Wrong object. Should have been the third, but was the " + NameField.GetText
     else
        Printlog "   The third object still had the correct name, good."
     endif
     NameDlgObject.OK

     printlog "Open the Context-menu and check if the description of the object is right"
     hOpenContextMenu
     sleep (2)
     'Choose "Description"
     if hMenuFindSelect(27033, true, 14) = false then
        Warnlog "Context-Menu-entry `Description` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif
     kontext "DescriptionObject"
     if DescriptionTitle.GetText <> "ThirdTitle" then
        Warnlog "   The description-title for object three were not 'ThirdTitle', but: " + DescriptionTitle.GetText
     else
        printlog "   The description-title for the second object were correct."
     endif
     if DescriptionText.GetText <> "ThirdText" then
        Warnlog "   The description-text for object three were not 'ThirdText', but: " + DescriptionText.GetText
     else
        printlog "   The description-text for object three were correct."
     endif
     DescriptionObject.Ok

     Kontext "ExtrusionObjectBar"
     if ExtrusionObjectBar.Exists then
        if ExtrusionObjectBar.IsVisible then
           printlog "   The third object got converted into 3D-object (Extrusion-Object), correct."
        else
           warnlog "   No Extrusion-Object-toolbar visible. Eventually a problem making the object into 3D."
        endif
     endif

     printlog "End of test. Close the document."
     Call hCloseDocument         
     
endcase 'tCustomshapes
'---------------------------------------------------------
testcase tFormatPaintbrush

   dim counter, ZaehlerVerlauf, j as Integer
   printlog "Hatching "
   dim HatchingDistance as string
   dim HatchingAngle as string
   dim HatchingLineType as string
   dim HatchingLineColor as string

   dim position_value as string
   dim show_value as boolean
   dim distance_value as string
   dim color_value as string
   dim transparency_value as string

   dim NoTransparency_value as boolean
   dim linearTransparency_value as boolean
   dim MFLintransparency_value as string
   dim Transparencyverlauf_value as boolean
   dim TransparencyType_value as string
   dim MFCenterX_value as string
   dim MFCenterY_value as string
   dim MFAngle_value as string
   dim MFRand_value as string
   dim MFStart_value as string
   dim MFEnd_value as string

   dim colorname_value as string
   dim color2_value as string
   dim colormodel_value as integer
   dim R_value as string
   dim G_value as string
   dim B_value as string
   dim C_value as string
   dim M_value as string
   dim Y_value as string
   dim K_value as string

   dim type_value as string
   dim centerx_value as string
   dim centery_value as string
   dim angle_value as string
   dim rand_value as string
   dim from_value as string
   dim fromvalue_value as string
   dim to_value as string
   dim tovalue_value as string
   dim to_wert_value as string

   dim front_value as string
   dim background_value as string

   printlog "Create new document "
   Call hNewDocument
      printlog "Create a new rectangle "
      hRechteckErstellen ( 10, 10, 30, 30 )
      FormatArea
         printlog "Switch to Tabpage Area "
         Kontext
         Active.SetPage TabArea
         Kontext "TabArea"
         sleep 1
         printlog "Select hatching, control after closing and reopening dialog if changes are recognized "
         FillOptions.Select 4          
         printlog "Switch to Tabpage Hatching "
         Kontext
         Active.setpage TabSchraffuren
         Kontext "TabSchraffuren"

         printlog "Get the values for the Rectangle "
         HatchingDistance = Abstand.GetText
         HatchingAngle = Winkel.GetText
         'Definition
         HatchingLineType = Linientyp.GetSelText
         HatchingLineColor = Linienfarbe.GetSelText

         'TabSchraffuren.OK
         sleep 1

'         FormatArea
         printlog "Switch to Tabpage Shaddow "
         Kontext
         Active.setpage TabSchatten
         Kontext "TabSchatten"

         show_value = Anzeigen.isChecked 'Boolean
         if Anzeigen.isChecked then
            position_value = Position.GetSelText
            distance_value = Entfernung.GetSelText
            color_value = Farbe.GetSelText
            transparency_value = Transparenz.GetSelText
         endif
         sleep 1

         printlog "Switch to Tabpage Transparency "
         Kontext
         Active.SetPage TabTransparenz
         Kontext "TabTransparenz"

         NoTransparency_value = KeineTransparenz.isChecked
         linearTransparency_value = LineareTransparenz.isChecked
         MFLinTransparency_value = MFLinTransparenz.GetText
         Transparencyverlauf_value = Transparenzverlauf.isChecked
         TransparencyType_value = TransparenzverlaufTyp.GetSelText
         MFCenterX_value = MFZentrumX.GetText
         MFCenterY_value = MFZentrumY.GetText
         MFAngle_value = MFWinkel.GetText
         MFRand_value = MFRand.GetText
         MFStart_value = MFStartwert.GetText
         MFEnd_value = MFEndwert.GetText

         sleep 1
         printlog "Switch to Tabpage Color "
         Kontext
         Active.setpage TabFarben
         Kontext "TabFarben"

         colorname_value = FarbName.GetText
         color_value = Farbe.GetSelText
         colormodel_value = Farbmodell.GetSelIndex
         if colormodel_value = 1 then
            R_value = R.GetText
            G_value = G.GetText
            B_value = B.GetText
         else 'farbmodell_value = 2
            C_value = C.GetText
            M_value = M.GetText
            Y_value = Y.GetText
            K_value = K.GetText
         endif
         sleep 1

         printlog "Switch to Tabpage Gradient "
         Kontext
         Active.setpage TabFarbverlaeufe
         Kontext "TabFarbverlaeufe"

         type_value = Typ.GetSelText
         Centerx_value = ZentrumX.GetText
         Centery_value = ZentrumY.GetText
         angle_value = Winkel.GetText
         rand_value = Rand.GetText
         from_value = Von.GetSelText
         fromvalue_value = VonWert.GetText
         to_value = Bis.GetSelText
         tovalue_value = BisWert.GetText
         sleep 1

         printlog "Switch to Tabpage Bitmap "
         Kontext
         Active.setpage TabBitmap
         Kontext "TabBitmap"

         front_value = Vordergrund.GetSelText
         background_value = Hintergrund.GetSelText
         sleep 1
      printlog "Close the FormatArea-dialogue with OK "
      TabBitmap.OK
      sleep 2

      printlog "Create a second rectangle, and use the Paintbrush-icon to copy the settings for the first rectangle onto the new one "
      hRechteckErstellen ( 70, 70, 90, 90 )
      if gApplication = "DRAW" then
         kontext "Documentdraw"
         Documentdraw.Mousedown 20, 20
         Documentdraw.Mouseup 20, 20
         sleep 1
         kontext "Standardbar"
         FormatPaintbrush.Click
         kontext "Documentdraw"
         sleep 1
         Documentdraw.Mousedown 80, 80
         Documentdraw.Mouseup 80, 80
         sleep 1
      else '(Impress)
         Kontext "DocumentImpress"
         DocumentImpress.typekeys("<F6>",true)
         DocumentImpress.typekeys("<ESCAPE>",true)
         DocumentImpress.typekeys("<TAB>",true)
         sleep 1
         kontext "Standardbar"
         FormatPaintbrush.Click
         kontext "DocumentImpress"
         sleep 1
         DocumentImpress.typekeys("<TAB>",true)
         sleep 1
      endif

      printlog "Check if the Formatting is like the first one. "
      FormatArea
         printlog "Switch to Tabpage Area "
         Kontext
         Active.SetPage TabArea
         Kontext "TabArea"
'         if FillOptions.GetSelIndex <> 4 then        printlog "The selected value should be 'no fill' "
'            warnlog "The FillOptions-value was not transferred correctly."
'         else
'            printlog "The FillOptions-value was transferred correctly."
'         endif

            sleep 1
            printlog "control if changes are still there after closing and reopening dialog "
            FillOptions.Select 4
            printlog "Select hatching, control after closing and reopening dialog if changes are recognized "
            printlog "Switch to Tabpage Hatching "
            kontext
            Active.setpage TabSchraffuren
            kontext "TabSchraffuren"

            if HatchingDistance <> Abstand.GetText then
               warnlog "Hatching:Distance  changed. Should be: " + HatchingDistance + ". Was: " + Abstand.GetText
            endif
            if HatchingAngle <> Winkel.GetText then
               warnlog "Hatching:Angle  changed. Should be: " + HatchingAngle + ". Was: " + Winkel.GetText
            endif
            'Definition
            if HatchingLineType <> Linientyp.GetSelText then
               warnlog "Hatching:LineType  changed. Should be: " + HatchingLineType + ". Was: " + Linientyp.GetSelText
            endif
            if HatchingLineColor <> Linienfarbe.GetSelText then
               warnlog "Hatching:LineColor  changed. Should be: " + HatchingLineColor + ". Was: " + Linienfarbe.GetSelText
            endif
            sleep 1

         printlog "Switch to Tabpage Shaddow "
         Kontext
         Active.setpage TabSchatten
         Kontext "TabSchatten"

            if Show_value = true then
               if Anzeigen.isChecked <> true then
                  warnlog "Shaddow:Show  had changed! Should have been true, but was " + Anzeigen.isChecked
               else
                  if position_value <> Position.GetSelText then
                     warnlog "Shaddow:position:value  changed. Should be: " + position_value + ". Was: " + Position.GetSelText
                  endif
                  if distance_value <> Entfernung.GetSelText then
                     warnlog "Shaddow:distance:value  changed. Should be: " + distance_value + ". Was: " + Entfernung.GetSelText
                  endif
                  if color_value <> Farbe.GetSelText then
                     warnlog "Shaddow:Color:value  changed. Should be: " + color_value + ". Was: " + Farbe.GetSelText
                  endif
                  if transparency_value <> Transparenz.GetSelText then
                     warnlog "Shaddow:transparency:value  changed. Should be: " + transparency_value + ". Was: " + Transparenz.GetSelText
                  endif
               endif
            endif
            sleep 1

         printlog "Switch to Tabpage Transparency "
         Kontext
         Active.SetPage TabTransparenz
         Kontext "TabTransparenz"
            if NoTransparency_value <> KeineTransparenz.isChecked then
               warnlog "Transparency:NoTransparency -value changed. Should be: " + NoTransparency_value + ". Was: " + KeineTransparenz.isChecked
            endif
            if linearTransparency_value <> LineareTransparenz.isChecked then
               warnlog "Transparency:lineareTransparency -value changed. Should be: " + linearTransparency_value + ". Was: " + LineareTransparenz.isChecked
            endif
            if MFLinTransparency_value <> MFLinTransparenz.GetText then
               warnlog "Transparency:MFLinTransparency -value changed. Should be: " + MFLinTransparency_value + ". Was: " + MFLinTransparenz.GetText
            endif
            if Transparencyverlauf_value <> Transparenzverlauf.isChecked then
               warnlog "Transparency:Transparencyverlauf -value changed. Should be: " + Transparencyverlauf_value + ". Was: " + Transparenzverlauf.isChecked
            endif
            if TransparencyType_value <> TransparenzverlaufTyp.GetSelText then
               warnlog "Transparency:TransparencyType -value changed. Should be: " + TransparencyType_value + ". Was: " + TransparenzverlaufTyp.GetSelText
            endif
            if MFCenterX_value <> MFZentrumX.GetText then
               warnlog "Transparency:MFCenterX -value changed. Should be: " + MFCenterX_value + ". Was: " + MFZentrumX.GetText
            endif
            if MFCenterY_value <> MFZentrumY.GetText then
               warnlog "Transparency:MFCenterY -value changed. Should be: " + MFCenterY_value + ". Was: " + MFZentrumY.GetText
            endif
            if MFAngle_value <> MFWinkel.GetText then
               warnlog "Transparency:MFAngle -value changed. Should be: " + MFAngle_value + ". Was: " + MFWinkel.GetText
            endif
            if MFRand_value <> MFRand.GetText then
               warnlog "Transparency:MFRand -value changed. Should be: " + MFRand_value + ". Was: " + MFRand.GetText
            endif
            if MFStart_value <> MFStartwert.GetText then
               warnlog "Transparency:MFStart -value changed. Should be: " + MFStart_value + ". Was: " + MFStartwert.GetText
            endif
            if MFEnd_value <> MFEndwert.GetText then
               warnlog "Transparency:MFEnd -value changed. Should be: " + MFEnd_value + ". Was: " + MFEndwert.GetText
            endif
            sleep 1

         printlog "Switch to Tabpage Color "
         Kontext
         Active.setpage TabFarben
         Kontext "TabFarben"

            if colorname_value <> FarbName.GetText then
               warnlog "colorname_value changed. Should be: " + colorname_value + ". Was: " + FarbName.GetText
            endif
            if color_value <> Farbe.GetSelText then
               warnlog "color_value changed. Should be: " + color_value + ". Was: " + Farbe.GetSelText
            endif
            if colormodel_value <> Farbmodell.GetSelIndex then
               warnlog "colormodel_value changed. Should be: " + colormodel_value + ". Was: " + Farbmodell.GetSelIndex
            endif
            if colormodel_value = 1 then
               if R_value <> R.GetText then
                  warnlog "R_value changed. Should be: " + R_value + ". Was: " + R.GetText
               endif
               if G_value <> G.GetText then
                  warnlog "G_value changed. Should be: " + G_value + ". Was: " + G.GetText
               endif
               if B_value <> B.GetText then
                  warnlog "B_value changed. Should be: " + B_value + ". Was: " + B.GetText
               endif
            endif
            if colormodel_value = 2 then
               if C_value <> C.GetText then
                  warnlog "C_value changed.Should be: " + C_value + ". Was: " + C.GetText
               endif
               if M_value <> M.GetText then
                  warnlog "M_value changed. Should be: " + M_value + ". Was: " + M.GetText
               endif
               if Y_value <> Y.GetText then
                  warnlog "Y_value changed. Should be: " + Y_value + ". Was: " + Y.GetText
               endif
               if K_value <> K.GetText then
                  warnlog "K_value changed. Should be: " + K_value + ". Was: " + K.GetText
               endif
            endif
            sleep 1

         printlog "Switch to Tabpage Gradient "
         Kontext
         Active.setpage TabFarbverlaeufe
         Kontext "TabFarbverlaeufe"
            if type_value <> Typ.GetSelText then
               warnlog "type_value changed. Should be: " + type_value + ". Was: " + Typ.GetSelText
            endif
            if centerx_value <> ZentrumX.GetText then
               warnlog "centerx_value changed. Should be: " + centerx_value + ". Was: " + ZentrumX.GetText
            endif
            if centery_value <> ZentrumY.GetText then
               warnlog "centery_value changed. Should be: " + centery_value + ". Was: " + ZentrumY.GetText
            endif
            if angle_value <> Winkel.GetText then
               warnlog "angle_value changed. Should be: " + angle_value + ". Was: " + Winkel.GetText
            endif
            if rand_value <> Rand.GetText then
               warnlog "rand_value changed. Should be: " + rand_value + ". Was: " + Rand.GetText
            endif
            if from_value <> Von.GetSelText then
               warnlog "from_value changed. Should be: " + from_value + ". Was: " + Von.GetSelText
            endif
            if fromvalue_value <> VonWert.GetText then
               warnlog "fromvalue_value changed. Should be: " + fromvalue_value + ". Was: " + VonWert.GetText
            endif
            if to_value <> Bis.GetSelText then
               warnlog "to_value changed. Should be: " + to_value + ". Was: " + Bis.GetSelText
            endif
            if tovalue_value <> BisWert.GetText then
               warnlog "tovalue_value changed. Should be: " + tovalue_value + ". Was: " + BisWert.GetText
            endif
            sleep 1

         printlog "Switch to Tabpage Bitmap "
         Kontext
         Active.setpage TabBitmap
         Kontext "TabBitmap"
            if front_value <> Vordergrund.GetSelText then
               warnlog "vordergrund_value changed. Should be: " + front_value + ". Was: " + Vordergrund.GetSelText
            endif
            if background_value <> Hintergrund.GetSelText then
               warnlog "background_value changed. Should be: " + background_value + ". Was: " + Hintergrund.GetSelText
            endif
            sleep 1
      TabBitmap.OK
      printlog "Close Document "
   Call hCloseDocument         
   
endcase 'tFormatPaintbrush
