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

'Stub for Word Prepare H&F
Function Prepare_HeaderFooter_GraphicFrames(docAnalysis As DocumentAnalysis, myIssue As IssueInfo, _
                                            var As Variant, currDoc As Workbook) As Boolean
    Prepare_HeaderFooter_GraphicFrames = False
End Function

Function Prepare_WorkbookVersion() As Boolean

    ' **************************************************************************
    ' Because the workbook version is changed when the workbook is being saved,
    ' the actual preparation for this issue is done in the sub DoAnalyze of
    ' the class module MigrationAnalyser when the prepared workbook is saved.
    ' The reason for having this function is more for documentation/structural
    ' purposes rather than actually needing the function.
    ' **************************************************************************
    
    Prepare_WorkbookVersion = True

End Function


