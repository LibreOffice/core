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
'* short description : Toolbar tools - Writer
'*
'***************************************************************************************
'*
' #0 fGetObjectWriter
' #0 fGetObjectCalc
'*
'\*************************************************************************************

'*******************************************************
'* This function will get the location for image button 
'* in Commands in Tools/Customize/Toolbars from Writer
'*******************************************************
function fGetObjectWriter(sToolbar as String , sObject as String) as Integer

  Select case sToolbar
      case "3D-Settings"
          Select case sObject
              case "Extrusion On/Off"   : fGetObjectWriter  = 1
                   '-----------------                         2
              case "Tilt Down"          : fGetObjectWriter  = 3
              case "Tilt Up"            : fGetObjectWriter  = 4
              case "Tilt Left"          : fGetObjectWriter  = 5
              case "Tilt Right"         : fGetObjectWriter  = 6
                   '-----------------                         7
              case "Depth"              : fGetObjectWriter  = 8
              case "Direction"          : fGetObjectWriter  = 9
              case "Lighting"           : fGetObjectWriter  = 10
              case "Surfact"            : fGetObjectWriter  = 11
              case "3D Color"           : fGetObjectWriter  = 12
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Align"
          Select case sObject
              case "Left"              : fGetObjectWriter  = 1
              case "Centered"          : fGetObjectWriter  = 2
              case "Right"             : fGetObjectWriter  = 3
              case "Top"               : fGetObjectWriter  = 4
              case "Center"            : fGetObjectWriter  = 5
              case "Bottom"            : fGetObjectWriter  = 6
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Basic Shapes"
          Select case sObject
              case "Rectangle"          : fGetObjectWriter  = 1
              case "Rectangle,Rounded"  : fGetObjectWriter  = 2
              case "Square"             : fGetObjectWriter  = 3
              case "Square,Rounded"     : fGetObjectWriter  = 4
              case "Circle"             : fGetObjectWriter  = 5
              case "Ellipse"            : fGetObjectWriter  = 6
                   '-----------------                         7
              case "Circle Pie"         : fGetObjectWriter  = 8
              case "Isosceles Triangle" : fGetObjectWriter  = 9
              case "Right Triangle"     : fGetObjectWriter  = 10
              case "Trapezoid"          : fGetObjectWriter  = 11
              case "Diamond"            : fGetObjectWriter  = 12
              case "Parallelogram"      : fGetObjectWriter  = 13
                   '-----------------                         14
              case "Regular Pentagon"   : fGetObjectWriter  = 15
              case "Hexagon"            : fGetObjectWriter  = 16
              case "Octagon"            : fGetObjectWriter  = 17
              case "Cross"              : fGetObjectWriter  = 18
              case "Ring"               : fGetObjectWriter  = 19
              case "Block Arc"          : fGetObjectWriter  = 20
                   '-----------------                         21
              case "Cylinder"           : fGetObjectWriter  = 22
              case "Cube"               : fGetObjectWriter  = 23
              case "Rolded Corner"      : fGetObjectWriter  = 24
              case "Rrame"              : fGetObjectWriter  = 25                   
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Block Arrows"
          Select case sObject
          end select

      case "Bullets and Numbering"
          Select case sObject
          end select

      case "Callouts"
          Select case sObject
              case "Rectanguar Callout"          : fGetObjectWriter  = 1
              case "Rounded Rectanguar Callout"  : fGetObjectWriter  = 2
              case "Round Callout"               : fGetObjectWriter  = 3
              case "Cloud"                       : fGetObjectWriter  = 4
              case "Line Callout 1"              : fGetObjectWriter  = 5
              case "Line Callout 2"              : fGetObjectWriter  = 6
              case "Line Callout 3"              : fGetObjectWriter  = 7
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Color"
          Select case sObject
              case "Red"               : fGetObjectWriter  = 1
                   '-----------------                        2
              case "Green"             : fGetObjectWriter  = 3
                   '-----------------                        4
              case "Blue"              : fGetObjectWriter  = 5
                   '-----------------                        6             
              case "Brightness"        : fGetObjectWriter  = 7
                   '-----------------                        8             
              case "Contrast"          : fGetObjectWriter  = 9
                   '-----------------                        10
              case "Gamma"             : fGetObjectWriter  = 11
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Controls"
          Select case sObject
          end select

      case "Database Controls"
          Select case sObject
          end select

      case "Database Form Design"
          Select case sObject
              case "Change Anchor"   : fGetObjectWriter  = 12
              case "Bring To Front"  : fGetObjectWriter  = 13
              case "Send To Back"    : fGetObjectWriter  = 14
              case "Group"           : fGetObjectWriter  = 15
              case "UnGroup"         : fGetObjectWriter  = 18
              case "Enter Group"     : fGetObjectWriter  = 19
              case "Exit Group"      : fGetObjectWriter  = 20
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Drawing"
          Select case sObject
              case "Select"            : fGetObjectWriter  = 1
                   '-----------------                        2
              case "Line"              : fGetObjectWriter  = 3
              case "Rectangle"         : fGetObjectWriter  = 4
              case "Ellipse"           : fGetObjectWriter  = 5
              case "Polygon"           : fGetObjectWriter  = 6
              case "Curve"             : fGetObjectWriter  = 7
              case "Freeform Line"     : fGetObjectWriter  = 8
              case "Arc"               : fGetObjectWriter  = 9
              case "Ellipse Pie"       : fGetObjectWriter  = 10
              case "Circle Segment"    : fGetObjectWriter  = 11
              case "Text"              : fGetObjectWriter  = 12
              case "Vertical Text"     : fGetObjectWriter  = 13
              case "Text Animation"    : fGetObjectWriter  = 14
              case "Callouts"          : fGetObjectWriter  = 13
              case "Vertical Callouts" : fGetObjectWriter  = 14
                   '-----------------                        15
              case "Basic Shapes"      : fGetObjectWriter  = 16
              case "Symbol Shapes"     : fGetObjectWriter  = 17
              case "Block Arrows"      : fGetObjectWriter  = 18
              case "Flowcharts"        : fGetObjectWriter  = 19
              case "Callouts2"         : fGetObjectWriter  = 20
              case "Stars"             : fGetObjectWriter  = 21
                   '-----------------                        22
              case "Points"            : fGetObjectWriter  = 23
                   '-----------------                        24
              case "Fontwork Gallery"  : fGetObjectWriter  = 25
              case "Insert Graphics"   : fGetObjectWriter  = 26
                   '-----------------                        27
              case "Extrusion On/Off"  : fGetObjectWriter  = 28
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Drawing Object Properties"
          Select case sObject
              case "Line"                 : fGetObjectWriter  = 1
              case "Arrow Style"          : fGetObjectWriter  = 2
                   '-----------------                           3
              case "Line Style"           : fGetObjectWriter  = 4
                   '-----------------                           5
              case "Line Width"           : fGetObjectWriter  = 6
                   '-----------------                           7              
              case "Line Color"           : fGetObjectWriter  = 8
                   '-----------------                           9
              case "Area"                 : fGetObjectWriter  = 10
                   '-----------------                           11
              case "Area Style/Filling"   : fGetObjectWriter  = 12              
                   '-----------------                           13
              case "Object rotation mode" : fGetObjectWriter  = 14
              case "Display Grid"         : fGetObjectWriter  = 15
              case "Snap to Grid"         : fGetObjectWriter  = 16
              case "Guides When Moving"   : fGetObjectWriter  = 17
                   '-----------------                           18
              case "Wrap Off"             : fGetObjectWriter  = 19
              case "Page Wrap"            : fGetObjectWriter  = 20
              case "Optimal Page Wrap"    : fGetObjectWriter  = 21
              case "Wrap Left"            : fGetObjectWriter  = 22
              case "Wrap Right"           : fGetObjectWriter  = 23
              case "Wrap Through"         : fGetObjectWriter  = 24
              case "Contour"              : fGetObjectWriter  = 25
                   '-----------------                           26
              case "To Foreground"        : fGetObjectWriter  = 27
              case "To Background"        : fGetObjectWriter  = 28
              case "Bring to Front"       : fGetObjectWriter  = 29
              case "Send to Back"         : fGetObjectWriter  = 30
              case "Alignment"            : fGetObjectWriter  = 31
                   '-----------------                           32
              case "Change Anchor"        : fGetObjectWriter  = 33
                   '-----------------                           34
              case "Ungroup"              : fGetObjectWriter  = 35
              case "Group"                : fGetObjectWriter  = 36                   
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Edit Points"
          Select case sObject
          end select

      case "Flowchart"
          Select case sObject
          end select

      case "Fontwork"
          Select case sObject
              case "Fontwork Gallery"             : fGetObjectWriter  = 1
                   '-----------------                                   2
              case "Fontwork Shape"               : fGetObjectWriter  = 3
              case "Fontwork Same Letter Heights" : fGetObjectWriter  = 4
                   '-----------------                                   5
              case "Fontwork Alignment"           : fGetObjectWriter  = 6
              case "Fontwork Character Spacing"   : fGetObjectWriter  = 7
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Fontwork Shape"
          Select case sObject
          end select

      case "Form Design"
          Select case sObject
              case "Bring to Front"  : fGetObjectWriter  = 14
              case "Send to Back"    : fGetObjectWriter  = 15
              case "Group"           : fGetObjectWriter  = 17
              case "UnGroup"         : fGetObjectWriter  = 18
              case "Enter Group"     : fGetObjectWriter  = 19
              case "Exit Group"      : fGetObjectWriter  = 20
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Form Filter"
          Select case sObject
              case "Apply Form-Based Filter" : fGetObjectWriter  = 1
              case "Filter Navigation"       : fGetObjectWriter  = 2
                   '-----------------                              3
              case "Close"                   : fGetObjectWriter  = 4
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Form Navigation"
          Select case sObject
          end select

      case "Form Object"
          Select case sObject
          end select

      case "Formatting"
          Select case sObject
              case "Sytles and Formatting" : fGetObjectWriter  = 1
                   '-----------------                            2
              case "Apply Style"           : fGetObjectWriter  = 3
                   '-----------------                            4
              case "Font Name"             : fGetObjectWriter  = 5
                   '-----------------                            6
              case "Font Size"             : fGetObjectWriter  = 7
                   '-----------------                            8
              case "Bold"                  : fGetObjectWriter  = 9
              case "Italic"                : fGetObjectWriter  = 10
              case "Underline"             : fGetObjectWriter  = 11
              case "Superscript"           : fGetObjectWriter  = 12
              case "Subscript"             : fGetObjectWriter  = 13
                   '-----------------                            14
              case "Align Left"            : fGetObjectWriter  = 15
              case "Centered"              : fGetObjectWriter  = 16
              case "Align Right"           : fGetObjectWriter  = 17
              case "Justfied"              : fGetObjectWriter  = 18
                   '-----------------                            19
              case "Left-To-Right"         : fGetObjectWriter  = 20
              case "Right-To-Left"         : fGetObjectWriter  = 21
                   '-----------------                            22
              case "Line Spacing 1"        : fGetObjectWriter  = 23
              case "Line Spacing 1.5"      : fGetObjectWriter  = 24
              case "Line Spacing 2"        : fGetObjectWriter  = 25
                   '-----------------                            26
              case "Numbering Of/Off"      : fGetObjectWriter  = 27
              case "Bullets Of/Off"        : fGetObjectWriter  = 28
              case "Decrease Indent"       : fGetObjectWriter  = 29
              case "Increase Indent"       : fGetObjectWriter  = 30
              case "Increase Font"         : fGetObjectWriter  = 31
              case "Reduce Font"           : fGetObjectWriter  = 32
                   '-----------------                            33
              case "Font Color"            : fGetObjectWriter  = 34
              case "Highlighting"          : fGetObjectWriter  = 35
              case "Background Color"      : fGetObjectWriter  = 36
                   '-----------------                            37
              case "Select All"            : fGetObjectWriter  = 38
              case "Character"             : fGetObjectWriter  = 39
              case "Paragraph"             : fGetObjectWriter  = 40
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Frame"
          Select case sObject
              case "Optimal Page Wrap" : fGetObjectWriter  = 5
              case "Wrap Left"         : fGetObjectWriter  = 6
              case "Wrap Right"        : fGetObjectWriter  = 7
              case "Wrap Through"      : fGetObjectWriter  = 8
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Full Screen"
          Select case sObject
          end select

      case "Graphic Filter"
          Select case sObject
          end select

      case "Insert"
          Select case sObject
              case "Chart"          : fGetObjectWriter  = 18
              case "Insert Object"  : fGetObjectWriter  = 19
              case "Controls"       : fGetObjectWriter  = 20
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
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

      case "More Database Controls"
          Select case sObject
          end select

      case "More XML Form Controls"
          Select case sObject
          end select

      case "OLE-Object"
          Select case sObject
              case "Wrap Left"         : fGetObjectWriter  = 11
              case "Wrap Right"        : fGetObjectWriter  = 12
              case "Optimal Page Wrap" : fGetObjectWriter  = 16
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Optimize"
          Select case sObject
          end select

      case "Page Preview"
          Select case sObject
          end select

      case "Picture"
          Select case sObject
          end select

      case "Standard"
          Select case sObject
              case "Load URL"                   : fGetObjectWriter  = 1
              case "New"                        : fGetObjectWriter  = 2
              case "New Document From Template" : fGetObjectWriter  = 3
              case "Open"                       : fGetObjectWriter  = 4
              case "Save"                       : fGetObjectWriter  = 5              
              case "Save As"                    : fGetObjectWriter  = 6
              case "Document as E-mail"         : fGetObjectWriter  = 7
                   '-----------------                                 8
              case "Edit File"                  : fGetObjectWriter  = 9
                   '-----------------                                 10
              case "Export Directly as PDF"     : fGetObjectWriter  = 11
              case "Print File Directly"        : fGetObjectWriter  = 12
              case "Page Rreview"               : fGetObjectWriter  = 13
                   '-----------------                                 14
              case "Spellcheck"                 : fGetObjectWriter  = 15              
              case "AutoSpellcheck"             : fGetObjectWriter  = 16
                   '-----------------                                 17
              case "Cut"                        : fGetObjectWriter  = 18
              case "Copy"                       : fGetObjectWriter  = 19
              case "Paste"                      : fGetObjectWriter  = 20
              case "Format Paintbrush"          : fGetObjectWriter  = 21
                   '-----------------                                 22
              case "Can't Undo"                 : fGetObjectWriter  = 23
              case "Can't Restore"              : fGetObjectWriter  = 24
                   '-----------------                                 25
              case "Hyperlink"                  : fGetObjectWriter  = 26
              case "Table"                      : fGetObjectWriter  = 27
              case "Show Draw Functions"        : fGetObjectWriter  = 28
                   '-----------------                                 29
              case "Find & Replace"             : fGetObjectWriter  = 30
              case "Navigator"                  : fGetObjectWriter  = 31
              case "Gallery"                    : fGetObjectWriter  = 32
              case "Data Sources"               : fGetObjectWriter  = 33
              case "Nonprinting Characters"     : fGetObjectWriter  = 34
              case "Zoom"                       : fGetObjectWriter  = 35
                   '-----------------                                 36
              case "StarOffice Help"            : fGetObjectWriter  = 37              
              case "What's This?"               : fGetObjectWriter  = 38
              case else : QAErrorLog "The test does not support Object : " + sObject
                          fGetObjectWriter   = 0
          end select

      case "Standard(Viewing MOde)"
          Select case sObject
          end select

      case "Stars and Banners"
          Select case sObject
          end select

      case "Symbol Shapes"
          Select case sObject
          end select

      case "Table"
          Select case sObject
              case "Table"     : fGetObjectWriter  = 22
              case "Columns"   : fGetObjectWriter  = 23
              case "Rows"      : fGetObjectWriter  = 24
          end select

      case "Text Object"
          Select case sObject
              case "Line Spacing 1"   : fGetObjectWriter  = 16
              case "Line Spacing 1.5" : fGetObjectWriter  = 17
              case "Line Spacing 2"   : fGetObjectWriter  = 18
              case "Font Color"       : fGetObjectWriter  = 20
              case "Left-To-Right"    : fGetObjectWriter  = 22
              case "Right-To-Left"    : fGetObjectWriter  = 23
          end select

      case "XML Form Design"
          Select case sObject
              case "Bring to Front"  : fGetObjectWriter  = 14
              case "Send to Back"    : fGetObjectWriter  = 15
              case "Group"           : fGetObjectWriter  = 17
              case "UnGroup"         : fGetObjectWriter  = 18
              case "Enter Group"     : fGetObjectWriter  = 19
              case "Exit Group"      : fGetObjectWriter  = 20
          end select

  end select

end function


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



