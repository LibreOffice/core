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
' **
' ** owner : gregor.hartmann@oracle.com
' **
' ** short description : Tools to draw and select form controls in basic-ide
' **
'\******************************************************************************

private const ICONTROLCOUNT = 22

'*******************************************************************************

function hGetControlParams( cParam as string ) as integer

    '///<h3>Retrieve basic parameters to draw formcontrols to a dialog</h3>
    '///<i>All values are in percent relative to the window size.
    '///+ All values are optimized for 1024x768 pixels screen resolution but 
    '///+ have been tested successfully with 1280x1024 and 800x600</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the coordinate (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;XOREGO&quot; (Upper left corner)</li>
    '///+<li>&quot;YOREGO&quot; (Upper left corner)</li>
    '///+<li>&quot;XDIST&quot; (Distance between the upper left corners of neighbor controls)</li>
    '///+<li>&quot;YDIST&quot; (Distance between the upper left corners of neighbor controls)</li>
    '///+<li>&quot;XSIZE&quot; (Width of control)</li>
    '///+<li>&quot;YSIZE&quot; (Heighth of the control)</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Coordinate/Distance/Size in percent of window size (integer)</li>
    '///<ul>
    '///+<li>A number between 1 and 100</li>
    '///+<li>0 on error (invalid function parameter)</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    cParam = ucase( cParam ) 

    '///+<li>Currently following values are defined:</li>
    '///<ul>
    select case ( cParam )
    '///+<li>XOREGO = 31</li>
    case "XOREGO" : hGetControlParams() = 31
    '///+<li>XDIST = 8</li>
    case "XDIST"  : hGetControlParams() = 8
    '///+<li>XSIZE = 6</li>
    case "XSIZE"  : hGetControlParams() = 6
    '///+<li>YOREGO = 30</li>
    case "YOREGO" : hGetControlParams() = 30
    '///+<li>YDIST = 7</li>
    case "YDIST"  : hGetControlParams() = 7
    '///+<li>YSIZE = 5</li>
    case "YSIZE"  : hGetControlParams() = 5   
    '///+<li>Incorrect function parameter = 0</li>
    case else     : hGetControlParams() = 0
    '///</ul>
    end select
    '///</ul>
    
end function

'*******************************************************************************

function hGetControlName( iControl as integer ) as string

    '///<h3>A function to deliver a speaking name for all form controls</h3>
    '///<i>Note that the numbers of the controls are unique</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description below</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Name for a control (string)</li>
    '///<ul>
    '///+<li>Name for a control (may contain whitespaces)</li>
    '///+<li>An empty string in case of an invalid function parameter</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Currently following control are defined:</li>
    '///<ol>
    select case iControl
    '///+<li>Push Button</li>
    case 1  : hGetControlName() = "Push Button"
    '///+<li>Image Control</li>
    case 2  : hGetControlName() = "Image Control"
    '///+<li>Check Box</li>
    case 3  : hGetControlName() = "Check Box"
    '///+<li>Radio Button</li>
    case 4  : hGetControlName() = "Radio Button"
    '///+<li>Fixed Text</li>
    case 5  : hGetControlName() = "Fixed Text"
    '///+<li>Edit Field</li>
    case 6  : hGetControlName() = "Edit Field"
    '///+<li>List Box</li>
    case 7  : hGetControlName() = "List Box"
    '///+<li>Combo Box</li>
    case 8  : hGetControlName() = "Combo Box"
    '///+<li>Vertical ScrollBar</li>
    case 9  : hGetControlName() = "Vertical ScrollBar"
    '///+<li>Horizontal ScrollBar</li>
    case 10 : hGetControlName() = "Horizontal ScrollBar"
    '///+<li>Frame</li>
    case 11 : hGetControlName() = "Frame"
    '///+<li>Progress Bar</li>
    case 12 : hGetControlName() = "Progress Bar"
    '///+<li>Vertical Fixed Line</li>
    case 13 : hGetControlName() = "Vertical Fixed Line"
    '///+<li>Horizontal Fixed Line</li>
    case 14 : hGetControlName() = "Horizontal Fixed Line"
    '///+<li>Date Field</li>
    case 15 : hGetControlName() = "Date Field"
    '///+<li>Time Field</li>
    case 16 : hGetControlName() = "Time Field"
    '///+<li>Numeric Field</li>
    case 17 : hGetControlName() = "Numeric Field"
    '///+<li>Currency Field</li>
    case 18 : hGetControlName() = "Currency Field"
    '///+<li>Form Field</li>
    case 19 : hGetControlName() = "Form Field"
    '///+<li>Pattern Field</li>
    case 20 : hGetControlName() = "Pattern Field"
    '///+<li>File Control</li>
    case 21 : hGetControlName() = "File Control"
    '///+<li>Tree Control</li>
    case 22 : hGetControlName() = "Tree Control"
    '///</ol>
    '///+<li>&quot;&quot; for function parameter &lt; 1 or &gt; 22 </li>
    case else : sControl = ""
    end select
    '///</ul>
    
