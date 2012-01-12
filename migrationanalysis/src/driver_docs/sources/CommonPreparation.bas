Attribute VB_Name = "CommonPreparation"
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
Private Declare Function CryptAcquireContext Lib "advapi32.dll" _
   Alias "CryptAcquireContextA" (ByRef phProv As Long, _
   ByVal pszContainer As String, ByVal pszProvider As String, _
   ByVal dwProvType As Long, ByVal dwFlags As Long) As Long

Private Declare Function CryptReleaseContext Lib "advapi32.dll" ( _
   ByVal hProv As Long, ByVal dwFlags As Long) As Long

Private Declare Function CryptCreateHash Lib "advapi32.dll" ( _
   ByVal hProv As Long, ByVal Algid As Long, ByVal hKey As Long, _
   ByVal dwFlags As Long, ByRef phHash As Long) As Long

Private Declare Function CryptDestroyHash Lib "advapi32.dll" (ByVal hHash As Long) As Long

Private Declare Function CryptHashData Lib "advapi32.dll" (ByVal hHash As Long, _
    pbData As Any, ByVal dwDataLen As Long, ByVal dwFlags As Long) As Long

Private Declare Function CryptGetHashParam Lib "advapi32.dll" ( _
   ByVal hHash As Long, ByVal dwParam As Long, pbData As Any, _
   pdwDataLen As Long, ByVal dwFlags As Long) As Long
   
Private Const ALG_CLASS_ANY     As Long = 0
Private Const ALG_TYPE_ANY      As Long = 0
Private Const ALG_CLASS_HASH    As Long = 32768
Private Const ALG_SID_MD5       As Long = 3
' Hash algorithms
Private Const MD5_ALGORITHM As Long = ALG_CLASS_HASH Or ALG_TYPE_ANY Or ALG_SID_MD5
' CryptSetProvParam
Private Const PROV_RSA_FULL        As Long = 1
' used when acquiring the provider
Private Const CRYPT_VERIFYCONTEXT  As Long = &HF0000000
' Microsoft provider data
Private Const MS_DEFAULT_PROVIDER  As String = _
              "Microsoft Base Cryptographic Provider v1.0"

Function DoPreparation(docAnalysis As DocumentAnalysis, myIssue As IssueInfo, preparationNote As String, _
                       var As Variant, currDoc As Object) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "DoPreparation"
    
    DoPreparation = False
    
    'Log as Preparable
    AddIssueDetailsNote myIssue, 0, preparationNote, RID_STR_COMMON_PREPARATION_NOTE
    myIssue.Preparable = True
    docAnalysis.PreparableIssuesCount = docAnalysis.PreparableIssuesCount + 1
    
    If Not CheckDoPrepare Then Exit Function
 
    'Do Prepare

    If myIssue.IssueTypeXML = CSTR_ISSUE_OBJECTS_GRAPHICS_AND_FRAMES And _
        myIssue.SubTypeXML = CSTR_SUBISSUE_OBJECT_IN_HEADER_FOOTER Then
        DoPreparation = Prepare_HeaderFooter_GraphicFrames(docAnalysis, myIssue, var, currDoc)
        
    ElseIf myIssue.IssueTypeXML = CSTR_ISSUE_CONTENT_DOCUMENT_PROPERTIES And _
        myIssue.SubTypeXML = CSTR_SUBISSUE_OLD_WORKBOOK_VERSION Then
        DoPreparation = Prepare_WorkbookVersion()
        
    End If
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & _
    " : path " & docAnalysis.name & ": " & _
    " : myIssue " & myIssue.IssueTypeXML & "_" & myIssue.SubTypeXML & ": " & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function InDocPreparation() As Boolean
    InDocPreparation = True
End Function

