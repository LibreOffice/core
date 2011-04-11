'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Helpers for accessing treelists
'*
'\******************************************************************************

function hGetNodeCount( oControl as object ) as integer
    
    '///<h3>Retrieve the number of visible (open) nodes from a treelist</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Treelist control object (Object)</li>
    '///<ul>
    '///+<li>Object must exist in current context</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of items in treelist</li>
    '///<ul>
    '///+<li>0 on any error</li>
    '///+<li>&gt; 0 Number of items</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    
    const CFN = "hGetNodeCount::"
    const RETVAL_FAILURE = 0
    dim iCount as integer
    
    '///+<li>Verify that the control exists</li>
    if ( oControl.exists( 5 ) ) then
    
        '///+<li>Verify that the control is enabled</li>
        if ( oControl.isEnabled() ) then

            if ( GVERBOSE ) then printlog( CFN & "Regular access, control available and enabled" )
            
            '///+<li>get the number of items from the control</li>
            iCount = oControl.getItemCount()
        else
            printlog( CFN & "Failure: Control claims to be disabled." )
            iCount = RETVAL_FAILURE
        endif
    else
        try
            printlog( CFN & "Forcing access to non existing control" )
            iCount = oControl.getItemCount()
        catch
            printlog( CFN & "Failure: Control not available." )
            iCount = RETVAL_FAILURE
        endcatch
    endif

    if ( GVERBOSE ) then printlog( CFN & "Exit with nodecount = " & iCount )

    hGetNodeCount = iCount
    '///</ul>
    
end function

'*******************************************************************************

function hSelectTopNode( oControl as object ) as boolean
    
    '///<h3>Move selection to the first node in a treelist</h3>
    '///<ul>
    '///+<li>Type the &quot;Home&quot;-key in a treelist, to select index 1</li>
    '///+<li>Verify that the top node has been selected</li>
    '///</ul>

    const CFN = "hSelectTopNode::"
    
    try
        oControl.select( 1 ) 
        WaitSlot()
        hSelectTopNode() = true
    catch
        hSelectTopNode() = false
    endcatch
    
    if ( GVERBOSE ) then printlog( CFN & "Selected Node: " & oControl.getText() )
    
end function

'*******************************************************************************

function hSelectNextNode( oControl as object ) as integer
    
    '///<h3>Move one node down in a treelist</h3>
    '///<ul>
    '///+<li>Returns new position or 0 on error</li>
    '///</ul>
    
    const CFN = "hSelectNextNode::"
    
    dim iPosBefore as integer
    dim iPosAfter as integer
    
    iPosBefore = oControl.getSelIndex()
    
    oControl.typeKeys( "<DOWN>" )
    
    iPosAfter = oControl.getSelIndex()
    
    if ( iPosAfter = iPosBefore ) then hSelectNextNode() = 0
    
    if ( iPosAfter = ( iPosBefore + 1 ) ) then hSelectNextNode() = iPosAfter
    
    printlog( CFN & "Selected node: " & oControl.getText() )
    
end function

'*******************************************************************************

function hGetNodeName( oControl as object , iNode as integer ) as string
    
    '///<h3>Retrieve the name of a node in a treelist specified by index</h3>
    '///<ul>
    '///+<li>Returns the name of the node or empty string on error</li>
    '///</ul>
    const CFN = "hGetNodeName::"
    dim iItemCount as integer
    
    iItemCount = hGetNodeCount( oControl )
    if ( iNode > iItemCount ) then
        warnlog( CFN & "Selected node out of range, aborting" )
        hGetNodeName() = ""
    else
        oControl.select( iNode )
        hGetNodeName() = oControl.getSelText()
    endif
    
end function

'*******************************************************************************

function hExpandNode( oControl as object, iNode as integer ) as integer
    
    '///<h3>Expand a node in a treelist specified by index</h3>
    '///<ul>
    '///+<li>Returns new nodecount or 0 on error</li>
    '///</ul>
    const RC_FAILURE = 0
    const CFN = "hExpandNode::"
    
    dim iOldNodeCount as integer
    
    if ( GVERBOSE ) then
        printlog( CFN & "Enter with option (Control): " & oControl.name() )
        printlog( CFN & "Enter with option (iNode): " & iNode )
    endif
    
    iOldNodeCount = hGetNodeCount( oControl )
    if ( iNode <= iOldNodeCount ) then
        if ( iNode > RC_FAILURE ) then
            oControl.select( iNode )
        endif
        try
            oControl.typekeys( "<RIGHT>" )
            hExpandNode() = hGetNodeCount( oControl )
        catch
            warnlog( "#i84194# Treelist access failed (Keyboard navigation)" )
            hExpandNode() = RC_FAILURE
        endcatch
    else
        hExpandNode() = RC_FAILURE
    endif
    