end function

'*******************************************************************************

function hInsertControl( iControl as integer ) as string

    '///<h3>Function to insert one of the BASIC formcontrols by index</h3>
    '///<i>Note that the numbers of the controls are unique</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description below</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Name for a control (string)</li>
    '///<ul>
    '///+<li>Name for a control (may contain whitespaces)</li>
    '///+<li>An empty string in case of an invalid function parameter</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hInsertControl::"
    
    Kontext "ToolsCollectionBar"

    '///+<li>Click on the requested button on the ToolsCollectionBar:</li>
    '///<ol>
    select case iControl
    '///+<li>Push Button</li>
    case 1  : InsPushButton.click()
    '///+<li>Image Control</li>
    case 2  : InsImgCtrl.click()
    '///+<li>Check Box</li>
    case 3  : InsCheckbox.click()
    '///+<li>Radio Button</li>
    case 4  : InsRadioButton.click()
    '///+<li>Fixed Text</li>
    case 5  : InsFixedText.click()
    '///+<li>Edit Field</li>
    case 6  : InsEditField.click()
    '///+<li>List Box</li>
    case 7  : InsListbox.click()
    '///+<li>Combo Box</li>
    case 8  : InsComboBox.click()
    '///+<li>Vertical ScrollBar</li>
    case 9  : InsScrollBarV.click()
    '///+<li>Horizontal ScrollBar</li>
    case 10 : InsScrollBarH.click()
    '///+<li>Frame</li>
    case 11 : InsFrame.click()
    '///+<li>Progress Bar</li>
    case 12 : InsProgressbar.click()
    '///+<li>Vertical Fixed Line</li>
    case 13 : InsFixedLineV.click()
    '///+<li>Horizontal Fixed Line</li>
    case 14 : InsFixedLineH.click()
    '///+<li>Date Field</li>
    case 15 : InsDateField.click()
    '///+<li>Time Field</li>
    case 16 : InsTimeField.click()
    '///+<li>Numeric Field</li>
    case 17 : InsNumField.click()
    '///+<li>Currency Field</li>
    case 18 : InsCurrencyField.click()
    '///+<li>Form Field</li>
    case 19 : InsFormField.click()
    '///+<li>Pattern Field</li>
    case 20 : InsPatternField.click()
    '///+<li>File Control</li>
    case 22 : InsFileCtrl.click()
    '///+<li>Tree Control</li>
    case 22 : InsTreeControl.click()
    end select
    '///</ol>
    '///+<li>&quot;&quot; for function parameter &lt; 1 or &gt; 22 </li>
    
    hInsertControl() = hGetControlName( iControl )
    '///</ul>
    
end function

'*******************************************************************************

function hDrawControlOnDialog( iControl as integer ) as string

    '///<h3>Draw a control on a dialog at a fixed position</h3>
    '///<i>Note that the numbers of the controls are unique</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Name for a control (string)</li>
    '///<ul>
    '///+<li>Name for a control (may contain whitespaces)</li>
    '///+<li>An empty string in case of an invalid function parameter</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hDrawControlOnDialog::"
    
    dim sControl as string   ' The name of the current control
    dim brc as boolean       ' some returnvalue
    
    ' coordinates of the controls on the dialog in the dialog-editor
    dim iXO as integer
    dim iYO as integer
    dim iXE as integer
    dim iYE as integer
    
    '///+<li>determine where the control is to be painted (hGetControlPos...)</li>
    iXO = hGetControlPosXO( iControl )
    iYO = hGetControlPosYO( iControl )
    iXE = hGetControlPosXE( iControl )
    iYE = hGetControlPosYE( iControl )
    
    '///+<li>click the desired control</li> 
    sControl = hInsertControl( iControl )
    printlog( CFN & " Index=" & iControl & _
                    " at XO=" & iXO & _
                    " XE=" & iXE & _
                    " YO=" & iYO & _
                    " YE=" & iYE & _
                    " : " & sControl )

    '///+<li>Draw the control (using hDrawControl(...))</li>
    brc = hDrawControl( iXO, iYO, iXE, iYE )    
    hDrawControlOnDialog() = sControl
    '///</ul>
    
