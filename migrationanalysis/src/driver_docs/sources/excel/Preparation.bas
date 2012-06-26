Attribute VB_Name = "Preparation"
'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
' This file incorporates work covered by the following license notice:
'
'   Licensed to the Apache Software Foundation (ASF) under one or more
'   contributor license agreements. See the NOTICE file distributed
'   with this work for additional information regarding copyright
'   ownership. The ASF licenses this file to you under the Apache
'   License, Version 2.0 (the "License"); you may not use this file
'   except in compliance with the License. You may obtain a copy of
'   the License at http://www.apache.org/licenses/LICENSE-2.0 .
'

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


