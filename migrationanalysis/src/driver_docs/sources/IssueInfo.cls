VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "IssueInfo"
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

Private mIssueID As Integer
Private mIssueType As String
Private mSubType As String
Private mIssueTypeXML As String
Private mSubTypeXML As String
Private mLocationXML As String
Private mLocation As String
Private mSubLocation As Variant
Private mLine As Long
Private mColumn As Variant
Private mAttributes As Collection
Private mValues As Collection
Private mPreparable As Boolean

'General Constants
Public Property Get CLocationDocument() As String
    CLocationDocument = RID_STR_COMMON_RESULTS_LOCATION_TYPE_DOCUMENT
End Property
Public Property Get CLocationPage() As String
    CLocationPage = RID_STR_COMMON_RESULTS_LOCATION_TYPE_PAGE
End Property
Public Property Get CLocationWorkBook() As String
    CLocationWorkBook = RID_STR_COMMON_RESULTS_LOCATION_TYPE_WORKBOOK
End Property
Public Property Get CLocationSheet() As String
    CLocationSheet = RID_STR_COMMON_RESULTS_LOCATION_TYPE_SHEET
End Property
Public Property Get CLocationPresentation() As String
    CLocationPresentation = RID_STR_COMMON_RESULTS_LOCATION_TYPE_PRESENTATION
End Property
Public Property Get CLocationSlide() As String
    CLocationSlide = RID_STR_COMMON_RESULTS_LOCATION_TYPE_SLIDE
End Property

'General XML Constants - NOT localised
Public Property Get CXMLLocationDocument() As String
    CXMLLocationDocument = "Document"
End Property
Public Property Get CXMLLocationPage() As String
    CXMLLocationPage = "Page"
End Property
Public Property Get CXMLLocationWorkBook() As String
    CXMLLocationWorkBook = "Workbook"
End Property
Public Property Get CXMLLocationSheet() As String
    CXMLLocationSheet = "Sheet"
End Property
Public Property Get CXMLLocationPresentation() As String
    CXMLLocationPresentation = "Presentation"
End Property
Public Property Get CXMLLocationSlide() As String
    CXMLLocationSlide = "Slide"
End Property

'Settable Properties
Public Property Get IssueID() As Integer
    IssueID = mIssueID
End Property

Public Property Let IssueID(ByVal vNewValue As Integer)
    mIssueID = vNewValue
End Property
Public Property Get IssueType() As String
    IssueType = mIssueType
End Property

Public Property Let IssueType(ByVal vNewValue As String)
    mIssueType = vNewValue
End Property
Public Property Get IssueTypeXML() As String
    IssueTypeXML = mIssueTypeXML
End Property

Public Property Let IssueTypeXML(ByVal vNewValue As String)
    mIssueTypeXML = vNewValue
End Property
Public Property Get SubType() As String
    SubType = mSubType
End Property

Public Property Let SubType(ByVal vNewValue As String)
    mSubType = vNewValue
End Property
Public Property Get SubTypeXML() As String
    SubTypeXML = mSubTypeXML
End Property

Public Property Let SubTypeXML(ByVal vNewValue As String)
    mSubTypeXML = vNewValue
End Property

Public Property Get Location() As String
    Location = mLocation
End Property

Public Property Let Location(ByVal vNewValue As String)
    mLocation = vNewValue
End Property
Public Property Get locationXML() As String
    locationXML = mLocationXML
End Property

Public Property Let locationXML(ByVal vNewValue As String)
    mLocationXML = vNewValue
End Property

Public Property Get SubLocation() As Variant
    SubLocation = mSubLocation
End Property

Public Property Let SubLocation(ByVal vNewValue As Variant)
    mSubLocation = vNewValue
End Property

Public Property Get Line() As Long
    Line = mLine
End Property

Public Property Let Line(ByVal vNewValue As Long)
    mLine = vNewValue
End Property
Public Property Get column() As Variant
    column = mColumn
End Property

Public Property Let column(ByVal vNewValue As Variant)
    mColumn = vNewValue
End Property

Public Property Get Attributes() As Collection
    Set Attributes = mAttributes
End Property

Public Property Let Attributes(ByVal vNewValue As Collection)
    Set mAttributes = vNewValue
End Property
Public Property Get Values() As Collection
    Set Values = mValues
End Property

Public Property Let Values(ByVal vNewValue As Collection)
    Set mValues = vNewValue
End Property

Public Property Get Preparable() As Boolean
    Preparable = mPreparable
End Property

Public Property Let Preparable(ByVal vNewValue As Boolean)
    mPreparable = vNewValue
End Property



Private Sub Class_Initialize()
    Set mAttributes = New Collection
    Set mValues = New Collection
    mIssueID = -1
    mLine = -1
    mColumn = ""
    mSubLocation = ""
    mPreparable = False
End Sub
Private Sub Class_Terminate()
    Set mAttributes = Nothing
    Set mValues = Nothing
End Sub

