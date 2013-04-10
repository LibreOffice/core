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


#include <hintids.hxx>

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif








#include <svl/stritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <doc.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // Listen, bzw. Tables im ItemSet der DocShell anlegen
        PutItem( SvxGradientListItem( pDrDoc->GetGradientList(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem( pDrDoc->GetHatchList(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem( pDrDoc->GetBitmapList(), SID_BITMAP_LIST ) );
        PutItem( SvxDashListItem( pDrDoc->GetDashList(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem( pDrDoc->GetLineEndList(), SID_LINEEND_LIST ) );

        Outliner& rOutliner = pDrDoc->GetDrawOutliner();
        uno::Reference<linguistic2::XHyphenator> xHyphenator( ::GetHyphenator() );
        rOutliner.SetHyphenator( xHyphenator );
    }
    else
        PutItem( SvxColorTableItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ));
}



