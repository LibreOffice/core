'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_toolbar_calc.inc,v $
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
'* owner : helge.delfs@sun.com
'*
'* short description : Toolbar tools - Calc
'*
'***************************************************************************************
'*
' #0 fGetObjectCalc
'*
'\*************************************************************************************

'*******************************************************
'* This function will get the location for image button 
'* in Commands in Tools/Customize/Toolbars from Calc
'*******************************************************
function fGetObjectCalc(sToolbar as String , sObject as String) as Integer

  Select case sToolbar        
      case "3D-Settings"
          Select case sObject
              case "Extrusion On/Off"   : fGetObjectCalc  = 1
                   '-----------------                       2
              case "Tilt Down"          : fGetObjectCalc  = 3
              case "Tilt Up"            : fGetObjectCalc  = 4
              case "Tilt Left"          : fGetObjectCalc  = 5
              case "Tilt Right"         : fGetObjectCalc  = 6
                   '-----------------                       7
              case "Depth"              : fGetObjectCalc  = 8
              case "Direction"          : fGetObjectCalc  = 9
              case "Lighting"           : fGetObjectCalc  = 10
              case "Surfact"            : fGetObjectCalc  = 11
              case "3D Color"           : fGetObjectCalc  = 12
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0          
          end select

      case "Align"
          Select case sObject
              case "Left"              : fGetObjectCalc  = 1
              case "Centered"          : fGetObjectCalc  = 2
              case "Right"             : fGetObjectCalc  = 3
              case "Top"               : fGetObjectCalc  = 4
              case "Center"            : fGetObjectCalc  = 5
              case "Bottom"            : fGetObjectCalc  = 6
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Basic Shapes"
          Select case sObject
          end select

      case "Block Arrows"
          Select case sObject
          end select

      case "Callouts"
          Select case sObject
          end select

      case "Color"
          Select case sObject
          end select

      case "Controls"
          Select case sObject
          end select

      case "Drawing"
          Select case sObject
              case "Select"            : fGetObjectCalc  = 1
                   '-----------------                      2
              case "Line"              : fGetObjectCalc  = 3
              case "Rectangle"         : fGetObjectCalc  = 4
              case "Ellipse"           : fGetObjectCalc  = 5
              case "Polygon"           : fGetObjectCalc  = 6
              case "Curve"             : fGetObjectCalc  = 7
              case "Freeform Line"     : fGetObjectCalc  = 8
              case "Arc"               : fGetObjectCalc  = 9
              case "Ellipse Pie"       : fGetObjectCalc  = 10
              case "Circle Segment"    : fGetObjectCalc  = 11
              case "Text"              : fGetObjectCalc  = 12
              case "Vertical Text"     : fGetObjectCalc  = 13
              case "Text Animation"    : fGetObjectCalc  = 14
              case "Callouts"          : fGetObjectCalc  = 15
              case "Vertical Callouts" : fGetObjectCalc  = 16
                   '-----------------                      17              
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Drawing Object Properties"
          Select case sObject
              case "Display Grid"       : fGetObjectCalc  = 21
              case "Snap to Grid"       : fGetObjectCalc  = 22
              case "Guides When Moving" : fGetObjectCalc  = 23
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
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
              case "Bring to Front"  : fGetObjectCalc  = 14
              case "Send to Back"    : fGetObjectCalc  = 15
              case "Group"           : fGetObjectCalc  = 17
              case "UnGroup"         : fGetObjectCalc  = 18
              case "Enter Group"     : fGetObjectCalc  = 19
              case "Exit Group"      : fGetObjectCalc  = 20
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Form Filter"
          Select case sObject
          end select

      case "Form Navigation"
          Select case sObject
          end select

      case "Form Object"
          Select case sObject
          end select

      case "Formatting" :
          Select case sObject
              case "Styles and Formatting"     : fGetObjectCalc  = 1
              case "Apply Style"               : fGetObjectCalc  = 2
              '----------------------                              3
              case "Font Name"                 : fGetObjectCalc  = 4
              '----------------------                              5
              case "Font Size"                 : fGetObjectCalc  = 6
              '----------------------                              7
              case "Bold"                      : fGetObjectCalc  = 8
              case "Italic"                    : fGetObjectCalc  = 9
              case "Underline"                 : fGetObjectCalc  = 10
              case "Underline:Double"          : fGetObjectCalc  = 11 
              '----------------------                              12
              case "Align Left"                : fGetObjectCalc  = 13
              case "Align Center Horizontally" : fGetObjectCalc  = 14
              case "Align Right"               : fGetObjectCalc  = 15
              case "Justified"                 : fGetObjectCalc  = 16
              case "Merge Cells"               : fGetObjectCalc  = 17
              '----------------------                              18
              case "Left-To-Right"             : fGetObjectCalc  = 19
              case "Right-To-Left"             : fGetObjectCalc  = 20
              '----------------------                              21
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Full Screen"
          Select case sObject
              case "Full Screen" : fGetObjectCalc  = 1
          end select

      case "Graphic Filter"
          Select case sObject
          end select

      case "Insert"
          Select case sObject
              case "Chart"          : fGetObjectCalc  = 18
              case "Insert Object"  : fGetObjectCalc  = 19
              case "Controls"       : fGetObjectCalc  = 20
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Insert Cell"
          Select case sObject
          end select

      case "Insert Object"
          Select case sObject
          end select

      case "Media Playback"
          Select case sObject
          end select

      case "More Controls"
          Select case sObject
          end select

      case "Picture"
          Select case sObject
          end select

      case "Standard"
          Select case sObject
              case "Load URL"                   : fGetObjectCalc  = 1
              case "New"                        : fGetObjectCalc  = 2
              case "New Document From Template" : fGetObjectCalc  = 3
              case "Open"                       : fGetObjectCalc  = 4
              case "Save"                       : fGetObjectCalc  = 5              
              case "Save As"                    : fGetObjectCalc  = 6
              case "Document as E-mail"         : fGetObjectCalc  = 7
                   '-----------------                               8
              case "Edit File"                  : fGetObjectCalc  = 9
                   '-----------------                               10
              case "Export Directly as PDF"     : fGetObjectCalc  = 11
              case "Print File Directly"        : fGetObjectCalc  = 12
              case "Page Rreview"               : fGetObjectCalc  = 13
                   '-----------------                               14
              case "Spellcheck"                 : fGetObjectCalc  = 15              
              case "AutoSpellcheck"             : fGetObjectCalc  = 16
                   '-----------------                               17
              case "Cut"                        : fGetObjectCalc  = 18
              case "Copy"                       : fGetObjectCalc  = 19
              case "Paste"                      : fGetObjectCalc  = 20
              case "Format Paintbrush"          : fGetObjectCalc  = 21
                   '-----------------                               22
              case "Can't Undo"                 : fGetObjectCalc  = 23
              case "Can't Restore"              : fGetObjectCalc  = 24
                   '-----------------                               25
              case "Hyperlink"                  : fGetObjectCalc  = 26
              case "Sort Ascending"             : fGetObjectCalc  = 27
              case "Sort Descending"            : fGetObjectCalc  = 28
                   '-----------------                               29
              case "Insert Chart"               : fGetObjectCalc  = 30
              case "Show Draw Functions"        : fGetObjectCalc  = 31
                   '-----------------                               32
              case "Find & Replace"             : fGetObjectCalc  = 33
              case "Navigator"                  : fGetObjectCalc  = 34
              case "Gallery"                    : fGetObjectCalc  = 35
              case "Data Sources"               : fGetObjectCalc  = 36
              case "Zoom"                       : fGetObjectCalc  = 37
                   '-----------------                               38
              case "StarOffice Help"            : fGetObjectCalc  = 39            
              case "What's This?"               : fGetObjectCalc  = 40
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectCalc   = 0
          end select

      case "Standard(Viewing Mode)"
          Select case sObject
          end select

      case "Stars and Banners"
          Select case sObject
          end select

      case "Symbol Shapes"
          Select case sObject
          end select

      case "Text Formatting"
          Select case sObject
          end select

      case "Tools"
          Select case sObject
          end select
        
      case "previewbar"
          Select case sObject
          end select

  end select

end function
