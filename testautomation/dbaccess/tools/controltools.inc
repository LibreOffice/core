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
'* owner : marc.neumann@oracle.com
'*
'* short description : Helper Routines for Form and Control Tests
'*
'***************************************************************************************
'*
' #1 fSwitchAutopilotOff
' #1 fSetControlProperty
' #1 fGetControlProperty
' #1 hDrawingWithSelection 
' #1 fOpenControlPropertyBrowser
' #1 addControl
' #1 fSwitchControlDesignMode
' #1 closePropertyBrowserAndDocument
' #1 fOpenMoreControlsToolBar
'*
'\**************************************************************************************


function fSwitchAutopilotOff(SwitchAutoOff as boolean)
    '/// turn the Control autopilots off or on
    '/// <u>parameter:</u>
    '/// <b>SwitchAutoOff:</b> If true the autopilot is turned off, if the parameter is false the autopilot is turned on
                
    '/// open a new document
    printlog "open a new document"
    gApplication = "WRITER"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)
    Kontext "Toolbar"
        call hToolbarSelect("FormControls",true)
  	Kontext "FormControls"   	
    '/// insert a group box to activate autopilot button
   	ListBox.Click
   	sleep(1)      
    call hDrawingWithSelection ( 50, 50, 60, 60 )
	sleep(2)
    '/// check if auto pilot dialog is available
    Kontext "AutoPilotListComboBox"
    if AutoPilotListComboBox.Exists(3) then
        Kontext "AutoPilotListComboBox"
        AutoPilotListComboBox.Cancel
        sleep(2)
        if SwitchAutoOff = true then
            Kontext "FormControls" 
            printlog "switch autopilot off"   	
            UseWizards.Click
            sleep(2)
        else 
            printlog "let autopilot on"
        endif
    else
        if SwitchAutoOff = false then
            Kontext "FormControls" 
            printlog "switch autopilot on"   	
            UseWizards.Click
            sleep(2)
        else 
            printlog "let autopilot off"
        endif
    endif 	
    hCloseDocument
