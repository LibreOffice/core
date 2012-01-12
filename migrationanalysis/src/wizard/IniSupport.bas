Attribute VB_Name = "IniSupport"
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

Private Declare Function GetPrivateProfileString Lib "kernel32" _
   Alias "GetPrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpDefault As String, _
   ByVal lpReturnedString As String, _
   ByVal nSize As Long, _
   ByVal lpFileName As String) As Long
   
Private Declare Function WritePrivateProfileString Lib "kernel32" _
   Alias "WritePrivateProfileStringA" _
  (ByVal lpSectionName As String, _
   ByVal lpKeyName As Any, _
   ByVal lpString As Any, _
   ByVal lpFileName As String) As Long


Public Function ProfileGetItem(lpSectionName As String, _
                               lpKeyName As String, _
                               defaultValue As String, _
                               inifile As String) As String

'Retrieves a value from an ini file corresponding
'to the section and key name passed.
        
   Dim success As Long
   Dim nSize As Long
   Dim ret As String
  
  'call the API with the parameters passed.
  'The return value is the length of the string
  'in ret, including the terminating null. If a
  'default value was passed, and the section or
  'key name are not in the file, that value is
  'returned. If no default value was passed (""),
  'then success will = 0 if not found.

  'Pad a string large enough to hold the data.
   ret = Space$(2048)
   nSize = Len(ret)
   success = GetPrivateProfileString(lpSectionName, _
                                     lpKeyName, _
                                     defaultValue, _
                                     ret, _
                                     nSize, _
                                     inifile)
   
   If success Then
      ProfileGetItem = Left$(ret, success)
   End If
   
End Function


Public Sub ProfileDeleteItem(lpSectionName As String, _
                             lpKeyName As String, _
                             inifile As String)

'this call will remove the keyname and its
'corresponding value from the section specified
'in lpSectionName. This is accomplished by passing
'vbNullString as the lpValue parameter. For example,
'assuming that an ini file had:
'  [Colours]
'  Colour1=Red
'  Colour2=Blue
'  Colour3=Green
'
'and this sub was called passing "Colour2"
'as lpKeyName, the resulting ini file
'would contain:
'  [Colours]
'  Colour1=Red
'  Colour3=Green
   
   Call WritePrivateProfileString(lpSectionName, _
                                  lpKeyName, _
                                  vbNullString, _
                                  inifile)

End Sub


Public Sub ProfileDeleteSection(lpSectionName As String, _
                                inifile As String)

'this call will remove the entire section
'corresponding to lpSectionName. This is
'accomplished by passing vbNullString
'as both the lpKeyName and lpValue parameters.
'For example, assuming that an ini file had:
'  [Colours]
'  Colour1=Red
'  Colour2=Blue
'  Colour3=Green
'
'and this sub was called passing "Colours"
'as lpSectionName, the resulting Colours
'section in the ini file would be deleted.
   
   Call WritePrivateProfileString(lpSectionName, _
                                  vbNullString, _
                                  vbNullString, _
                                  inifile)

End Sub

Private Function StripNulls(startStrg As String) As String

'take a string separated by nulls, split off 1 item, and shorten the string
'so the next item is ready for removal.
'The passed string must have a terminating null for this function to work correctly.
'If you remain in a loop, check this first!

   Dim pos As Long
   Dim item As String
   
   pos = InStr(1, startStrg, Chr$(0))
   
   If pos Then

      item = Mid$(startStrg, 1, pos - 1)
      startStrg = Mid$(startStrg, pos + 1, Len(startStrg))
      StripNulls = item
    
   End If

End Function

Public Function ProfileLoadList(lst As ComboBox, _
                                lpSectionName As String, _
                                inifile As String) As Long
   Dim success As Long
   Dim c As Long
   Dim nSize As Long
   Dim KeyData As String
   Dim lpKeyName As String
   Dim ret As String
  
  ' call the API passing lpKeyName = null. This causes
  ' the API to return a list of all keys under that section.
  ' Pad the passed string large enough to hold the data.
   ret = Space$(2048)
   nSize = Len(ret)
   success = GetPrivateProfileString( _
    lpSectionName, vbNullString, "", ret, nSize, inifile)
   
  ' The returned string is a null-separated list of key names,
  ' terminated by a pair of null characters.
  ' If the Get call was successful, success holds the length of the
  ' string in ret up to but not including that second terminating null.
  ' The ProfileGetItem function below extracts each key item using the
  ' nulls as markers, so trim off the terminating null.
   If success Then
    
     'trim terminating null and trailing spaces
      ret = Left$(ret, success)
      
        'with the resulting string extract each element
         Do Until ret = ""
           'strip off an item (i.e. "Item1", "Item2")
            lpKeyName = StripNulls(ret)
        
           'pass the lpKeyName received to a routine that
           'again calls GetPrivateProfileString, this
           'time passing the real key name. Returned
           'is the value associated with that key,
           'ie the "Apple" corresponding to the ini
           'entry "Item1=Apple"
            KeyData = ProfileGetItem( _
                lpSectionName, lpKeyName, "", inifile)
         
           'add the item retruned to the listbox
            lst.AddItem KeyData
         Loop
  
   End If
  
  'return the number of items as an
  'indicator of success
   ProfileLoadList = lst.ListCount
End Function

Public Function ProfileLoadDict(dict As Scripting.Dictionary, _
                                lpSectionName As String, _
                                inifile As String) As Long
   Dim success As Long
   Dim c As Long
   Dim nSize As Long
   Dim KeyData As String
   Dim lpKeyName As String
   Dim ret As String
  
  ' call the API passing lpKeyName = null. This causes
  ' the API to return a list of all keys under that section.
  ' Pad the passed string large enough to hold the data.
   ret = Space$(2048)
   nSize = Len(ret)
   success = GetPrivateProfileString( _
    lpSectionName, vbNullString, "", ret, nSize, inifile)
   
  ' The returned string is a null-separated list of key names,
  ' terminated by a pair of null characters.
  ' If the Get call was successful, success holds the length of the
  ' string in ret up to but not including that second terminating null.
  ' The ProfileGetItem function below extracts each key item using the
  ' nulls as markers, so trim off the terminating null.
   If success Then
    
     'trim terminating null and trailing spaces
      ret = Left$(ret, success)
      
        'with the resulting string extract each element
         Do Until ret = ""
           'strip off an item (i.e. "Item1", "Item2")
            lpKeyName = StripNulls(ret)
        
           'pass the lpKeyName received to a routine that
           'again calls GetPrivateProfileString, this
           'time passing the real key name. Returned
           'is the value associated with that key,
           'ie the "Apple" corresponding to the ini
           'entry "Item1=Apple"
            KeyData = ProfileGetItem( _
                lpSectionName, lpKeyName, "", inifile)
         
           dict.add lpKeyName, KeyData
         Loop
  
   End If
  
   ProfileLoadDict = dict.count
End Function







