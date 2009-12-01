'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_tools3.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:11 $
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
'* short description : Global Tools III
'*
'\*************************************************************************************


sub hToolbarSelect( sType as string, sOpen as boolean, optional SetToDefault as boolean )
    Dim sDefault as integer, sKontext as string, sPosition as integer
    Dim SteppedThrough as boolean
    '/// Created by helge.delfs@sun.com
    '/// This function opens/closes a toolbar through View/Toolbar menu
    '/// Required parameters:
    '/// sType as string 		-> Name of the toolbar to be opened / closed
    '/// sOpen as boolean	-> Shall the toolbar be opened (true) or closed (false)
    '/// optional SetToDefault as boolean -> true if Toolbar-State should be set to default

        Select Case Ucase(sType)
        case "3DSETTING"
            Kontext "ExtrusionObjectBar"
            if ExtrusionObjectBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsThreeDSettings
                        else
                            if SetToDefault = True then ViewToolbarsThreeDSettings
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsThreeDSettings
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsThreeDSettings
                    endif
                endif
            endif
                

        case "ALIGN"					
            Kontext "Alignmentbar"
            if Alignmentbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsAlign
                        else
                            if SetToDefault = True then ViewToolbarsAlign
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsAlign
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsAlign
                    endif
                endif
            endif
                                   
        Case "BULLETSANDNUMBERING"
            Kontext "NumObjectbar"
            if NumObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsBulletsAndNumbering
                        else
                            if SetToDefault = True then ViewToolbarsBulletsAndNumbering
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsBulletsAndNumbering
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsBulletsAndNumbering
                    endif
                endif
            endif			
            
        case "COLOR"            
            Kontext "ColorBar"
            if ColorBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsColor
                        else
                            if SetToDefault = True then ViewToolbarsColor
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsColor
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsColor
                    endif
                endif
            endif
         
        case "CUSTOMIZE"
            ViewToolbarsCustomize
                    
        case "DRAWING"	
            Kontext "DrawBar"
            if DrawBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsDrawing
                        else
                            if SetToDefault = True then ViewToolbarsDrawing
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsDrawing
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsDrawing
                    endif
                endif
            endif
                                        
        case "FORMATTING"
            Kontext "TextObjectbar"
            if TextObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormatting
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormatting
                    else
                        if SetToDefault = True then ViewToolbarsFormatting
                    endif
                endif
            endif

        case "FORMCONTROLS"
            Kontext "FormControls"
            if FormControls.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormControls
                        else
                            if SetToDefault = True then ViewToolbarsFormControls
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormControls
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormControls
                    endif
                endif
            endif
        case "CONTROLS"
            Kontext "FormControls"
            if FormControls.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormControls
                        else
                            if SetToDefault = True then ViewToolbarsFormControls
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormControls
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormControls
                    endif
                endif
            endif                
            
        case "FORMDESIGNTOOLS", "FORMDESIGN"		     	
            Kontext "FormDesignTools"
            if FormDesignTools.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormDesign
                        else
                            if SetToDefault = True then ViewToolbarsFormDesign
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormDesign
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormDesign
                    endif
                endif
            endif
                
        case "HTMLSOURCEVIEW"		     	
            Kontext "SourceViewToolbar"
            if SourceViewToolbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsHTMLSourceView
                        else
                            if SetToDefault = True then ViewToolbarsHTMLSourceView
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsHTMLSourceView
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsHTMLSourceView
                    endif
                endif
            endif

        case "FORMFILTER"
            Kontext "FormsFilterBar"
            if FormsFilterBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormFilter
                        else
                            if SetToDefault = True then ViewToolbarsFormFilter
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormFilter
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormFilter
                    endif
                endif
            endif

        case "FORMNAVIGATION"
            Kontext "FormsNavigationBar"
            if FormsNavigationBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormNavigation
                        else
                            if SetToDefault = True then ViewToolbarsFormNavigation
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormNavigation
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormNavigation
                    endif
                endif
            endif
             
        case "FORMOBJECT"
            Kontext "FormsObjectbar"
            if FormsObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFormObject
                        else
                            if SetToDefault = True then ViewToolbarsFormObject
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFormObject
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFormObject
                    endif
                endif
            endif

        case "FRAME"	
            Kontext "FrameObjectbar"
            if FrameObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFrame
                        else
                            if SetToDefault = True then ViewToolbarsFrame
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFrame
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFrame
                    endif
                endif
            endif

        case "FULLSCREEN"
            Kontext "FullScreenBar"
            if FullScreenBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsFullScreen
                        else
                            if SetToDefault = True then ViewToolbarsFullScreen
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsFullScreen
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsFullScreen
                    endif
                endif
            endif
            
        case "GRAPHIC", "DRAWINGOBJECTPROPERTIES"
            Kontext "DrawingObjectbar"
            if DrawingObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsGraphic
                        else
                            if SetToDefault = True then ViewToolbarsGraphic
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsGraphic
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsGraphic
                    endif
                endif
            endif

        case "HYPERLINK", "HYPERLINKBAR"
            Kontext "Hyperlinkleiste"
            if Hyperlinkleiste.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsInsertHyperlink
                        else
                            if SetToDefault = True then ViewToolbarsInsertHyperlink
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsInsertHyperlink
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsInsertHyperlink
                    endif
                endif
            endif
                        
            
        case "INSERT"		
            Kontext "InsertBar"
            if InsertBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsInsert
                        else
                            if SetToDefault = True then ViewToolbarsInsert
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsInsert
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsInsert
                    endif
                endif
            endif
                                    
        case "INSERTOBJECT"
            Kontext "InsertObjectBar"
            if InsertObjectBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsInsertObject
                        else
                            if SetToDefault = True then ViewToolbarsInsertObject
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsInsertObject
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsInsertObject
                    endif
                endif
            endif
           
        case "MEDIAPLAYBACK"		
            Kontext "MediaObjectBar"
            if MediaObjectBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsMediaPlayback
                        else
                            if SetToDefault = True then ViewToolbarsMediaPlayback
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsMediaPlayback
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsMediaPlayback
                    endif
                endif
            endif
                                    
        case "MORECONTROLS"
            Kontext "MoreControls"
            if MoreControls.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsMoreControls
                        else
                            if SetToDefault = True then ViewToolbarsMoreControls
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsMoreControls
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsMoreControls
                    endif
                endif
            endif
                            
        case "MOREXFORMCONTROLS"
            Kontext "MoreXFormControls"
            if MoreXFormControls.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsMoreXFormControls
                        else
                            if SetToDefault = True then ViewToolbarsMoreXFormControls
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsMoreXFormControls
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsMoreXFormControls
                    endif
                endif
            endif
                
        case "OLEOBJECT"		
            Kontext "OLEObjectbar"
            if OLEObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsOleObject
                        else
                            if SetToDefault = True then ViewToolbarsOleObject
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsOleObject
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsOleObject
                    endif
                endif
            endif
                        
        case "OPTIMIZETABLE"
            Kontext "OptimizeTablebar"
            if OptimizeTablebar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsOptimizeTable
                        else
                            if SetToDefault = True then ViewToolbarsOptimizeTable
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsOptimizeTable
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsOptimizeTable
                    endif
                endif
            endif
                        
        case "PAGEPREVIEW"
            Kontext "PreviewObjectbar"
            if PreviewObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsPagePreview
                        else
                            if SetToDefault = True then ViewToolbarsPagePreview
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsPagePreview
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsPagePreview
                    endif
                endif
            endif
                                    
        case "PATH"
            Kontext "BezierObjectBar"
            if BezierObjectBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsBezier
                        else
                            if SetToDefault = True then ViewToolbarsBezier
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsBezier
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsBezier
                    endif
                endif
            endif
                                               
        case "PICTURE"		
            Kontext "GraphicObjectbar"
            if GraphicObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsPicture
                        else
                            if SetToDefault = True then ViewToolbarsPicture
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsPicture
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsPicture
                    endif
                endif
            endif
                                    
        case "PICTUREFILTER"
            Kontext "GraphicFilterBar"
            if GraphicFilterBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsPictureFilter
                        else
                            if SetToDefault = True then ViewToolbarsPictureFilter
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsPictureFilter
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsPictureFilter
                    endif
                endif
            endif
                        
        case "STANDARD"            
            Kontext "StandardBar"
            if StandardBar.Exists then
                if StandardBar.IsVisible then
                    Select Case sOpen
                        Case False
                            if IsMissing(SetToDefault) then
                                ViewToolbarsStandard
                            endif
                        end select
                else
                    if sOpen = True then
                        if IsMissing(SetToDefault) then
                            ViewToolbarsStandard
                        else
                            if SetToDefault = True then ViewToolbarsStandard
                        endif
                    endif
                endif	            
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsStandard
                    else
                        if SetToDefault = True then ViewToolbarsStandard
                    endif
                endif
            endif

        case "STANDARDVIEWINGMODE"		
            Kontext "Viewerbar"
            if Viewerbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsStandardView
                        else
                            if SetToDefault = True then ViewToolbarsStandardView
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsStandardView
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsStandardView
                    endif
                endif
            endif
                        
        case "TABLE"
            Kontext "TableObjectbar"
            if TableObjectbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsTable
                        else
                            if SetToDefault = True then ViewToolbarsTable
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsTable
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsTable
                    endif
                endif
            endif
                                                                                                                        
        case "TEXTOBJECT"
            Kontext "DrawTextObjectBar"
            if DrawTextObjectBar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsTextObject
                        else
                            if SetToDefault = True then ViewToolbarsTextObject
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsTextObject
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsTextObject
                    endif
                endif
            endif
                                                                                                
        case "TOOLS"            
            Kontext "Toolbar"
            if Toolbar.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsTools
                        else
                            if SetToDefault = True then ViewToolbarsTools
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsTools
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsTools
                    endif
                endif
            endif
            
                
        case "MATH"
            Kontext "OL_SW_Rechenleiste"
            if OL_SW_Rechenleiste.Exists then
                Select Case sOpen
                    Case False
                        if IsMissing(SetToDefault) then
                            ViewToolbarsInsertFormula
                        else
                            if SetToDefault = True then ViewToolbarsInsertFormula
                        endif
                    Case True
                        if IsMissing(SetToDefault) = False then
                            if SetToDefault = True then ViewToolbarsInsertFormula
                        endif
                end select
            else
                if sOpen = True then
                    if IsMissing(SetToDefault) then
                        ViewToolbarsInsertFormula
                    endif
                endif
            endif
                                        
        case else
            Warnlog "No menuentry for parameter " & sType & " found!"
             
        end select    
        Sleep 1
        