end function
'--------------------------------------------------------------------------------------
function fSetControlProperty(sControlType, sPropertyName, sPropertyValue as string)
  	'/// set the property of a control via the control property browser    
    '/// <u>parameter:</u>
    '/// <b>sControlType:</b> The name of the control type f.e. TextBox
    '/// <b>sPropertyName:</b> The name of the control property f.e. DefaultValue
    '/// <b>sPropertyValue:</b> The value of the control property    
    
    ' switch to the General Page
    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabGeneralControl 

    Kontext "TabGeneralControl"	
    if TabGeneralControl.exists(3) then        
        printlog "Set Property '" + sPropertyName + "' to the value '" + sPropertyValue + "' for a '" + sControlType + "' control."   
        ' A - C
                
                if sPropertyName = "Accuracy" then
                    Accuracy.setText(sPropertyValue)
                elseif sPropertyName = "Align" then        
                    Align.select(Cint(sPropertyValue))
                elseif sPropertyName = "AdditionalInfo" then
                    Information.setText(sPropertyValue)
                    Information.TypeKeys("<RETURN>",true)
                elseif sPropertyName = "AutoComplete" then
                    AutoComplete.select(Cint(sPropertyValue))
                elseif sPropertyName = "Background" then
                    Background.select(Cint(sPropertyValue))
                elseif sPropertyName = "Border" then
                    Border.select(Cint(sPropertyValue))
                elseif sPropertyName = "BorderColor" then
                    BorderColor.select(Cint(sPropertyValue))
                elseif sPropertyName = "ButtonType" then
                    ButtonType.select(Cint(sPropertyValue))
                elseif sPropertyName = "BlockIncrement" then
                    BlockIncrement.setText(sPropertyValue)
                elseif sPropertyName = "CellBound" then
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabDataControl
                    Kontext "TabDataControl"
                        CellBound.setText(sPropertyValue)
                        CellBound.typeKeys("<RETURN>",true)
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabGeneralControl
                elseif sPropertyName = "CellRange" then
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabDataControl
                    Kontext "TabDataControl"
                        CellRange.setText(sPropertyValue)
                        CellRange.typeKeys("<RETURN>",true)
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabGeneralControl
                elseif sPropertyName = "CurrSymPosition" then
                    CurrSymPosition.select(Cint(sPropertyValue))
                elseif sPropertyName = "CurrencySymbol" then
                    CurrencySymbol.setText(sPropertyValue)
            
        'D - F
        
                elseif sPropertyName = "DateMax" then
                    DateMax.setText(sPropertyValue)
                elseif sPropertyName = "DateMin" then
                    DateMin.setText(sPropertyValue)
                elseif sPropertyName = "DateFormat" then
                    DateFormat.select(Cint(sPropertyValue))
                elseif sPropertyName = "DefaultButton" then
                    DefaultButton.select(Cint(sPropertyValue))
                elseif sPropertyName = "DefaultSelection2" then
                    DefaultSelection2.setText(sPropertyValue)
                elseif sPropertyName = "DefaultState" then
                    DefaultState.select(Cint(sPropertyValue))
                elseif sPropertyName = "DefaultValue" then
                    TabGeneralControl.MouseDown 1,1,1,true
                    TabGeneralControl.MouseUp 1,1,1,true
                    sleep(1)
                    TabGeneralControl.TypeKeys "<TAB>" , 16 , true
                    sleep(1)
                    TabGeneralControl.TypeKeys sPropertyValue , true                    
                    sleep(1)
                    TabGeneralControl.TypeKeys "<RETURN>" , true
                    sleep(1)
                elseif sPropertyName = "Delay" then
                    Delay.setText(sPropertyValue)
                elseif sPropertyName = "DropDown" then
                    DropDown.select(Cint(sPropertyValue))
                elseif sPropertyName = "Enabled" then
                    Enabled.select(Cint(sPropertyValue))
                elseif sPropertyName = "EditMask" then
                    EditMask.setText(sPropertyValue)    
                elseif sPropertyName = "FilteringSorting" then
                    FilteringSorting.select(Cint(sPropertyValue))
                elseif sPropertyName = "Frame" then
                    Frame.select(Cint(sPropertyValue))
        
        'G - L
        
                elseif sPropertyName = "Graphics" then
                    Graphics.setText(sPropertyValue)  
                elseif sPropertyName = "GraphicsAlignment" then
                    GraphicsAlignment.select(Cint(sPropertyValue))
                elseif sPropertyName = "HelpText" then
                    HelpText.setText(sPropertyValue)
                elseif sPropertyName = "HelpURL" then
                    HelpURL.setText(sPropertyValue)
                    HelpURL.TypeKeys "<RETURN>",true
                elseif sPropertyName = "HiddenValue" then
                    HiddenValue.setText(sPropertyValue)
                elseif sPropertyName = "HideSelection" then
                    HideSelection.select(Cint(sPropertyValue))
                elseif sPropertyName = "IconSize" then
                    IconSize.select(Cint(sPropertyValue))
                elseif sPropertyName = "LineCount" then
                     LineCount.setText(sPropertyValue) 
                elseif sPropertyName = "LineIncrement" then
                    LineIncrement.setText(sPropertyValue)
                elseif sPropertyName = "LiteralMask" then
                    LiteralMask.setText(sPropertyValue)
                elseif sPropertyName = "ListEntries" then
                    ListEntries.setText(sPropertyValue)
            
        'M - O
         
                elseif sPropertyName = "MaxTextLen" then
                     MaxTextLen.setText(sPropertyValue)
                elseif sPropertyName = "MultiLine" then
                    qaerrorlog "The Property MultiLine doesn't exists anymore."
                    qaerrorlog "Please use TextType instead. This property will be removed soon."
                    TextType.select(Cint(sPropertyValue))
                elseif sPropertyName = "MultiLine2" then
                    MultiLine.select(Cint(sPropertyValue))
                elseif sPropertyName = "Multiselection" then
                    Multiselection.select(Cint(sPropertyValue))
                elseif sPropertyName = "NameText" then
                    NameText.setText(sPropertyValue)
                    NameText.typeKeys("<RETURN>",true)
                elseif sPropertyName = "Navigation" then
                    Navigation.select(Cint(sPropertyValue))
                elseif sPropertyName = "NavigationBar" then
                    NavigationBar.select(Cint(sPropertyValue))            
                elseif sPropertyName = "Orientation" then
                    Orientation.select(Cint(sPropertyValue))
                elseif sPropertyName = "Order" then
                    Order.setText(sPropertyValue)
            
        'P - R
        
                elseif sPropertyName = "Password" then
                    Password.setText(sPropertyValue)
                elseif sPropertyName = "ReferenceValue" then
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabDataControl
                    Kontext "TabDataControl"
                        ReferenceValue.setText(sPropertyValue)
                        ReferenceValue.TypeKeys("<RETURN>",true)
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabGeneralControl
                elseif sPropertyName = "Repeat" then
                    Repeat.select(Cint(sPropertyValue))
                elseif sPropertyName = "Printable" then
                    Printable.select(Cint(sPropertyValue))
                elseif sPropertyName = "Positioning" then
                    Positioning.select(Cint(sPropertyValue))
                elseif sPropertyName = "RecordActions" then
                    RecordActions.select(Cint(sPropertyValue))    
                elseif sPropertyName = "RecordMarker" then
                    RecordMarker.select(Cint(sPropertyValue))
                elseif sPropertyName = "ReferenceValueOff" then
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabDataControl
                    Kontext "TabDataControl"
                        ReferenceValueOff.setText(sPropertyValue)
                        ReferenceValueOff.TypeKeys("<RETURN>",true)
                    Kontext "ControlPropertiesTabControl"
                        ControlPropertiesTabControl.setPage TabGeneralControl
                elseif sPropertyName = "Readonly" then
                    Readonly.select(Cint(sPropertyValue))    
                elseif sPropertyName = "RowHeight" then
                    RowHeight.setText(sPropertyValue)
            
        'S
        
                elseif sPropertyName = "Scale" then
                    Scale.select(Cint(sPropertyValue))
                elseif sPropertyName = "Spin" then            
                    Spin.select(CInt(sPropertyValue))
                elseif sPropertyName = "StrictFormat" then            
                    StrictFormat.select(CInt(sPropertyValue))
                elseif sPropertyName = "Scrollbars" then
                    Scrollbars.select(Cint(sPropertyValue))
                elseif sPropertyName = "SymbolColor" then
                    SymbolColor.select(Cint(sPropertyValue))        
                elseif sPropertyName = "TextType" then
                    TextType.select(Cint(sPropertyValue))
                elseif sPropertyName = "ScrollValueMin" then
                    ScrollValueMin.setText(sPropertyValue) 
                elseif sPropertyName = "ScrollValueMax" then
                    ScrollValueMax.setText(sPropertyValue)
                elseif sPropertyName = "ScrollValueDefault" then
                    ScrollValueDefault.setText(sPropertyValue)        
        
        'T - U
        
                elseif sPropertyName = "TabOrder" then            
                    Order.setText(sPropertyValue)
                    TabGeneralControl.TypeKeys "<RETURN>",true
                elseif sPropertyName = "TabStop" then            
                    TabStop.select(Cint(sPropertyValue))
                elseif sPropertyName = "TakeFocus" then            
                    TakeFocus.select(Cint(sPropertyValue))
                elseif sPropertyName = "TextType" then            
                    TextType.select(Cint(sPropertyValue))
                elseif sPropertyName = "ThousandSeperator" then            
                    ThousandSeperator.select(CInt(sPropertyValue))
                elseif sPropertyName = "TimeFormat" then
                    TimeFormat.select(Cint(sPropertyValue))
                elseif sPropertyName = "TimeMax" then
                    TimeMax.setText(sPropertyValue)
                elseif sPropertyName = "TimeMin" then
                    TimeMin.setText(sPropertyValue)
                elseif sPropertyName = "ToggleProp" then
                    ToggleProp.select(Cint(sPropertyValue))
                elseif sPropertyName = "TriState" then
                    TriState.select(Cint(sPropertyValue))
                elseif sPropertyName = "URL" then        
                    URL.setText(sPropertyValue)
            
        'V - Z
        
                elseif sPropertyName = "VisibleSize" then
                    VisibleSize.setText(sPropertyValue)
                elseif sPropertyName = "ValueMin" then
                    if sControlType = "FormattedField" then
                        EffectiveMin.setText(sPropertyValue)
                    else
                        ValueMin.setText(sPropertyValue)
                    endif
                elseif sPropertyName = "ValueMax" then
                    if sControltype = "FormattedField" then
                        EffectiveMax.setText(sPropertyValue)
                    else
                        ValueMax.setText(sPropertyValue)
                    endif
                elseif sPropertyName = "ValueStep" then
                    ValueStep.setText(sPropertyValue)
        '        
                else
                    warnlog "The property " + sPropertyName + " doesn't exists in the function fSetControlProperty."
                    warnlog "Please add it to the function in dbaccess/tools/controltools.inc."          
                endif
        
        'need additional return
        sleep(1)
        TabGeneralControl.TypeKeys "<RETURN>",true
    else
        warnlog "The control property browser doesn't exists."
    endif
    sleep(1)
