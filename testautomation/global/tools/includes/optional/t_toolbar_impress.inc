'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_toolbar_impress.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:09 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description : Toolbar tools - Impress
'*
'***************************************************************************************
'*
' #0 fGetObjectImpress
'*
'\*************************************************************************************

'*******************************************************
'* This function will get the location for image button 
'* in Commands in Tools/Customize/Toolbars from Impress
'*******************************************************
function fGetObjectImpress(sToolbar as String , sObject as String) as Integer

  Select case sToolbar
      case "3D-Objects"
          Select case sObject
          end select

      case "3D-Settings"
          Select case sObject
              case "Extrusion On/Off"   : fGetObjectImpress  = 1
                   '-----------------                          2
              case "Tilt Down"          : fGetObjectImpress  = 3
              case "Tilt Up"            : fGetObjectImpress  = 4
              case "Tilt Left"          : fGetObjectImpress  = 5
              case "Tilt Right"         : fGetObjectImpress  = 6
                   '-----------------                          7
              case "Depth"              : fGetObjectImpress  = 8
              case "Direction"          : fGetObjectImpress  = 9
              case "Lighting"           : fGetObjectImpress  = 10
              case "Surfact"            : fGetObjectImpress  = 11
              case "3D Color"           : fGetObjectImpress  = 12
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectImpress   = 0
          end select

      case "Align"
          Select case sObject
              case "Left"              : fGetObjectImpress  = 1
              case "Centered"          : fGetObjectImpress  = 2
              case "Right"             : fGetObjectImpress  = 3
                   '-----------------                         4
              case "Top"               : fGetObjectImpress  = 5
              case "Center"            : fGetObjectImpress  = 6
              case "Bottom"            : fGetObjectImpress  = 7
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectImpress   = 0
          end select

      case "Arrows"
          Select case sObject
          end select

      case "Basic Shapes"
          Select case sObject
          end select

      case "Block Arrows"
          Select case sObject
          end select

      case "Bullets and Numbering"
          Select case sObject
          end select

      case "Callouts"
          Select case sObject
          end select

      case "Color"
          Select case sObject
          end select

      case "Connectors"
          Select case sObject
          end select

      case "Controls"
          Select case sObject
          end select

      case "Drawing"
          Select case sObject
              case "Select"            : fGetObjectImpress  = 1
                   '-----------------                         2
              case "Line"              : fGetObjectImpress  = 3
              case "Rectangle"         : fGetObjectImpress  = 4
              case "Ellipse"           : fGetObjectImpress  = 5
              case "Text"              : fGetObjectImpress  = 6
              case "Vertical Text"     : fGetObjectImpress  = 7
                   '-----------------                         8
              case "Curve"             : fGetObjectImpress  = 9
              case "Connector"         : fGetObjectImpress  = 10
              case "3D Objects"        : fGetObjectImpress  = 11
              case "Basic Shapes"      : fGetObjectImpress  = 12
              case "Symbol Shapes"     : fGetObjectImpress  = 13
              case "Block Arrows"      : fGetObjectImpress  = 14
              case "Flowcharts"        : fGetObjectImpress  = 15
              case "Callouts"          : fGetObjectImpress  = 16
              case "Stars"             : fGetObjectImpress  = 17
                   '-----------------                         18              
              case "Points"            : fGetObjectImpress  = 19
              case "Glue Points"       : fGetObjectImpress  = 20
              case "To Curve"          : fGetObjectImpress  = 21
              case "To Polygon"        : fGetObjectImpress  = 22
              case "To 3D"             : fGetObjectImpress  = 23
              case "To 3D Rotation Objet" : fGetObjectImpress  = 24
                   '-----------------                         25           
              case "Fontwork Gallery"  : fGetObjectImpress  = 26
              case "From File"         : fGetObjectImpress  = 27
              case "Gallerty"          : fGetObjectImpress  = 28              
                   '-----------------                         29           
              case "Rotate"            : fGetObjectImpress  = 30
              case "Rotation and Size" : fGetObjectImpress  = 31
              case "Flip"              : fGetObjectImpress  = 32
              case "Alignment"         : fGetObjectImpress  = 33
              case "Arrange"           : fGetObjectImpress  = 34
                   '-----------------                         35
              case "Insert"            : fGetObjectImpress  = 36
              case "Controls"          : fGetObjectImpress  = 37
                   '-----------------                         38
              case "Extrusion On/Off"  : fGetObjectImpress  = 39
              case "Custom Animation"  : fGetObjectImpress  = 40
              case "Interaction"       : fGetObjectImpress  = 41
              case "Animated Image"    : fGetObjectImpress  = 42
              case "3D Effects"        : fGetObjectImpress  = 43
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectImpress   = 0
          end select

      case "Edit Points"
          Select case sObject
          end select

      case "Flowchart"
          Select case sObject
          end select

      case "Fontwork"
          Select case sObject
          end select

      case "Fontwork Shape"
          Select case sObject
          end select

      case "Form Design"
          Select case sObject
          end select

      case "Form Filter"
          Select case sObject
          end select

      case "Form Navigation"
          Select case sObject
          end select

      case "Full Screen"
          Select case sObject
          end select

      case "Gluepoints"
          Select case sObject
          end select

      case "Graphic Filter"
          Select case sObject
          end select

      case "Insert"
         Select case sObject
              case "Floating Frame" : fGetObjectImpress  = 6
              case "Plug-in"        : fGetObjectImpress  = 8
              case "Applet"         : fGetObjectImpress  = 9
              case "File"           : fGetObjectImpress  = 11
              case "Sound"          : fGetObjectImpress  = 14
              case "Video"          : fGetObjectImpress  = 15
          end select

      case "OLE-Object"
          Select case sObject
              case "Wrap Left"         : fGetObjectImpress  = 11
              case "Wrap Right"        : fGetObjectImpress  = 12
              case "Optimal Page Wrap" : fGetObjectImpress  = 16
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectImpress   = 0
          end select

      case "Standard"
          Select case sObject
              case "Load URL"                   : fGetObjectImpress  = 1
              case "New"                        : fGetObjectImpress  = 2
              case "New Document From Template" : fGetObjectImpress  = 3
              case "Open"                       : fGetObjectImpress  = 4
              case "Save"                       : fGetObjectImpress  = 5              
              case "Save As"                    : fGetObjectImpress  = 6
              case "Document as E-mail"         : fGetObjectImpress  = 7
                   '-----------------                                  8
              case "Edit File"                  : fGetObjectImpress  = 9
                   '-----------------                                  10
              case "Export Directly as PDF"     : fGetObjectImpress  = 11
              case "Print File Directly"        : fGetObjectImpress  = 12
                   '-----------------                                  13
              case "Spellcheck"                 : fGetObjectImpress  = 14              
              case "AutoSpellcheck"             : fGetObjectImpress  = 15
                   '-----------------                                  16
              case "Cut"                        : fGetObjectImpress  = 17
              case "Copy"                       : fGetObjectImpress  = 18
              case "Paste"                      : fGetObjectImpress  = 19
              case "Format Paintbrush"          : fGetObjectImpress  = 20
                   '-----------------                                  21
              case "Can't Undo"                 : fGetObjectImpress  = 22
              case "Can't Restore"              : fGetObjectImpress  = 23
                   '-----------------                                  24
              case "Chart"                      : fGetObjectImpress  = 25
              case "CALC"                : fGetObjectImpress  = 26
              case "Hyperlink"                  : fGetObjectImpress  = 27
                   '-----------------                                  28
              case "Display Grid"               : fGetObjectImpress  = 29
                   '-----------------                                  30
              case "Navigator"                  : fGetObjectImpress  = 31
              case "Zoom"                       : fGetObjectImpress  = 32
                   '-----------------                                  33
              case "StarOffice Help"            : fGetObjectImpress  = 34              
              case "What's This?"               : fGetObjectImpress  = 35
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectImpress   = 0
          end select

      case "Table"
          Select case sObject
              case "Table"     : fGetObjectImpress  = 22
              case "Columns"   : fGetObjectImpress  = 23
              case "Rows"      : fGetObjectImpress  = 24
          end select

      case "Text Object"
          Select case sObject
              case "Line Spacing 1"   : fGetObjectImpress  = 16
              case "Line Spacing 1.5" : fGetObjectImpress  = 17
              case "Line Spacing 2"   : fGetObjectImpress  = 18
              case "Font Color"       : fGetObjectImpress  = 20
              case "Left-To-Right"    : fGetObjectImpress  = 22
              case "Right-To-Left"    : fGetObjectImpress  = 23
          end select

      case "XML Form Design"
          Select case sObject
              case "Bring to Front"  : fGetObjectImpress  = 14
              case "Send to Back"    : fGetObjectImpress  = 15
              case "Group"           : fGetObjectImpress  = 17
              case "UnGroup"         : fGetObjectImpress  = 18
              case "Enter Group"     : fGetObjectImpress  = 19
              case "Exit Group"      : fGetObjectImpress  = 20
          end select

  end select

end function