end function

'*******************************************************************************

function hExpandAllNodes( oControl as object ) as integer
    
    '///<h3>Expand all nodes of treelist</h3>
    '///<ul>
    '///+<li>Run through all items in the treelist, expand every node</li>
    '///+<li>Returns the number of all nodes in the treelist</li>
    '///</ul>
    
    dim iNode as integer
    dim iNodeCurCount as integer
    dim iNodeRefCount as integer
    dim iIteration as integer
    
    const CFN = "hExpandAllNodes::"
    
    hSelectTopNode( oControl )
    iNodeCurCount = hGetNodeCount( oControl )
    iNodeRefCount = -1
    iIteration = 0
    
    if ( GVERBOSE ) then
        printlog( CFN & "Initial iNodeCurCount: " & iNodeCurCount )
        printlog( CFN & "Initial iNodeRefCount: " & iNodeRefCount )    
    endif
    
    do while ( iNodeRefCount < iNodeCurCount )
    
        iIteration = iIteration + 1
        iNodeRefCount = iNodeCurCount
    
        for iNode = iNodeCurCount to 1 step -1 
            hExpandNode( oControl , iNode )
        next iNode
        
        iNodeCurCount = hGetNodeCount( oControl )
        
        if ( GVERBOSE ) then
            printlog( "" )
            printlog( CFN & "Exit iteration....: " & iIteration    )
            printlog( CFN & "Exit iNodeCurCount: " & iNodeCurCount )
            printlog( CFN & "Exit iNodeRefCount: " & iNodeRefCount )    
        endif
        
    loop
    
    if ( GVERBOSE ) then
        printlog( CFN & "Exit with " & iNodeCurCount & _
        " items after " & iIteration & " iterations" )
    endif
    hExpandAllNodes() = iNodeCurCount
    
end function

'*******************************************************************************

function hGetAllNodeNames( oControl as object , lsList() as string ) as integer

    hExpandAllNodes( oControl )
    hGetAllNodeNames() = hGetVisibleNodeNames( oControl, lsList() )

end function

'*******************************************************************************