end function

'*******************************************************************************

function hDrawControl( xPos as integer, _
                       yPos as integer, _
                       xEnd as integer, _
                       yEnd as integer ) as boolean
                       
    '///<h3>Draw a control on the dialog pane in the dialog editor</h3>
    '///<i>Starting point: Basic IDE/Dialog editor</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>    
    '///<i>Note: All units are in percent of the relative to the current window size</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>X-Orego (Upper left corner) (integer)</li>
    '///<ul>
    '///+<li>Min = 0</li>
    '///+<li>Max = 100 (allowed but not useful)</li>
    '///</ul>
    '///+<li>Y-Orego (Upper left corner) (integer)</li>
    '///<ul>
    '///+<li>Min = 0</li>
    '///+<li>Max = 100 (allowed but not useful)</li>
    '///</ul>
    '///+<li>X-End (Lower right corner) (integer)</li>
    '///<ul>
    '///+<li>Min = 0</li>
    '///+<li>Max = 100 (allowed but not useful)</li>
    '///</ul>
    '///+<li>Y-End (Lower right corner) (integer)</li>
    '///<ul>
    '///+<li>Min = 0</li>
    '///+<li>Max = 100 (allowed but not useful)</li>
    '///</ul>    
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hDrawControl::"   
    
    '///+<li>Set context to Basic IDE</li>
    Kontext "BasicIDE"
    
    '///+<li>Draw the control using mouse actions</li>
    '///<ol>
    '///+<li>Mouse down on pos X/Y-Orego</li>
    '///+<li>Mouse move to pos X/Y-End</li>
    '///+<li>Mouse up on pos X/Y-End</li>
    '///</ol>
    '///</ul>

    autoexecute = false
    DialogWindow.MouseUp( 20 , 20 ) : wait 100
    
    try
        DialogWindow.MouseMove ( xPos, yPos ) : wait 100
        DialogWindow.MouseDown ( xPos, yPos ) : wait 100
        DialogWindow.MouseMove ( xEnd, yEnd ) : wait 100
        DialogWindow.MouseUp   ( xEnd, yEnd ) : wait 100
        hDrawControl() = true
    catch
        warnlog( "#i39852# " & CFN & "Unable to complete mouseactions on dialog" )
        hDrawControl() = false
    endcatch
    autoexecute = true

end function

'*******************************************************************************

function hGetControlPosXO( iControl as integer ) as integer

    '///<h3>Retrieve the upper left X-coordinate for a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>X-Orego in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    dim xOffset as integer
        xOffset = hGetControlParams( "xorego" )
        
    dim xDistance as integer
        xDistance = hGetControlParams( "xdist" )
    
    '///+<li>Define an offset for the control depending on its ID</li>
    '///<ul>
    '///+<li>&lt; 7 : Column one</li>
    '///+<li>7 ... 12 : Column two</li>
    '///+<li>13 ... 18 : Column three</li>
    '///+<li>&gt; 18 : Column four</li>
    '///</ul>
    '///</ul>

    select case ( iControl )
    case  1,  2,  3,  4,  5,  6 : hGetControlPosXO() = xOffset
    case  7,  8,  9, 10, 11, 12 : hGetControlPosXO() = xOffset + 1 * xDistance
    case 13, 14, 15, 16, 17, 18 : hGetControlPosXO() = xOffset + 2 * xDistance
    case 19, 20, 21, 22, 23, 24 : hGetControlPosXO() = xOffset + 3 * xDistance
    end select
        
end function
    
'*******************************************************************************

