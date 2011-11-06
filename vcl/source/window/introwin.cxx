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
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/introwin.hxx>

#include <impbmp.hxx>
#include <svdata.hxx>
#include <salframe.hxx>



// =======================================================================

void IntroWindow::ImplInitIntroWindowData()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpIntroWindow = this;
}

// -----------------------------------------------------------------------

IntroWindow::IntroWindow( ) :
    WorkWindow( WINDOW_INTROWINDOW )
{
    ImplInitIntroWindowData();
    WorkWindow::ImplInit( 0, WB_INTROWIN, NULL );
}

// -----------------------------------------------------------------------

IntroWindow::~IntroWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->mpIntroWindow == this )
        pSVData->mpIntroWindow = NULL;
}

void IntroWindow::SetBackgroundBitmap( const Bitmap& rBitmap )
{
    if( ! rBitmap.IsEmpty() )
    {
        SalBitmap* pBmp = rBitmap.ImplGetImpBitmap()->ImplGetSalBitmap();
        ImplGetFrame()->SetBackgroundBitmap( pBmp );
    }
}

void IntroWindow::SetBackgroundBitmap( const BitmapEx& rBitmapEx )
{
    if( ! rBitmapEx.IsEmpty() )
    {
        SalBitmap* pBmp = rBitmapEx.ImplGetBitmapImpBitmap()->ImplGetSalBitmap();
        ImplGetFrame()->SetBackgroundBitmap( pBmp );
    }
}
