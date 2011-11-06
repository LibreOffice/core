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


#include <SwPortionHandler.hxx>
#include "viewopt.hxx"  // SwViewOptions

#include "txtcfg.hxx"
#include "portox.hxx"
#include "inftxt.hxx"       // GetTxtSize()

/*************************************************************************
 *               virtual SwToxPortion::Paint()
 *************************************************************************/

void SwToxPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_TOX );
        SwTxtPortion::Paint( rInf );
    }
}

/*************************************************************************
 *                      class SwIsoToxPortion
 *************************************************************************/

SwLinePortion *SwIsoToxPortion::Compress() { return this; }

SwIsoToxPortion::SwIsoToxPortion() : nViewWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_ISOTOX );
}

/*************************************************************************
 *               virtual SwIsoToxPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwIsoToxPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Wir stehen zwar im const, aber nViewWidth sollte erst im letzten
    // Moment errechnet werden:
    SwIsoToxPortion* pThis = (SwIsoToxPortion*)this;
    // nViewWidth muss errechnet werden.
    if( !Width() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()   )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize( ' ' ).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwIsoToxPortion::Format()
 *************************************************************************/

sal_Bool SwIsoToxPortion::Format( SwTxtFormatInfo &rInf )
{
    const sal_Bool bFull = SwLinePortion::Format( rInf );
    return bFull;
}

/*************************************************************************
 *               virtual SwIsoToxPortion::Paint()
 *************************************************************************/

void SwIsoToxPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, POR_TOX );
}

/*************************************************************************
 *              virtual SwIsoToxPortion::HandlePortion()
 *************************************************************************/

void SwIsoToxPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

