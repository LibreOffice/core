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

function hToggleSearchDialog()

    printlog( "Make button <Search Dialog> visible via context menu" )

    const MENU_VISIBLE_BUTTONS = 1
    const MENU_SEARCH_AND_REPLACE = 4

    kontext "FindBar"
    FindBar.openContextMenu()
    wait( 1000 )

    hMenuSelectNr( MENU_VISIBLE_BUTTONS )
    wait( 1000 )

    hMenuSelectNr( MENU_SEARCH_AND_REPLACE )
    wait( 1000 )

end function