end function
'--------------------------------------------------------------------------------------
function fGetControlProperty(sControlType,sPropertyName)
	'/// get the property of a control from the control property browser    
    '/// <u>parameter:</u>
    '/// <b>sControlType:</b> The name of the control type f.e. TextBox
    '/// <b>sPropertyName:</b> The name of the control property f.e. DefaultValue
            
    Dim sReturn as String

    ' switch to the General Page
    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabGeneralControl

    Kontext "TabGeneralControl"	
    if TabGeneralControl.exists(3) then
        
    ' A - C
        
        if sPropertyName = "Accuracy" then
            sReturn = Accuracy.getSelIndex
        elseif sPropertyName = "AdditionalInfo" then
            sReturn = Information.getText
        elseif sPropertyName = "Align" then
            sReturn = Align.getSelIndex 
        elseif sPropertyName = "AutoComplete" then
            sReturn = AutoComplete.getSelIndex   
        elseif sPropertyName = "Background" then
            sReturn = Background.getSelIndex
        elseif sPropertyName = "BlockIncrement" then
            sReturn = BlockIncrement.getText
        elseif sPropertyName = "Border" then
            sReturn = Border.getSelIndex
        elseif sPropertyName = "BorderColor" then
            sReturn = BorderColor.getSelIndex
        elseif sPropertyName = "ButtonType" then
            sReturn = ButtonType.getSelIndex
        elseif sPropertyName = "CellBound" then
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabDataControl
            Kontext "TabDataControl"                
                sReturn = CellBound.getText()                
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabGeneralControl
        elseif sPropertyName = "CellRange" then
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabDataControl
            Kontext "TabDataControl"                
                sReturn = CellRange.getText()                
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabGeneralControl
        elseif sPropertyName = "CurrSymPosition" then
            sReturn = CurrSymPosition.getSelIndex
        elseif sPropertyName = "CurrencySymbol" then
            sReturn = CurrencySymbol.getText
            
    'D - F
            
        elseif sPropertyName = "DateMax" then
            sReturn = DateMax.getText
        elseif sPropertyName = "DateMin" then
            sReturn = DateMin.getText    
        elseif sPropertyName = "DateFormat" then
            sReturn = DateFormat.getSelIndex
        elseif sPropertyName = "DefaultButton" then
            sReturn = DefaultButton.getSelIndex
        elseif sPropertyName = "Delay" then
            sReturn = Delay.getText
        elseif sPropertyName = "DefaultState" then
            sReturn = DefaultState.getSelIndex
        elseif sPropertyName = "DefaultSelection2" then
            sReturn = DefaultSelection2.getText
        elseif sPropertyName = "DefaultValue" then
            sReturn = DefaultValue.getText
        elseif sPropertyName = "DropDown" then
            sReturn = DropDown.getSelIndex
        elseif sPropertyName = "EditMask" then
            sReturn = EditMask.getText
        elseif sPropertyName = "Enabled" then
            sReturn = Enabled.getSelIndex
        elseif sPropertyName = "FilteringSorting" then
            sReturn = FilteringSorting.getSelIndex
        elseif sPropertyName = "Formatkey" then
            sReturn = Formatkey.getText
        elseif sPropertyName = "Frame" then
            sReturn = Frame.getSelIndex
            
    'G - L
            
        elseif sPropertyName = "Graphics" then
            sReturn = Graphics.getSelText
        elseif sPropertyName = "GraphicsAlignment" then  
            sReturn = GraphicsAlignment.getSelIndex
        elseif sPropertyName = "HelpText" then
            sReturn = HelpText.getText
        elseif sPropertyName = "HelpURL" then
            sReturn = HelpURL.getText
        elseif sPropertyName = "HiddenValue" then
            sReturn = HiddenValue.getText 
        elseif sPropertyName = "HideSelection" then
            sReturn = HideSelection.getSelIndex            
        elseif sPropertyName = "IconSize" then
            sReturn = IconSize.getSelIndex
        elseif sPropertyName = "LineCount" then
            sReturn = LineCount.getText
        elseif sPropertyName = "LineIncrement" then
            sReturn = LineIncrement.getText
        elseif sPropertyName = "LiteralMask" then
            sReturn = LiteralMask.getText
        elseif sPropertyName = "ListEntries" then  
            sReturn = ListEntries.getText
        
    'M - O
        
        elseif sPropertyName = "MaxTextLen" then
            sReturn = MaxTextLen.getText
        elseif sPropertyName = "MultiLine" then
            sReturn = MultiLine.getSelIndex
        elseif sPropertyName = "MultiLine2" then
            sReturn = MultiLine.getSelIndex
        elseif sPropertyName = "Multiselection" then
            sReturn = Multiselection.getSelIndex
        elseif sPropertyName = "NameText" then
            sReturn = NameText.getText
        elseif sPropertyName = "Navigation" then
            sReturn = Navigation.getSelIndex
        elseif sPropertyName = "NavigationBar" then
            sReturn = NavigationBar.getSelIndex
        elseif sPropertyName = "Order" then
            sReturn = Order.getText
        elseif sPropertyName = "Orientation" then
            sReturn = Orientation.getSelIndex        
         
    'P - R
        
        elseif sPropertyName = "Password" then
            sReturn = Password.getText
        elseif sPropertyName = "Positioning" then
            sReturn = Positioning.getSelIndex
        elseif sPropertyName = "Printable" then
            sReturn = Printable.getSelIndex
        elseif sPropertyName = "Readonly" then
            sReturn = Readonly.getSelIndex
        elseif sPropertyName = "RecordActions" then
            sReturn = RecordActions.getSelIndex
        elseif sPropertyName = "RecordMarker" then
            sReturn = RecordMarker.getSelIndex
        elseif sPropertyName = "ReferenceValue" then
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabDataControl
            Kontext "TabDataControl"
                sReturn = ReferenceValue.getText
            Kontext "ControlPropertiesTabControl"
                ControlPropertiesTabControl.setPage TabGeneralControl
        elseif sPropertyName = "Repeat" then
            sReturn = Repeat.getSelIndex  
        elseif sPropertyName = "RowHeight" then
            sReturn = RowHeight.getText
    
    'S
        
        elseif sPropertyName = "Scale" then
            sReturn = Scale.getSelIndex
        elseif sPropertyName = "Scrollbars" then
            sReturn = Scrollbars.getSelIndex
        elseif sPropertyName = "ScrollValueDefault" then
            sReturn = ScrollValueDefault.getText
        elseif sPropertyName = "ScrollValueMax" then
            sReturn = ScrollValueMax.getText
        elseif sPropertyName = "ScrollValueMin" then
            sReturn = ScrollValueMin.getText
        elseif sPropertyName = "Spin" then
            sReturn = Spin.getSelIndex
        elseif sPropertyName = "StrictFormat" then
            sReturn = StrictFormat.getSelIndex
        elseif sPropertyName = "SymbolColor" then
            sReturn = SymbolColor.getSelIndex
        
    'T - U
        
        elseif sPropertyName = "TabStop" then
            sReturn = TabStop.getSelIndex
        elseif sPropertyName = "TakeFocus" then
            sReturn = TakeFocus.getSelIndex
        elseif sPropertyName = "TextType" then            
            sReturn = TextType.getSelIndex
        elseif sPropertyName = "ThousandSeperator" then
            sReturn = ThousandSeperator.getSelIndex
        elseif sPropertyName = "TimeFormat" then
            sReturn = TimeFormat.getSelIndex
        elseif sPropertyName = "TimeMax" then
            sReturn = TimeMax.getText
        elseif sPropertyName = "TimeMin" then
            sReturn = TimeMin.getText
        elseif sPropertyName = "ToggleProp" then  
            sReturn = ToggleProp.getSelIndex
        elseif sPropertyName = "TriState" then
            sReturn = TriState.getSelIndex
    
    'V - Z
    
        elseif sPropertyName = "ValueMin" then
            if sControlType = "FormattedField" then
                sReturn = EffectiveMin.getText
            else
                sReturn = ValueMin.getText
            endif
        elseif sPropertyName = "ValueMax" then
            if sControltype = "FormattedField" then
                sReturn = EffectiveMax.getText
            else
                sReturn = ValueMax.getText
            endif
        elseif sPropertyName = "VisibleSize" then
            sReturn = VisibleSize.getText
        elseif sPropertyName = "ValueStep" then
            sReturn = ValueStep.getText
            
        endif
    else
        warnlog "The control property browser doesn't exists."
    endif
    fGetControlProperty = sReturn
