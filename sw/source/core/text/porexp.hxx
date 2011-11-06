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



#ifndef _POREXP_HXX
#define _POREXP_HXX

#include "portxt.hxx"

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

class SwExpandPortion : public SwTxtPortion
{
public:
    inline  SwExpandPortion() { SetWhichPor( POR_EXP ); }
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};


/*************************************************************************
 *                      class SwBlankPortion
 *************************************************************************/

class SwBlankPortion : public SwExpandPortion
{
    xub_Unicode cChar;
    sal_Bool bMulti;        // For multiportion brackets
public:
    inline  SwBlankPortion( xub_Unicode cCh, sal_Bool bMult = sal_False )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(1); SetWhichPor( POR_BLANK ); }

    sal_Bool IsMulti() const { return bMulti; }
    void SetMulti( sal_Bool bNew ) { bMulti = bNew; }

    virtual SwLinePortion *Compress();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    MSHORT MayUnderFlow( const SwTxtFormatInfo &rInf, xub_StrLen nIdx,
        sal_Bool bUnderFlow ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/

class SwPostItsPortion : public SwExpandPortion
{
    KSHORT  nViewWidth;
    sal_Bool    bScript;
public:
            SwPostItsPortion( sal_Bool bScrpt );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    sal_Bool IsScript() const { return bScript; }
    OUTPUT_OPERATOR
};


CLASSIO( SwExpandPortion )
CLASSIO( SwBlankPortion )
CLASSIO( SwPostItsPortion )


#endif
