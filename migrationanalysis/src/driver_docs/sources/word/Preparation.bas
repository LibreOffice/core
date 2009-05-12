Attribute VB_Name = "Preparation"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: Preparation.bas,v $
' *
' * This file is part of OpenOffice.org.
' *
' * OpenOffice.org is free software: you can redistribute it and/or modify
' * it under the terms of the GNU Lesser General Public License version 3
' * only, as published by the Free Software Foundation.
' *
' * OpenOffice.org is distributed in the hope that it will be useful,
' * but WITHOUT ANY WARRANTY; without even the implied warranty of
' * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' * GNU Lesser General Public License version 3 for more details
' * (a copy is included in the LICENSE file that accompanied this code).
' *
' * You should have received a copy of the GNU Lesser General Public License
' * version 3 along with OpenOffice.org.  If not, see
' * <http://www.openoffice.org/license.html>
' * for a copy of the LGPLv3 License.
' *
' ************************************************************************/
Option Explicit

Function Prepare_HeaderFooter_GraphicFrames(docAnalysis As DocumentAnalysis, myIssue As IssueInfo, _
                                            var As Variant, currDoc As Document) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Prepare_HeaderFooter_GraphicFrames"
    
    Dim myPrepInfo As PrepareInfo
    Set myPrepInfo = var
    
    Dim smove As Long
    Dim temp As Single
    Dim ELength As Single
    Dim PageHeight As Single
    Dim Snum As Integer
    Dim Fnum As Integer
    Dim I As Integer
    Dim myshape As Shape
    Dim shapetop() As Single
    Dim temptop As Single
    
    With currDoc.ActiveWindow 'change to printview
    If .View.SplitSpecial = wdPaneNone Then
        .ActivePane.View.Type = wdPrintView
    Else
        .Panes(2).Close
        .ActivePane.View.Type = wdPrintView
        .View.Type = wdPrintView
    End If
    End With
    
    PageHeight = currDoc.PageSetup.PageHeight
    PageHeight = PageHeight / 2
    
    Selection.GoTo what:=wdGoToPage, Which:=wdGoToAbsolute, _
           count:=myPrepInfo.HF_OnPage
    currDoc.ActiveWindow.ActivePane.View.SeekView = wdSeekCurrentPageHeader
    
    Snum = myPrepInfo.HF_Shapes.count
    If Snum <> 0 Then
       ReDim shapetop(Snum)
       ReDim top(Snum)
        I = 0
        For Each myshape In myPrepInfo.HF_Shapes
            If myshape.Type = msoPicture Then
                If myshape.RelativeVerticalPosition <> wdRelativeVerticalPositionPage Then
                    shapetop(I) = myshape.top + myshape.Anchor.Information(wdVerticalPositionRelativeToPage)
                Else
                    shapetop(I) = myshape.top
                End If
            ElseIf myshape.Type = msoTextBox Then
                myshape.TextFrame.TextRange.Select
        
                shapetop(I) = Selection.Information(wdVerticalPositionRelativeToPage)
            End If
            I = I + 1
        Next myshape
    End If
    
    currDoc.Content.Select
    Selection.GoTo what:=wdGoToPage, Which:=wdGoToAbsolute, _
           count:=myPrepInfo.HF_OnPage 'set frametop might change the selection position
    
    If myPrepInfo.HF_inheader Then
        currDoc.ActiveWindow.ActivePane.View.SeekView = wdSeekCurrentPageHeader
        Selection.MoveStart
        ELength = 0
        While ELength < myPrepInfo.HF_extendLength
            Selection.TypeParagraph
            ELength = ELength + Selection.Characters.First.Font.Size
        Wend
    Else
        currDoc.ActiveWindow.ActivePane.View.SeekView = wdSeekCurrentPageFooter
        Selection.MoveStart
        ELength = 0
        While ELength < myPrepInfo.HF_extendLength
            Selection.TypeParagraph
            ELength = ELength + Selection.Characters.First.Font.Size
        Wend
    End If

    If Snum <> 0 Then
        I = 0
        For Each myshape In myPrepInfo.HF_Shapes
            If myshape.Type = msoPicture Then
                If myshape.RelativeVerticalPosition <> wdRelativeVerticalPositionPage Then
                    temptop = myshape.top + myshape.Anchor.Information(wdVerticalPositionRelativeToPage)
                Else
                    temptop = myshape.top
                End If
            ElseIf myshape.Type = msoTextBox Then
                myshape.TextFrame.TextRange.Select
        
                temptop = Selection.Information(wdVerticalPositionRelativeToPage)
            End If
            Selection.GoTo what:=wdGoToPage, Which:=wdGoToAbsolute, _
            count:=myPrepInfo.HF_OnPage
            If myPrepInfo.HF_inheader Then
                currDoc.ActiveWindow.ActivePane.View.SeekView = wdSeekCurrentPageHeader
            Else
                currDoc.ActiveWindow.ActivePane.View.SeekView = wdSeekCurrentPageFooter
            End If
            Selection.HeaderFooter.Shapes(myshape.name).Select
            Selection.ShapeRange.IncrementTop shapetop(I) - temptop
            I = I + 1
        Next myshape
    End If
    ActiveWindow.ActivePane.View.SeekView = wdSeekMainDocument
    Prepare_HeaderFooter_GraphicFrames = True
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

'Stub for Excel Prepare SheetName
Function Prepare_WorkbookVersion() As Boolean
    Prepare_WorkbookVersion = False
End Function