end sub

sub hCloseAllToolbars
    '/// Created by thorsten.bosbach@sun.com
    '/// This function closes all known toolbars which are not docked
    '/// only if Build ID is below 8892
    if (gBuild < 8892) AND (gBuild > 8888) then
        try
                Kontext "ExtrusionObjectBar"
                if ExtrusionObjectBar.Exists(0) then if NOT ExtrusionObjectBar.isDocked then ExtrusionObjectBar.close
                Kontext "Alignmentbar"
                if Alignmentbar.Exists(0) then if NOT Alignmentbar.isDocked then Alignmentbar.close
                Kontext "NumObjectbar"
                if NumObjectbar.Exists(0) then if NOT NumObjectbar.isDocked then NumObjectbar.close
                Kontext "DrawBar"
                if DrawBar.Exists(0) then if NOT DrawBar.isDocked then DrawBar.close
                Kontext "TextObjectbar"
                if TextObjectbar.Exists(0) then if NOT TextObjectbar.isDocked then TextObjectbar.close
                Kontext "FormControls"
                if FormControls.Exists(0) then if NOT FormControls.isDocked then FormControls.close
                Kontext "FormControls"
                if FormControls.Exists(0) then if NOT FormControls.isDocked then FormControls.close
                Kontext "FormDesignTools"
                if FormDesignTools.Exists(0) then if NOT FormDesignTools.isDocked then FormDesignTools.close
                Kontext "SourceViewToolbar"
                if SourceViewToolbar.Exists(0) then if NOT SourceViewToolbar.isDocked then SourceViewToolbar.close
                Kontext "FormsFilterBar"
                if FormsFilterBar.Exists(0) then if NOT FormsFilterBar.isDocked then FormsFilterBar.close
                Kontext "FormsNavigationBar"
                if FormsNavigationBar.Exists(0) then if NOT FormsNavigationBar.isDocked then FormsNavigationBar.close
                Kontext "FormsObjectbar"
                if FormsObjectbar.Exists(0) then if NOT FormsObjectbar.isDocked then FormsObjectbar.close
                Kontext "FrameObjectbar"
                if FrameObjectbar.Exists(0) then if NOT FrameObjectbar.isDocked then FrameObjectbar.close
                Kontext "FullScreenBar"
                if FullScreenBar.Exists(0) then if NOT FullScreenBar.isDocked then FullScreenBar.close
                Kontext "DrawingObjectbar"
                if DrawingObjectbar.Exists(0) then if NOT DrawingObjectbar.isDocked then DrawingObjectbar.close
                Kontext "Hyperlinkleiste"
                if Hyperlinkleiste.Exists(0) then if NOT Hyperlinkleiste.isDocked then Hyperlinkleiste.close
                Kontext "InsertBar"
                if InsertBar.Exists(0) then if NOT InsertBar.isDocked then InsertBar.close
                Kontext "InsertObjectBar"
                if InsertObjectBar.Exists(0) then if NOT InsertObjectBar.isDocked then InsertObjectBar.close
                Kontext "MediaObjectBar"
                if MediaObjectBar.Exists(0) then if NOT MediaObjectBar.isDocked then MediaObjectBar.close
                Kontext "MoreControls"
                if MoreControls.Exists(0) then if NOT MoreControls.isDocked then MoreControls.close
                Kontext "OLEObjectbar"
                if OLEObjectbar.Exists(0) then if NOT OLEObjectbar.isDocked then OLEObjectbar.close
                Kontext "OptimizeTablebar"
                if OptimizeTablebar.Exists(0) then if NOT OptimizeTablebar.isDocked then OptimizeTablebar.close
                Kontext "PreviewObjectbar"
                if PreviewObjectbar.Exists(0) then if NOT PreviewObjectbar.isDocked then PreviewObjectbar.close
                Kontext "BezierObjectBar"
                if BezierObjectBar.Exists(0) then if NOT BezierObjectBar.isDocked then BezierObjectBar.close
                Kontext "GraphicObjectbar"
                if GraphicObjectbar.Exists(0) then if NOT GraphicObjectbar.isDocked then GraphicObjectbar.close
                Kontext "GraphicFilterBar"
                if GraphicFilterBar.Exists(0) then if NOT GraphicFilterBar.isDocked then GraphicFilterBar.close
                Kontext "StandardBar"
                if StandardBar.Exists(0) then if NOT StandardBar.isDocked then StandardBar.close
                Kontext "Viewerbar"
                if Viewerbar.Exists(0) then if NOT Viewerbar.isDocked then Viewerbar.close
                Kontext "TableObjectbar"
                if TableObjectbar.Exists(0) then if NOT TableObjectbar.isDocked then TableObjectbar.close
                Kontext "DrawTextObjectBar"
                if DrawTextObjectBar.Exists(0) then if NOT DrawTextObjectBar.isDocked then DrawTextObjectBar.close
                Kontext "Toolbar"
                if Toolbar.Exists(0) then if NOT Toolbar.isDocked then Toolbar.close
                Kontext "OL_SW_Rechenleiste"
                if OL_SW_Rechenleiste.Exists(0) then if NOT OL_SW_Rechenleiste.isDocked then OL_SW_Rechenleiste.close
        catch
            printlog "tools3.inc::hCloseAllToolbars Can't close some toolbar"
        endcatch
    endif