function hGetVisibleNodeNames( oControl as object , lsList() as string ) as integer
    
    '///<h3>Retrieve the names of all nodes in a treelist</h3>
    '///<ul>
    '///+<li>Expand all nodes of a treelist</li>
    '///+<li>Store all node names into an array</li>
    '///+<li>Return the number of nodes read (listcount), 0 on error</li>
    '///</ul>
    
    ' Get the list of all visible (expanded) nodes and store it
    ' in lsList(). if _id > ubound lsList() an error is given and lsList remains
    ' empty, thus returning "0"
    
    const CFN = "hGetVisibleNodeNames::"
    
    dim iNodeCount as integer
    dim iThisNode as integer
    
    if ( GVERBOSE ) then printlog( CFN & "Enter" )
    
    iNodeCount = hGetNodeCount( oControl )

    ' Test whether the array provided is large enough to hold all items
    ' from the treelist/list. If the array is ok fill it.    
    if ( iNodeCount > ubound( lsList() ) ) then
        warnlog( "Array to small to hold: " & iNodeCount & " items" )
    else
        hSelectTopNode( oControl )
        for iThisNode = 1 to iNodeCount
            listappend( lsList() , hGetNodeName( oControl , iThisNode )
        next iThisNode
    endif
    
    hGetVisibleNodeNames() = listcount( lsList() )
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
    
end function

'*******************************************************************************

function hSelectNode( oControl as object , _id as integer ) as string
    
    '///<h3>Select a node in a treelist by index</h3>
    '///<ul>
    '///+<li>Return the name of the selected node</li>
    '///</ul>
    
    oControl.select( _id ) : hSelectNode() = hGetNodeName( oControl , _id )
    
end function

'*******************************************************************************

function hSelectNodeByName( oControl as object , _name as string ) as integer
    
    '///<h3>Select a node by name in treelist (first occurrence)</h3>
    '///<ul>
    '///+<li>Try to find a node by name - directly</li>
    '///+<li>If the node is not visible, expand all and search the tree</li>
    '///+<li>Returns index of requested node or 0 on failure</li>
    '///</ul>
    
    const CFN = "hSelectNodeByName::"
    const RETVAL_FAILURE = 0
    
    dim iThisNode as integer
    dim iCurrentNode as integer
    dim iNodeCount as integer
    
    dim cNodeName as string
    
    if ( GVERBOSE ) then printlog( CFN & "Enter with option (NodeName): " & _name )
    
    iThisNode = RETVAL_FAILURE
    
    ' First we try to jump to the node directly, if this fails we use the
    ' slower but safer method (expand all nodes and step through)
    try
        oControl.select( _name )
        iThisNode = oControl.getSelIndex()
        hSelectNodeByName() = iThisNode
    catch
        printlog( CFN & "Node not visible: Using search method" )
        iNodeCount = hExpandAllNodes( oControl )
        
        for iCurrentNode = 1 to iNodeCount
            oControl.select( iCurrentNode )
            cNodeName = oControl.getSelText()
            
            if ( cNodeName = _name ) then
                iThisNode = iCurrentNode
                exit for
            endif

            if ( instr( cNodeName, _name ) > 0 ) then
                if ( instr( cNodeName, "(" ) > 0 ) then
                    printlog( CFN & "Node has readonly marker" )
                    iThisNode = iCurrentNode
                    exit for
                else
                    qaerrorlog( CFN & "Fuzzy match. This might or might not be the correct node" )
                endif
            endif
        next iCurrentNode
    endcatch
    
    if ( iThisNode = RETVAL_FAILURE ) then
        printlog( CFN & "Exit: Node not found." )
    else
        if ( GVERBOSE ) then printlog( CFN & "Exit: Node selected at pos: " & iThisNode )
    endif
    
    hSelectNodeByName() = iThisNode
    
end function

'*******************************************************************************

function hSelectTheLastNode( oControl as object ) as integer
    
    '///<h3>Select the (absolute) last node in a treelist</h3>
    '///<ul>
    '///+<li>Expand all nodes</li>
    '///+<li>Go to the last node, select it</li>
    '///+<li>Return the number of the last node in the treelist</li>
    '///</ul>
    
    const CFN = "hSelectTheLastNode::"
    dim iCurrentNodeCount as integer
    
    iCurrentNodeCount = hExpandAllNodes( oControl )
    if ( iCurrentNodeCount > 0 ) then
        oControl.select( iCurrentNodeCount )
        if ( GVERBOSE ) then
            printlog( CFN & "Nodename.....: " & oControl.getText() )
            printlog( CFN & "Node position: " & iCurrentNodeCount )
        endif
    else
        iCurrentNodeCount = -1
    endif
    
    hSelectTheLastNode() = iCurrentNodeCount
    
end function

'*******************************************************************************

function hVerifyNodeName( oControl as object , cName as string ) as boolean
    
    '///<h3>Compare the name of the current node from a treelist to a reference string</h3>
    '///<ul>
    '///+<li>Returns true on success, false on failure</li>
    '///</ul>
    
    hVerifyNodeName() = false
    if ( oControl.getSelText() = cName ) then hVerifyNodeName() = true
    
end function

'******************************************************************************

function hGetListItems( oControl as object, aList() as string ) as integer
    
    '///<h3>Retrieve the names of all nodes from a plain (linear) list</h3>
    '///<ul>
    '///+<li>Cycle through a list, append all entries to an array</li>
    '///+<li>Returns number of items or 0 on error</li>
    '///</ul>
    
    const CFN = "hGetListItems::"
    const RETVAL_FAILURE = 0
    
    if ( GVERBOSE ) then printlog( CFN & "Enter with option (control): " & oControl.name() )
    
    dim iItemCount as integer
    dim iCurrentItem as integer
    dim cCurrentItem as string
    
    iItemCount = oControl.getItemCount()
    if ( iItemCount > ubound( aList() ) ) then
        printlog( CFN & "Array too small, needed: " & iItemCount )
        hGetListItems() = RETVAL_FAILURE
        exit function
    endif
    
    for iCurrentItem = 1 to iItemCount
        
        oControl.select( iCurrentItem )
        cCurrentItem = oControl.getSelText()
        hListAppend( cCurrentItem, aList() )
        
    next iCurrentItem
    
    if ( GVERBOSE ) then printlog( CFN & "Exit with  number of items: " & iItemCount )
    hGetListItems() = iItemCount
    
end function

'*******************************************************************************

function hFindInList( oControl as object, cObject as string ) as integer

    const RETVAL_FAILURE = 0
    dim iCurrentObject as integer

    for iCurrentObject = 1 to oControl.getItemCount()

        oControl.select( iCurrentObject )
        
        if ( oControl.getSelText() = cObject ) then
            hFindInList() = iCurrentObject
            exit function
        endif

    next iCurrentObject

    hFindInList() = RETVAL_FAILURE

end function