end function
'--------------------------------------------------------------------------------------
sub hDrawingWithSelection ( iStartX, iStartY, iEndX, iEndY )
    '/// move the mouse with pressed mouse button      
    '/// <u>parameter:</u>
    '/// <b>iStartX:</b> The x start coordinate
    '/// <b>iStartY:</b> The y start coordinate
    '/// <b>iEndX:</b> The x end coordinate
    '/// <b>iEndY:</b> The x end coordinate

   select case gApplication

      case "CALC"        : Kontext "DocumentCalc"
                            DocumentCalc.MouseDown ( iStartX, iStartY )
                            DocumentCalc.MouseMove ( iEndX, iEndY )
                            DocumentCalc.MouseUp ( iEndX, iEndY )

      case "WRITER"      : Kontext "DocumentWriter"
                            DocumentWriter.MouseDown ( iStartX, iStartY )
                            DocumentWriter.MouseMove ( iEndX, iEndY )
                            DocumentWriter.MouseUp ( iEndX, iEndY )

      case "HTML": Kontext "DocumentWriterWeb"
                            DocumentWriterWeb.MouseDown ( iStartX, iStartY )
                            DocumentWriterWeb.MouseMove ( iEndX, iEndY )
                            DocumentWriterWeb.MouseUp ( iEndX, iEndY )

      case "MASTERDOCUMENT"   : Kontext "DocumentMasterDoc"
                            DocumentMasterDoc.MouseDown ( iStartX, iStartY )
                            DocumentMasterDoc.MouseMove ( iEndX, iEndY )
                            DocumentMasterDoc.MouseUp ( iEndX, iEndY )

      case "IMPRESS"	 : Kontext "DocumentImpress"
                            DocumentImpress.MouseDown ( iStartX, iStartY )
                            DocumentImpress.MouseMove ( iEndX, iEndY )
                            DocumentImpress.MouseUp ( iEndX, iEndY )

      case "DRAW"        : Kontext "DocumentDraw"
                            DocumentDraw.MouseDown ( iStartX, iStartY )
                            DocumentDraw.MouseMove ( iEndX, iEndY )
                            DocumentDraw.MouseUp ( iEndX, iEndY )

   end select