Function Prepare_DocumentCustomProperties(docAnalysis As DocumentAnalysis, myIssue As IssueInfo, _
                                          var As Variant, currDoc As Object) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Prepare_DocumentCustomProperties"
    
    Dim aProp As DocumentProperty
    Dim myCustomDocumentProperties As DocumentProperties
    Dim commentProp As DocumentProperty
    Prepare_DocumentCustomProperties = False
    
    Set myCustomDocumentProperties = getAppSpecificCustomDocProperties(currDoc)
    Set commentProp = getAppSpecificCommentBuiltInDocProperty(currDoc)
    Set aProp = var 'Safe as we know that a DocumentProperty is being passed in
                  
    If commentProp.value <> "" Then commentProp.value = commentProp.value & vbLf

    commentProp.value = commentProp.value & _
                RID_STR_COMMON_SUBISSUE_DOCUMENT_CUSTOM_PROPERTY & ": " & vbLf
    
    commentProp.value = commentProp.value & _
        RID_STR_COMMON_ATTRIBUTE_NAME & " - " & aProp.name & ", " & _
        RID_STR_COMMON_ATTRIBUTE_TYPE & " - " & getCustomDocPropTypeAsString(aProp.Type) & ", " & _
        RID_STR_COMMON_ATTRIBUTE_VALUE & " - " & aProp.value

    myCustomDocumentProperties.item(aProp.name).Delete
    
    Prepare_DocumentCustomProperties = True
    
FinalExit:
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & docAnalysis.name & ": " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Private Function GetProvider(hCtx As Long) As Boolean
    Const NTE_BAD_KEYSET = &H80090016
    Const NTE_EXISTS = &H8009000F
    Const NTE_KEYSET_NOT_DEF = &H80090019
    Dim currentFunctionName As String
    currentFunctionName = "GetProvider"
    
    Dim strTemp       As String
    Dim strProvider  As String
    Dim strErrorMsg   As String
    Dim errStr As String
    
    GetProvider = False
    
    On Error Resume Next
    strTemp = vbNullChar
    strProvider = MS_DEFAULT_PROVIDER & vbNullChar
    If CBool(CryptAcquireContext(hCtx, ByVal strTemp, _
             ByVal strProvider, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) Then
        GetProvider = True
        Exit Function
    End If
    
    Select Case Err.LastDllError
        Case NTE_BAD_KEYSET
            errStr = "Key container does not exist or You do not have access to the key container."
        Case NTE_EXISTS
            errStr = "The key container already exists, but you are attempting to create it"
        Case NTE_KEYSET_NOT_DEF
            errStr = "The Crypto Service Provider (CSP) may not be set up correctly"
    End Select
    WriteDebug currentFunctionName & "Problems acquiring Crypto Provider: " & MS_DEFAULT_PROVIDER & ": " & errStr
End Function



Function MD5HashString(ByVal Str As String) As String
    Const HP_HASHVAL = 2
    Const HP_HASHSIZE = 4
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "MD5HashString"
    
    Dim hCtx As Long
    Dim hHash As Long
    Dim ret As Long
    Dim lLen As Long
    Dim lIdx As Long
    Dim abData() As Byte

    If Not GetProvider(hCtx) Then Err.Raise Err.LastDllError
    
    ret = CryptCreateHash(hCtx, MD5_ALGORITHM, 0, 0, hHash)
    If ret = 0 Then Err.Raise Err.LastDllError

    ret = CryptHashData(hHash, ByVal Str, Len(Str), 0)
    If ret = 0 Then Err.Raise Err.LastDllError

    ret = CryptGetHashParam(hHash, HP_HASHSIZE, lLen, 4, 0)
    If ret = 0 Then Err.Raise Err.LastDllError
            

    ReDim abData(0 To lLen - 1)
    ret = CryptGetHashParam(hHash, HP_HASHVAL, abData(0), lLen, 0)
    If ret = 0 Then Err.Raise Err.LastDllError

    For lIdx = 0 To UBound(abData)
        MD5HashString = MD5HashString & Right$("0" & Hex$(abData(lIdx)), 2)
    Next
    CryptDestroyHash hHash
   
    CryptReleaseContext hCtx, 0

FinalExit:
    Exit Function
    
HandleErrors:
    MD5HashString = ""
    WriteDebug currentFunctionName & _
    Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

