VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "DocumentAnalysis"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
Option Explicit

Const CMAX_NUM_ISSUETYPES = 100

Private mName As String
Private mApplication As String
Private mPageCount As Long
Private mCreated As Date
Private mModified As Date
Private mAccessed As Date
Private mPrinted As Date
Private mSavedBy As String
Private mRevision As Long
Private mTemplate As String
Private mHasMacros As Boolean

Private mDocIssuesCosts As Long
Private mPreparableIssuesCosts As Long
Private mMacroCosts As Long

Private mReferences As Collection
Private mIssues As Collection
Private mIssuesCountArray(1 To CMAX_NUM_ISSUETYPES) As Long
Private mTotalIssueTypes As Long
Private mMinorIssuesCount As Long
Private mMacroIssuesCount As Long
Private mPreparableIssuesCount As Long

Private mDocOverallIssueClass As EnumDocOverallIssueClass

Private mMacroTotalNumLines As Long
Private mMacroNumUserForms As Long
Private mMacroNumUserFormControls As Long
Private mMacroNumUserFormControlTypes As Long
Private mMacroNumOLEControls As Long
Private mMacroNumFieldsUsingMacros As Long
Private mMacroNumExternalRefs As Long
Private mMacroOverallClass As EnumDocOverallMacroClass
Private mbBelowIssuesLimit As Boolean

Public Property Get BelowIssuesLimit() As Boolean
    BelowIssuesLimit = mbBelowIssuesLimit
End Property
Public Property Let BelowIssuesLimit(vNewValue As Boolean)
    mbBelowIssuesLimit = vNewValue
End Property

Public Property Get MacroOverallClass() As EnumDocOverallMacroClass
    MacroOverallClass = mMacroOverallClass
End Property
Public Property Let MacroOverallClass(vNewValue As EnumDocOverallMacroClass)
    mMacroOverallClass = vNewValue
End Property

Public Property Get MacroNumExternalRefs() As Long
    MacroNumExternalRefs = mMacroNumExternalRefs
End Property
Public Property Let MacroNumExternalRefs(vNewValue As Long)
    mMacroNumExternalRefs = vNewValue
End Property

Public Property Get MacroNumFieldsUsingMacros() As Long
    MacroNumFieldsUsingMacros = mMacroNumFieldsUsingMacros
End Property
Public Property Let MacroNumFieldsUsingMacros(vNewValue As Long)
    mMacroNumFieldsUsingMacros = vNewValue
End Property

Public Property Get MacroNumOLEControls() As Long
    MacroNumOLEControls = mMacroNumOLEControls
End Property
Public Property Let MacroNumOLEControls(vNewValue As Long)
    mMacroNumOLEControls = vNewValue
End Property

Public Property Get MacroNumUserFormControlTypes() As Long
    MacroNumUserFormControlTypes = mMacroNumUserFormControlTypes
End Property
Public Property Let MacroNumUserFormControlTypes(vNewValue As Long)
    mMacroNumUserFormControlTypes = vNewValue
End Property

Public Property Get MacroNumUserFormControls() As Long
    MacroNumUserFormControls = mMacroNumUserFormControls
End Property
Public Property Let MacroNumUserFormControls(vNewValue As Long)
    mMacroNumUserFormControls = vNewValue
End Property

Public Property Get MacroNumUserForms() As Long
    MacroNumUserForms = mMacroNumUserForms
End Property
Public Property Let MacroNumUserForms(vNewValue As Long)
    mMacroNumUserForms = vNewValue
End Property

Public Property Get MacroTotalNumLines() As Long
    MacroTotalNumLines = mMacroTotalNumLines
End Property
Public Property Let MacroTotalNumLines(vNewValue As Long)
    mMacroTotalNumLines = vNewValue
End Property

Public Property Get MacroCosts() As Long
    MacroCosts = mMacroCosts
End Property
Public Property Let MacroCosts(vNewValue As Long)
    mMacroCosts = vNewValue
End Property

Public Property Get DocIssuesCosts() As Long
    DocIssuesCosts = mDocIssuesCosts
End Property
Public Property Let DocIssuesCosts(vNewValue As Long)
    mDocIssuesCosts = vNewValue
End Property

Public Property Get PreparableIssuesCosts() As Long
    PreparableIssuesCosts = mPreparableIssuesCosts
End Property
Public Property Let PreparableIssuesCosts(vNewValue As Long)
    mPreparableIssuesCosts = vNewValue
End Property

Public Property Get DocOverallIssueClass() As EnumDocOverallIssueClass
    DocOverallIssueClass = mDocOverallIssueClass
End Property

Public Property Let DocOverallIssueClass(ByVal vNewValue As EnumDocOverallIssueClass)
    mDocOverallIssueClass = vNewValue
End Property