end sub
'--------------------------------------------------------------------------------------
function fOpenControlPropertyBrowser
    '/// open the Control Properties Dialog      
    '/// <u>parameter:</u>
    '/// <u>return:</u>    
    sleep(2)

    call hToolbarSelect("FormControls",true)  	

    Kontext "FormControls"
        ControlProperties.Click  
    Kontext "ControlPropertiesDialog"
    
    if ControlPropertiesDialog.exists(3) then
        'nothing
    else
       Kontext "FormControls"        
            ControlProperties.Click    
    endif
        
    sleep(2)
    fOpenControlPropertyBrowser=true
    
end function
'--------------------------------------------------------------------------------------
function addControl(sControl as String, isx as integer , isy as integer , iex as integer , iey as integer)
    '/// add the specified control and the specified position      
    '/// <u>parameter:</u>
    '/// <b>sControl</b> the control name
    '/// <b>isx</b> the start x coordinate
    '/// <b>isy</b> the start y coordinate
    '/// <b>iex</b> the end x coordinate
    '/// <b>iey</b> the end y coordinate
    '/// <u>return:</u>

    dim oControl as Object
        
    if sControl = "SpinButton" then
    
        call hToolbarSelect("MoreControls",true)            
        Kontext "MoreControls"  
            SpinButton.Click        

    elseif sControl = "ScrollBar" then
    
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            Scrollbar.Click
            
    elseif sControl = "ImageButton" then
    
        call hToolbarSelect("MoreControls",true)
        Kontext "MoreControls"  
            ImageButton.Click
            
    elseif sControl = "ImageControl" then
           
        call hToolbarSelect("MoreControls",true)
        Kontext "MoreControls"  
            ImageControl.Click
            
    elseif sControl = "FileControl" then
        
        call hToolbarSelect("MoreControls",true)
        Kontext "MoreControls"  
            FileControl.Click
            
    elseif sControl = "DateField" then
           
        call hToolbarSelect("MoreControls",true)       
        Kontext "MoreControls"  
            DateField.Click
            
    elseif sControl = "TimeField" then
       
        call hToolbarSelect("MoreControls",true)       
        Kontext "MoreControls"  
            TimeField.Click
                   
    elseif sControl = "NumericField" then
           
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            Numericfield.Click
            
    elseif sControl = "CurrencyField" then
           
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            CurrencyField.Click
            
    elseif sControl = "PatternField" then
           
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            PatternField.Click
            
    elseif sControl = "GroupBox" then
          
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            GroupBox.Click
            
    elseif sControl = "Grid" then
           
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            Grid.Click
            
    elseif sControl = "NavigationBar" then
           
        call hToolbarSelect("MoreControls",true)        
        Kontext "MoreControls"  
            NavigationBar.Click
            
    elseif sControl = "Edit" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            Edit.Click            

    elseif sControl = "Label" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            Label.Click
   
    elseif sControl = "CheckBox" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            CheckBox.Click
            
    elseif sControl = "FormattedField" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            FormattedField.Click
   
    elseif sControl = "PushButton" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            PushButton.Click 
    
    elseif sControl = "ListBox" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            ListBox.Click
   
    elseif sControl = "ComboBox" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            ComboBox.Click
            
    elseif sControl = "RadioButton" then
    
        call hToolbarSelect("FormControls",true)        
        Kontext "FormControls"  
            RadioButton.Click
            
    else
        warnlog "control " + sControl + " not found. add to addControl in control tools."    
    endif
    sleep(1)
    
    call hDrawingWithSelection ( isx, isy, iex, iey )
    
    sleep(2)
   	
    Kontext "FormControls"
    
end Function
'--------------------------------------------------------------------------------------
function fSwitchControlDesignMode()
    '/// switch the control design on or off
    '/// <u>parameter:</u>
    '/// <u>return:</u>
    
    Kontext "Toolbar"
        call hToolbarSelect("FormControls",true)
        sleep(1)
  	Kontext "FormControls"
        SwitchControlDesignMode.Click
        
end function
'--------------------------------------------------------------------------------------
function closePropertyBrowserAndDocument
    '/// close control property browser and the document
    '/// only used in the Control_Serveral tests
    '/// <u>parameter:</u>
    '/// <u>return:</u>
    
    printlog "close control property browser"    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    '/// close document
    printlog "close document"
    hCloseDocument
    
end function
'--------------------------------------------------------------------------------------
function fOpenMoreControlsToolBar
    '/// open the More Controls toolbar      
    '/// <u>parameter:</u>
    '/// <u>return:</u>
    Kontext "MoreControls"
    if not MoreControls.exists(1) then
        Kontext "FormControls"
            MoreControls.Click
        Kontext "MoreControls"    
    endif   
End function
