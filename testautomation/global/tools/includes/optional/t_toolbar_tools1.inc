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
'* owner : helge.delfs@oracle.com
'*
'* short description : Toolbar tools 1
'*
'***************************************************************************************
'*
' #1 fActiveObjectInToolbar           ' active/inactive image button in toolbar
'*
'\*************************************************************************************

'******************************************************************
'* Created by hercule.li@oracle.com
'* This function will active or inactive image button in toolbar
'* sToolbar : Toolbar name                  
'* sObject  : image button name                   
'* bActive  : Active or Inactive image button
'*            TRUE  --> Active
'*            FALSE --> Inactive             
'* Will return the original status of the image button
'*****************************************************************
function fActiveObjectInToolbar(sToolbar as String , sObject as String , bActive as Boolean) as Boolean

    Dim iObject      as Integer
    Dim sToolbarName as String
    Dim sFlag        as Boolean
    
    sToolbarName = fGetToolbarName(sToolbar)
    iObject      = fGetObject(sToolbar , sObject)

    ToolsCustomize
    sleep 3
    Kontext
    Active.SetPage TabCustomizeToolbars
    Kontext "TabCustomizeToolbars"
      Menu.Select sToolbarName
      Sleep 1
      ToolbarContents.Typekeys "<Home>"
      Sleep 1
      if iObject-1 > 0 then
          ToolbarContents.Typekeys "<Down>" , iObject-1
      endif
      sFlag = ToolbarContents.IsChecked
      if bActive = TRUE then
          if sFlag = FALSE then ToolbarContents.Check
      else
          ToolbarContents.UnCheck
      endif
    TabCustomizeToolbars.OK
    fActiveObjectInToolbar = sFlag

end function


