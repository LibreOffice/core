'encoding UTF-8  Do not remove or change this line!
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

*****

sub hGetScriptNames( oTreeList as object, oScriptList as object, cItemList() as string )

    ' oTreeList is the treelist object (left pane)
    ' oScriptList is the script list (right pane)
    ' cScriptList() is filled with the scriptnames of the form
    ' <module>:<script name> like "CommonLang:LoadLanguages"

    dim iCurrentTreeListItem as integer
    dim iObjectsInTreelist as integer

    dim cModuleName as string
    dim cReferenceString as string

    dim iScriptCount as integer
    dim iCurrentScript as integer
    dim cScriptList( 1000 ) as string

    ' Get the number of nodes from the modules treelist
    iObjectsInTreeList = hExpandAllNodes( oTreeList )

    ' For each item in the modules treelist get the number of scripts (right pane)
    for iCurrentTreeListItem = 1 to iObjectsInTreeList

        ' Get the name of the current treelist node, append it to the list
        ' (otherwise it does not appear if it has no scripts)
        cModuleName = hGetNodeName( oTreeList, iCurrentTreeListItem )
        hListAppend( cModuleName, cItemList() )

        ' Get the number of scripts from the right pane, reset the array in advance
        ListAllDelete( cScriptList() )
        iScriptCount = hGetListItems( oScriptList, cScriptList() )

        ' Build the string for the reference list and append it to cScriptList()
        for iCurrentScript = 1 to iScriptCount
            cReferenceString = cModuleName & ":" & cScriptList( iCurrentScript )
            hListAppend( cReferenceString, cItemList() )
        next iCurrentScript

    next iCurrentTreeListItem

end sub