end sub


function hIsAccessbridgeInstalled() as boolean
    '/// created by HDE
    '/// detects if a Java Access Bridge is installed which decreases testtool performance
    '/// and should not be installed while running automated tests
    '/// 1. searches in testtool.ini entry "UseAccessBridge" in profile "current". Is it <> True then
    '/// 2. searches in directory "C:\Program Files\Java Access Bridge" a Java-Bridge Installation. If this is not found
    '/// 3. searches in options of installed StarOffice on Java-Tabpage for the entry "with accessibility support"
    '/// if 1. = true then false is returned, because it should be tested with accessibility 
    '/// if 2. or 3. matches this func returns true else false 
    '/// if Java is deactivated in Office options it returns false, because no Java->no AccessBridge 
    '/// if AccessbridgeIsInstalled = false (after checking on Java-Tabpage) and iSprache <> English a warning will be printed, because the string in options maybe has to be adapted!
    
    Dim i as integer, j as integer, sAccessibilityCompare as string
    Dim sProgramFiles as string, sAccessBridgeFolder as string, sTmpEntry as string    
    Dim sCompareString as string, AccessbridgeIsInstalled as boolean
    AccessbridgeIsInstalled = False
    if GetIniValue ( gTesttoolIni, "UseAccessBridge", "Current" ) = True then
        AccessbridgeIsInstalled = False
    else
        Select case gPlatgroup
            Case "w95"  
                sProgramFiles = environ("ProgramFiles")
                sAccessBridgeFolder = "Java Access Bridge"
                sCompareString = sProgramFiles & "\" & sAccessBridgeFolder & "\AccessBridgeTester.class"
              	sAccessibilityCompare = "with accessibility support"
                if hFileExists(sCompareString) = True then
                    AccessbridgeIsInstalled = True
                else
                    ToolsOptions
                    Call hToolsOptions("STAROFFICE","JAVA")
                    Kontext "TabJava"
                    if UseJava.IsChecked = True then
                        For i = 1 to JavaList.GetItemCount()                                                                               
                            JavaList.Select i
                            if JavaList.GetText(4) = sAccessibilityCompare then
                                AccessbridgeIsInstalled = True
                                exit for
                            endif
                        next i
						if AccessbridgeIsInstalled = false then
		                    Select Case iSprache
		                        Case 01 : sAccessibilityCompare = "with accessibility support"
    		                    Case else : Warnlog "Accessibility check failed. Maybe language has to be adapted."
            	                        exit function
        	    	        end select
        	    	    endif
                    endif
                    Kontext "OptionenDlg"
                    OptionenDlg.Cancel
                endif                
        end select
    endif
    
    hIsAccessbridgeInstalled = AccessbridgeIsInstalled
    
end function
