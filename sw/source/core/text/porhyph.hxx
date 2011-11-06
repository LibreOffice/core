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


#ifndef _PORHYPH_HXX
#define _PORHYPH_HXX

#include "porexp.hxx"

/*************************************************************************
 *                      class SwHyphPortion
 *************************************************************************/

class SwHyphPortion : public SwExpandPortion
{
public:
    inline  SwHyphPortion( ) { SetWhichPor( POR_HYPH ); }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHyphStrPortion
 *************************************************************************/

class SwHyphStrPortion : public SwHyphPortion
{
    XubString aExpand;
public:
    inline  SwHyphStrPortion( const XubString &rStr );
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

class SwSoftHyphPortion : public SwHyphPortion
{
    sal_Bool    bExpand;
    KSHORT  nViewWidth;
    KSHORT  nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    inline void SetExpand( const sal_Bool bNew ) { bExpand = bNew; }
    sal_Bool IsExpand() const { return bExpand; }

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphStrPortion
 *************************************************************************/

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    SwSoftHyphStrPortion( const XubString &rStr );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

inline  SwHyphStrPortion::SwHyphStrPortion( const XubString &rStr )
        : aExpand( rStr )
{
    aExpand += '-';
    SetWhichPor( POR_HYPHSTR );
}

CLASSIO( SwHyphPortion )
CLASSIO( SwHyphStrPortion )
CLASSIO( SwSoftHyphPortion )
CLASSIO( SwSoftHyphStrPortion )


#endif
