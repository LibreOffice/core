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
#include "porref.hxx"
#include "inftxt.hxx"       // GetTxtSize()

/*************************************************************************
 *               virtual SwRefPortion::Paint()
 *************************************************************************/

void SwRefPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_REF );
        SwTxtPortion::Paint( rInf );
    }
}

/*************************************************************************
 *                      class SwIsoRefPortion
 *************************************************************************/

SwLinePortion *SwIsoRefPortion::Compress() { return this; }

SwIsoRefPortion::SwIsoRefPortion() : nViewWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_ISOREF );
}

/*************************************************************************
 *               virtual SwIsoRefPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwIsoRefPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Wir stehen zwar im const, aber nViewWidth sollte erst im letzten
    // Moment errechnet werden:
    SwIsoRefPortion* pThis = (SwIsoRefPortion*)this;
    if( !Width() && rInf.OnWin() && SwViewOption::IsFieldShadings() &&
            !rInf.GetOpt().IsReadonly() && !rInf.GetOpt().IsPagePreview() )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize( ' ' ).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwIsoRefPortion::Format()
 *************************************************************************/

sal_Bool SwIsoRefPortion::Format( SwTxtFormatInfo &rInf )
{
    const sal_Bool bFull = SwLinePortion::Format( rInf );
    return bFull;
}

/*************************************************************************
 *               virtual SwIsoRefPortion::Paint()
 *************************************************************************/

void SwIsoRefPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, POR_REF );
}

/*************************************************************************
 *              virtual SwIsoRefPortion::HandlePortion()
 *************************************************************************/

void SwIsoRefPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}