function hGetControlPosYO( iControl as integer ) as integer

    '///<h3>Retrieve the upper left Y-coordinate for a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 21, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Y-Orego in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    dim yOffset as integer
        yOffset = hGetControlParams( "yorego" )
        
    dim yDistance as integer
        yDistance = hGetControlParams( "ydist" )
        
    '///+<li>Define an offset for the control depending on its ID</li>
    '///<ul>
    '///+<li>1 ,  7 , 13 , 19 : Row one</li>
    '///+<li>2 ,  8 , 14 , 20 : Row two</li>
    '///+<li>3 ,  9 , 15 , 21 : Row three</li>
    '///+<li>4 , 10 , 16 , 22 : Row four</li>
    '///+<li>5 , 11 , 17 : Row five</li>
    '///+<li>6 , 12 , 18 : Row six</li>
    '///</ul>
    '///</ul>

    select case ( iControl )
    case  1,  7, 13, 19 : hGetControlPosYO() = yOffset
    case  2,  8, 14, 20 : hGetControlPosYO() = yOffset + 1 * yDistance
    case  3,  9, 15, 21 : hGetControlPosYO() = yOffset + 2 * yDistance
    case  4, 10, 16, 22 : hGetControlPosYO() = yOffset + 3 * yDistance
    case  5, 11, 17, 23 : hGetControlPosYO() = yOffset + 4 * yDistance
    case  6, 12, 18, 24 : hGetControlPosYO() = yOffset + 5 * yDistance
    end select
    
end function

'*******************************************************************************

function hGetControlPosXE( iControl as integer ) as integer

    '///<h3>Retrieve the lower right X-coordinate for a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>X-End in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    '///+<li>Get pos for X-Orego, add &quot;XSIZE&quot;</li>
    hGetControlPosXE() = hGetControlPosXO( iControl ) + _
                         hGetControlParams( "xsize" )
    '///</ul>
    
end function

'*******************************************************************************

function hGetControlPosYE( iControl as integer ) as integer

    '///<h3>Retrieve the lower right Y-coordinate for a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Y-End in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    

    '///+<li>Get pos for Y-Orego, add &quot;YSIZE&quot;</li>
    hGetControlPosYE() = hGetControlPosYO( iControl ) + _
                         hGetControlParams( "ysize" )
    '///</ul>
    
end function

'*******************************************************************************

function hGetControlPosXM( iControl as integer ) as integer

    '///<h3>Retrieve the center (X) of a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>X-Center in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>        

    dim XO as integer
    dim XE as integer
    
    '///+<li>Find X-Orego</li>
    XO = hGetControlPosXO( iControl )
    
    '///+<li>Find X-End</li>
    XE = hGetControlPosXE( iControl )
    
    '///+<li>Calculate the distance, find the middle between the two</li>
    hGetControlPosXM() = XO + 0.5 * ( XE - XO )
    '///</ul>

end function 

'*******************************************************************************

function hGetControlPosYM( iControl as integer ) as integer

    '///<h3>Retrieve the center (Y) of a control</h3>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...). The coordinates
    '///+ returned by this function can be used to draw and to select a control.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Y-Center in percent of window size (integer)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>        

    dim YO as integer
    dim YE as integer
    
    '///+<li>Find Y-Orego</li>
    YO = hGetControlPosYO( iControl )
    
    '///+<li>Find Y-End</li>
    YE = hGetControlPosYE( iControl )
    
    '///+<li>Calculate the distance, find the middle between the two</li>
    hGetControlPosYM() = YO + 0.5 * ( YE - YO )  
    '///</ul>
    
end function

'*******************************************************************************