'******************************************
'* This function will Get toolbar's name **
'* Return Toolbar's name in StarOffice   **
'******************************************
function fGetToolbarName(sToolbar) as String
    
  Select case sToolbar 
      case "3D-Settings" :
          Select case iSprache
              case 01   : fGetToolbarName   = "3D-Settings"
              case 33   : fGetToolbarName   = "3D-Settings"
              case 34   : fGetToolbarName   = "3D-Settings"
              case 39   : fGetToolbarName   = "3D-Settings"
              case 46   : fGetToolbarName   = "3D-Settings"
              case 49   : fGetToolbarName   = "3D-Einstellungen"
              case 50   : fGetToolbarName   = "Nastavitve 3D"
              case 55   : fGetToolbarName   = "3D-Settings"
              case 81   : fGetToolbarName   = "3D-Settings"
              case 82   : fGetToolbarName   = "3D-Settings"
              case 86   : fGetToolbarName   = "3D-Settings"
              case 88   : fGetToolbarName   = "3D-Settings"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "3D-Settings"
          end select

      case "Align" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Align"
              case 33   : fGetToolbarName   = "Align"
              case 34   : fGetToolbarName   = "Align"
              case 39   : fGetToolbarName   = "Align"
              case 46   : fGetToolbarName   = "Align"
              case 49   : fGetToolbarName   = "Ausrichten"
              case 50   : fGetToolbarName   = "Poravnava"
              case 55   : fGetToolbarName   = "Align"
              case 81   : fGetToolbarName   = "Align"
              case 82   : fGetToolbarName   = "Align"
              case 86   : fGetToolbarName   = "Align"
              case 88   : fGetToolbarName   = "Align"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Align"
          end select

      case "Basic Shapes" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Basic Shapes"
              case 33   : fGetToolbarName   = "Basic Shapes"
              case 34   : fGetToolbarName   = "Basic Shapes"
              case 39   : fGetToolbarName   = "Basic Shapes"
              case 46   : fGetToolbarName   = "Basic Shapes"
              case 49   : fGetToolbarName   = "Standardformen"
              case 50   : fGetToolbarName   = "Osnovni liki"
              case 55   : fGetToolbarName   = "Basic Shapes"
              case 81   : fGetToolbarName   = "Basic Shapes"
              case 82   : fGetToolbarName   = "Basic Shapes"
              case 86   : fGetToolbarName   = "Basic Shapes"
              case 88   : fGetToolbarName   = "Basic Shapes"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Basic Shapes"
          end select

      case "Block Arrows" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Block Arrows"
              case 33   : fGetToolbarName   = "Block Arrows"
              case 34   : fGetToolbarName   = "Block Arrows"
              case 39   : fGetToolbarName   = "Block Arrows"
              case 46   : fGetToolbarName   = "Block Arrows"
              case 49   : fGetToolbarName   = "Blockpfeile"
              case 50   : fGetToolbarName   = "Votle puščice"
              case 55   : fGetToolbarName   = "Block Arrows"
              case 81   : fGetToolbarName   = "Block Arrows"
              case 82   : fGetToolbarName   = "Block Arrows"
              case 86   : fGetToolbarName   = "Block Arrows"
              case 88   : fGetToolbarName   = "Block Arrows"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Block Arrows"
          end select

      case "Bullets and Numbering" : 
          Select case iSprache
              case 01   : fGetToolbarName   = "Bullets and Numbering"
              case 33   : fGetToolbarName   = "Bullets and Numbering"
              case 34   : fGetToolbarName   = "Bullets and Numbering"
              case 39   : fGetToolbarName   = "Bullets and Numbering"
              case 46   : fGetToolbarName   = "Bullets and Numbering"
              case 49   : fGetToolbarName   = "Nummerierung und Aufzählungszeichen"
              case 50   : fGetToolbarName   = "Oznake in oštevilčevanje"
              case 55   : fGetToolbarName   = "Bullets and Numbering"
              case 81   : fGetToolbarName   = "Bullets and Numbering"
              case 82   : fGetToolbarName   = "Bullets and Numbering"
              case 86   : fGetToolbarName   = "Bullets and Numbering"
              case 88   : fGetToolbarName   = "Bullets and Numbering"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Bullets and Numbering"
          end select

      case "Database Form Design" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Database Form Design"
              case 33   : fGetToolbarName   = "Database Form Design"
              case 34   : fGetToolbarName   = "Database Form Design"
              case 39   : fGetToolbarName   = "Database Form Design"
              case 46   : fGetToolbarName   = "Database Form Design"
              case 49   : fGetToolbarName   = "Datenbank Formularentwurf"
              case 50   : fGetToolbarName   = "Oblikovanje obrazca zbirke podatkov"
              case 55   : fGetToolbarName   = "Database Form Design"
              case 81   : fGetToolbarName   = "Database Form Design"
              case 82   : fGetToolbarName   = "Database Form Design"
              case 86   : fGetToolbarName   = "Database Form Design"
              case 88   : fGetToolbarName   = "Database Form Design"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Database Form Design"
          end select

      case "Drawing" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Drawing"
              case 33   : fGetToolbarName   = "Drawing"
              case 34   : fGetToolbarName   = "Drawing"
              case 39   : fGetToolbarName   = "Drawing"
              case 46   : fGetToolbarName   = "Drawing"
              case 49   : fGetToolbarName   = "Zeichnen"
              case 50   : fGetToolbarName   = "Risba"
              case 55   : fGetToolbarName   = "Drawing"
              case 81   : fGetToolbarName   = "Drawing"
              case 82   : fGetToolbarName   = "Drawing"
              case 86   : fGetToolbarName   = "Drawing"
              case 88   : fGetToolbarName   = "Drawing"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Drawing"
          end select

      case "Drawing Object Properties" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Drawing Object Properties"
              case 33   : fGetToolbarName   = "Drawing Object Properties"
              case 34   : fGetToolbarName   = "Drawing Object Properties"
              case 39   : fGetToolbarName   = "Drawing Object Properties"
              case 46   : fGetToolbarName   = "Drawing Object Properties"
              case 49   : fGetToolbarName   = "Zeichnungsobjekt-Eigenschaften"
              case 50   : fGetToolbarName   = "Lastnosti risanega predmeta"
              case 55   : fGetToolbarName   = "Drawing Object Properties"
              case 81   : fGetToolbarName   = "Drawing Object Properties"
              case 82   : fGetToolbarName   = "Drawing Object Properties"
              case 86   : fGetToolbarName   = "Drawing Object Properties"
              case 88   : fGetToolbarName   = "Drawing Object Properties"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Drawing Object Properties"
          end select

      case "Form Design" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Form Design"
              case 33   : fGetToolbarName   = "Form Design"
              case 34   : fGetToolbarName   = "Form Design"
              case 39   : fGetToolbarName   = "Form Design"
              case 46   : fGetToolbarName   = "Form Design"
              case 49   : fGetToolbarName   = "Formular Entwurf"
              case 50   : fGetToolbarName   = "Oblikovanje obrazca"
              case 55   : fGetToolbarName   = "Form Design"
              case 81   : fGetToolbarName   = "Form Design"
              case 82   : fGetToolbarName   = "Form Design"
              case 86   : fGetToolbarName   = "Form Design"
              case 88   : fGetToolbarName   = "Form Design"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Form Design"
          end select

      case "Formatting" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Formatting"
              case 33   : fGetToolbarName   = "Formatting"
              case 34   : fGetToolbarName   = "Formatting"
              case 39   : fGetToolbarName   = "Formatting"
              case 46   : fGetToolbarName   = "Formatting"
              case 49   : fGetToolbarName   = "Format"
              case 50   : fGetToolbarName   = "Oblikovanje"
              case 55   : fGetToolbarName   = "Formatting"
              case 81   : fGetToolbarName   = "Formatting"
              case 82   : fGetToolbarName   = "Formatting"
              case 86   : fGetToolbarName   = "Formatting"
              case 88   : fGetToolbarName   = "Formatting"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Formatting"
          end select

      case "Frame" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Frame"
              case 33   : fGetToolbarName   = "Frame"
              case 34   : fGetToolbarName   = "Frame"
              case 39   : fGetToolbarName   = "Frame"
              case 46   : fGetToolbarName   = "Frame"
              case 49   : fGetToolbarName   = "Rahmen"
              case 50   : fGetToolbarName   = "Okvir"
              case 55   : fGetToolbarName   = "Frame"
              case 81   : fGetToolbarName   = "Frame"
              case 82   : fGetToolbarName   = "Frame"
              case 86   : fGetToolbarName   = "Frame"
              case 88   : fGetToolbarName   = "Frame"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Frame"
          end select

      case "Insert" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Insert"
              case 33   : fGetToolbarName   = "Insert"
              case 34   : fGetToolbarName   = "Insert"
              case 39   : fGetToolbarName   = "Insert"
              case 46   : fGetToolbarName   = "Insert"
              case 49   : fGetToolbarName   = "Einfügen"
              case 50   : fGetToolbarName   = "Vstavi"
              case 55   : fGetToolbarName   = "Insert"
              case 81   : fGetToolbarName   = "Insert"
              case 82   : fGetToolbarName   = "Insert"
              case 86   : fGetToolbarName   = "Insert"
              case 88   : fGetToolbarName   = "Insert"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Insert"
          end select

      case "OLE-Object" :
          Select case iSprache
              case 01   : fGetToolbarName   = "OLE-Object"
              case 33   : fGetToolbarName   = "OLE-Object"
              case 34   : fGetToolbarName   = "OLE-Object"
              case 39   : fGetToolbarName   = "OLE-Object"
              case 46   : fGetToolbarName   = "OLE-Object"
              case 49   : fGetToolbarName   = "OLE Objekt"
              case 50   : fGetToolbarName   = "Predmet OLE"
              case 55   : fGetToolbarName   = "OLE-Object"
              case 81   : fGetToolbarName   = "OLE-Object"
              case 82   : fGetToolbarName   = "OLE-Object"
              case 86   : fGetToolbarName   = "OLE-Object"
              case 88   : fGetToolbarName   = "OLE-Object"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "OLE-Object"
          end select

      case "Standard" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Standard"
              case 33   : fGetToolbarName   = "Standard"
              case 34   : fGetToolbarName   = "Standard"
              case 39   : fGetToolbarName   = "Standard"
              case 46   : fGetToolbarName   = "Standard"
              case 49   : fGetToolbarName   = "Standard"
              case 50   : fGetToolbarName   = "Standardno"
              case 55   : fGetToolbarName   = "Standard"
              case 81   : fGetToolbarName   = "Standard"
              case 82   : fGetToolbarName   = "Standard"
              case 86   : fGetToolbarName   = "Standard"
              case 88   : fGetToolbarName   = "Standard"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Standard"
          end select

      case "Table" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Table"
              case 33   : fGetToolbarName   = "Table"
              case 34   : fGetToolbarName   = "Table"
              case 39   : fGetToolbarName   = "Table"
              case 46   : fGetToolbarName   = "Table"
              case 49   : fGetToolbarName   = "Tabelle"
              case 50   : fGetToolbarName   = "Tabela"
              case 55   : fGetToolbarName   = "Table"
              case 81   : fGetToolbarName   = "Table"
              case 82   : fGetToolbarName   = "Table"
              case 86   : fGetToolbarName   = "Table"
              case 88   : fGetToolbarName   = "Table"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Table"
          end select

      case "Text Object" :
          Select case iSprache
              case 01   : fGetToolbarName   = "Text Object"
              case 33   : fGetToolbarName   = "Text Object"
              case 34   : fGetToolbarName   = "Text Object"
              case 39   : fGetToolbarName   = "Text Object"
              case 46   : fGetToolbarName   = "Text Object"
              case 49   : fGetToolbarName   = "Textobjekt"
              case 50   : fGetToolbarName   = "Besedilni predmet"
              case 55   : fGetToolbarName   = "Text Object"
              case 81   : fGetToolbarName   = "Text Object"
              case 82   : fGetToolbarName   = "Text Object"
              case 86   : fGetToolbarName   = "Text Object"
              case 88   : fGetToolbarName   = "Text Object"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "Text Object"
          end select

      case "XML Form Design" :
          Select case iSprache
              case 01   : fGetToolbarName   = "XML Form Design"
              case 33   : fGetToolbarName   = "XML Form Design"
              case 34   : fGetToolbarName   = "XML Form Design"
              case 39   : fGetToolbarName   = "XML Form Design"
              case 46   : fGetToolbarName   = "XML Form Design"
              case 49   : fGetToolbarName   = "XML Formularentwurf"
              case 50   : fGetToolbarName   = "Oblikovanje obrazca XML"
              case 55   : fGetToolbarName   = "XML Form Design"
              case 81   : fGetToolbarName   = "XML Form Design"
              case 82   : fGetToolbarName   = "XML Form Design"
              case 86   : fGetToolbarName   = "XML Form Design"
              case 88   : fGetToolbarName   = "XML Form Design"
              case else : QAErrorLog "The test does not support the language " + iSprache
                          fGetToolbarName   = "XML Form Design"
          end select

  end select

end function

'*******************************************************
'* This function will get the location for image button 
'* in Commands in Tools/Customize/Toolbars
'*******************************************************
function fGetObject(sToolbar as String , sObject as String) as Integer

  select case gApplication
      
      case "WRITER"       :  fGetObject = fGetObjectWriter(sToolbar , sObject)   
      case "HTML" :  fGetObject = fGetObjectWriter(sToolbar , sObject)
      case "MASTERDOCUMENT"    :  fGetObject = fGetObjectWriter(sToolbar , sObject)
                 
      case "CALC"         :  fGetObject = fGetObjectCalc(sToolbar , sObject)

      case "IMPRESS"      :  fGetObject = fGetObjectImpress(sToolbar , sObject)

  end select

end function


