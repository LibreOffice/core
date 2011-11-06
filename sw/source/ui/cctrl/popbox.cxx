/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <swtypes.hxx>
#include <popbox.hxx>
#include <navipi.hxx>


// --- class SwHelpToolBox ---------------------------------------------


SwHelpToolBox::SwHelpToolBox( SwNavigationPI* pParent, const ResId& rResId )
    : ToolBox( pParent, rResId ),
    DropTargetHelper( this )
{
}


void SwHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
        // Zuerst DoubleClick-Link auswerten
        // Dessen Returnwert entscheidet ueber andere Verarbeitung
        // Doppelclickhandler nur, wenn nicht auf einen Button geclickt wurde
    if(rEvt.GetButtons() == MOUSE_RIGHT &&
        0 == GetItemId(rEvt.GetPosPixel()))
    {
        aRightClickLink.Call((MouseEvent *)&rEvt);
    }
    else
        ToolBox::MouseButtonDown(rEvt);
}


long SwHelpToolBox::DoubleClick( ToolBox* pCaller )
{
        // kein Doppelklick auf einen Button
    if( 0 == pCaller->GetCurItemId() && aDoubleClickLink.Call(0) )
        return sal_True;
    return sal_False;
}

/*-----------------26.02.94 00:36-------------------
 dtor ueberladen
--------------------------------------------------*/


SwHelpToolBox::~SwHelpToolBox() {}

sal_Int8 SwHelpToolBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->AcceptDrop( rEvt );
}

sal_Int8 SwHelpToolBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->ExecuteDrop( rEvt );
}