function hSelectControl( iControl as integer ) as boolean

    '///<h3>Function to select one of the BASIC formcontrols by index</h3>
    '///<i>Note: Refer to the inline documentation for implementation details</i><br>
    '///<i>In most cases it is desired to place multiple controls on a single 
    '///+ dialog pane. To prevent the controls from overlapping each other
    '///+ they are arranged in rows and columns. Each control is identified
    '///+ by a unique number (see description for hInsertControl(...)). The 
    '///+ dimensions are defined in hGetControlParams(...).</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of the control (integer)</li>
    '///<ul>
    '///+<li>Any number between 1 and 22, see description for hInsertControl()</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the properties-button on ToolsCollectionBar is enabled</li>
    '///+<li>FALSE in any other case</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>        

    dim xPos as integer
    dim yPos as integer
    dim iCurrentSelectionMethod as integer

    const SELECT_MIDDLE       = 1 ' click into the middle of the control
    const SELECT_UPPER_LEFT   = 2 ' click the upper left corner of the control
    const SELECT_LOWER_RIGHT  = 3 ' click the lower right corner of the control
    const SELECT_FRAME_AROUND = 4 ' select by drawing a frame around the control

    const EXTRA_FRAME_SIZE    = 1 ' one percent in-/outside the border of the control
    const SELECTION_METHODS   = 4 ' this function sports four ways of seleting a control
    const REPEAT_COUNT        = 5 ' number of times to send a keystroke to the dialog window

    hSelectControl() = false

    '///+<li>Verify that the ToolsCollectionBar is visible. if not: Abort</li>
    kontext "ToolsCollectionBar"
    if ( not ToolsCollectionBar.exists() ) then
        warnlog( "The ToolsCollectionBar is not visible, open it first" )
        exit function
    endif
    
    '///+<li>Enable the selection mode on ToolsCollectionBar</li>
    SelectMode.click()
    
    ' Note: The controls have areas where they ignore a mouseclick. E.g. the
    ' framecontrol can only be grabbed at the border, you won't be able to 
    ' select it by clicking in the middle. Furthermore - even if the Dialog-
    ' editor-window is maximized - we might still miss the upper left corner.
    ' So what happens in this loop is that we try to click in the middle of the
    ' control. This works in 21 out of 22 cases. If it fails, we try to grab the 
    ' border, first upper left corner then lower right. If this still fails,
    ' we try to select the control by using a rectangle selection around the
    ' control. If this last resort fails, the function exits gracefully but
    ' with a warnlog (causing some other functions to fail with warnings
    ' as well). Beware of possible problems with screen resolutions. 
    ' This function has been tested for 1024x768 and 1280x1024 pixels.
    ' Method 4 is dangerous because it might accidentially select the 
    ' background window which is the reason why this is not the default.
    
    '///+<li>Try four different ways of selecting the control before giving up</li>
    '///<ol>
    '///+<li>Mouse-Click in the middle</li>
    '///+<li>Mouse-Click on upper left corner</li>
    '///+<li>Mouse-Click on lower right corner</li>
    '///+<li>Rubberband around the control (Mouse movement)</li>
    '///+<li>Deselct everything and use <TAB> to activate the control</li>
    '///</ol>

    autoexecute = false
    for iCurrentSelectionMethod = 1 to SELECTION_METHODS

        Kontext "BasicIDE"
        DialogWindow.typeKeys( "<UP><LEFT>" , REPEAT_COUNT )
        select case ( iCurrentSelectionMethod )
        case SELECT_MIDDLE

            xPos = hGetControlPosXM( iControl )  
            yPos = hGetControlPosYM( iControl )
            DialogWindow.MouseMove( xPos, yPos )
            DialogWindow.MouseDown( xPos, yPos )
            DialogWindow.MouseUp  ( xPos, yPos )

        case SELECT_UPPER_LEFT

            xPos = hGetControlPosXO( iControl ) + EXTRA_FRAME_SIZE
            yPos = hGetControlPosYO( iControl ) + EXTRA_FRAME_SIZE
            DialogWindow.MouseMove( xPos, yPos )
            DialogWindow.MouseDown( xPos, yPos )
            DialogWindow.MouseUp  ( xPos, yPos )

        case SELECT_LOWER_RIGHT

            xPos = hGetControlPosXE( iControl ) - EXTRA_FRAME_SIZE
            yPos = hGetControlPosYE( iControl ) - EXTRA_FRAME_SIZE
            DialogWindow.MouseMove( xPos, yPos )
            DialogWindow.MouseDown( xPos, yPos )
            DialogWindow.MouseUp  ( xPos, yPos )

        case SELECT_FRAME_AROUND

            xPos = hGetControlPosXO( iControl ) - EXTRA_FRAME_SIZE
            yPos = hGetControlPosYO( iControl ) - EXTRA_FRAME_SIZE
            DialogWindow.MouseMove( xPos, yPos )
            DialogWindow.MouseDown( xPos, yPos )
            
            xPos = hGetControlPosXE( iControl ) + EXTRA_FRAME_SIZE
            yPos = hGetControlPosYE( iControl ) + EXTRA_FRAME_SIZE
            DialogWindow.MouseMove( xPos, yPos )
            DialogWindow.MouseUp  ( xPos, yPos )

        end select

        try
            if ( iControl = 11 ) then
                wait( 100 )
                printlog( "tried method: " & iCurrentSelectionMethod )
            endif
            ContextProperties
            hSelectControl() = true
            exit for
        catch
        endcatch
        
    next iCurrentSelectionMethod
    autoexecute = true
    '///</ul>

end function