Public Property Get TotalIssueTypes() As Long
    TotalIssueTypes = mTotalIssueTypes
End Property

Public Property Let TotalIssueTypes(ByVal vNewValue As Long)
    mTotalIssueTypes = vNewValue
End Property
Public Property Get name() As String
    name = mName
End Property

Public Property Let name(ByVal vNewValue As String)
    mName = vNewValue
End Property

Public Property Get Application() As String
    Application = mApplication
End Property

Public Property Let Application(ByVal vNewValue As String)
    mApplication = vNewValue
End Property
Public Property Get PageCount() As Long
    PageCount = mPageCount
End Property

Public Property Let PageCount(ByVal vNewValue As Long)
    mPageCount = vNewValue
End Property

Public Property Get Created() As Date
    Created = mCreated
End Property

Public Property Let Created(ByVal vNewValue As Date)
    mCreated = vNewValue
End Property

Public Property Get Modified() As Date
    Modified = mModified
End Property

Public Property Let Modified(ByVal vNewValue As Date)
    mModified = vNewValue
End Property

Public Property Get Accessed() As Date
    Accessed = mAccessed
End Property

Public Property Let Accessed(ByVal vNewValue As Date)
    mAccessed = vNewValue
End Property
Public Property Get Printed() As Date
    Printed = mPrinted
End Property

Public Property Let Printed(ByVal vNewValue As Date)
    mPrinted = vNewValue
End Property

Public Property Get SavedBy() As String
    SavedBy = mSavedBy
End Property

Public Property Let SavedBy(ByVal vNewValue As String)
    mSavedBy = vNewValue
End Property

Public Property Get Revision() As Long
    Revision = mRevision
End Property

Public Property Let Revision(ByVal vNewValue As Long)
    mRevision = vNewValue
End Property

Public Property Get Template() As String
    Template = mTemplate
End Property

Public Property Let Template(ByVal vNewValue As String)
    mTemplate = vNewValue
End Property
Public Property Get HasMacros() As Boolean
    HasMacros = mHasMacros
End Property

Public Property Let HasMacros(ByVal vNewValue As Boolean)
    mHasMacros = vNewValue
End Property

Public Property Get References() As Collection
    Set References = mReferences
End Property

Public Property Let References(ByVal vNewValue As Collection)
    Set mReferences = vNewValue
End Property

Public Property Get Issues() As Collection
    Set Issues = mIssues
End Property

Public Property Let Issues(ByVal vNewValue As Collection)
    Set mIssues = vNewValue
End Property

Public Property Get IssuesCountArray(ByVal index As Integer) As Long
    If index >= LBound(mIssuesCountArray) And index <= UBound(mIssuesCountArray) Then
        IssuesCountArray = mIssuesCountArray(index)
    Else
        IssuesCountArray = Null
    End If
End Property

Public Property Let IssuesCountArray(ByVal index As Integer, ByVal vNewValue As Long)
    If index >= LBound(mIssuesCountArray) And index <= UBound(mIssuesCountArray) Then
        mIssuesCountArray(index) = vNewValue
    End If
End Property

Public Property Get IssuesCount() As Long
    IssuesCount = mIssues.count
End Property

Public Property Get ComplexIssuesCount() As Long
    Dim complexCount As Long
    complexCount = mIssues.count - mMinorIssuesCount - mMacroIssuesCount
    ComplexIssuesCount = IIf(complexCount > 0, complexCount, 0)
End Property

Public Property Get MacroIssuesCount() As Long
    MacroIssuesCount = mMacroIssuesCount
End Property

Public Property Let MacroIssuesCount(ByVal vNewValue As Long)
    mMacroIssuesCount = vNewValue
End Property

Public Property Get MinorIssuesCount() As Long
    MinorIssuesCount = mMinorIssuesCount
End Property

Public Property Let MinorIssuesCount(ByVal vNewValue As Long)
    mMinorIssuesCount = vNewValue
End Property

Public Property Get PreparableIssuesCount() As Long
    PreparableIssuesCount = mPreparableIssuesCount
End Property

Public Property Let PreparableIssuesCount(ByVal vNewValue As Long)
    mPreparableIssuesCount = vNewValue
End Property

Private Sub Class_Initialize()
    mApplication = ""
    mTotalIssueTypes = 0
    mHasMacros = False
    Set mIssues = New Collection
    Set mReferences = New Collection
    mPreparableIssuesCount = 0
    mMacroOverallClass = enNone
    mDocOverallIssueClass = enNone
    mDocIssuesCosts = 0
    mPreparableIssuesCosts = 0
    mMacroCosts = 0
    mbBelowIssuesLimit = False
End Sub
Private Sub Class_Terminate()
    Set mIssues = Nothing
    Set mReferences = Nothing
End Sub

