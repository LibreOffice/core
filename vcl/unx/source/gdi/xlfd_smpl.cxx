/*************************************************************************
 *
 *  $RCSfile: xlfd_smpl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef XLFD_ATTRIBUTE_HXX
#include "xlfd_attr.hxx"
#endif
#ifndef XLFD_SIMPLE_HXX
#include "xlfd_smpl.hxx"
#endif

// --------------------------------------------------------------------------
//
//
// broken down structure equivalent to a Xlfd string
//
//
// --------------------------------------------------------------------------

Xlfd::Xlfd()
{
}

// XlfdCompare abi has to be qsort(3) compatible, the sorting result must
// guarantee that fonts with SameFontoutline() are successive
// XlfdCompare relies on vFrom->mpFactory eq vTo->mpFactory. Since comparing
// Xlfd's is done by comparing attributes there is no way around this.
extern "C" int
XlfdCompare( const void *vFrom, const void *vTo )
{
    const Xlfd *pFrom = (Xlfd*)vFrom;
    const Xlfd *pTo   = (Xlfd*)vTo;

    // Compare outline description
    if ( pFrom->mnFoundry != pTo->mnFoundry )
        return (int)pFrom->mnFoundry - (int)pTo->mnFoundry;
    if ( pFrom->mnFamily != pTo->mnFamily )
        return (int)pFrom->mnFamily - (int)pTo->mnFamily;
    if ( pFrom->mnWeight != pTo->mnWeight )
        return (int)pFrom->mnWeight - (int)pTo->mnWeight;
    if ( pFrom->mnSlant != pTo->mnSlant )
        return (int)pFrom->mnSlant - (int)pTo->mnSlant;
    if ( pFrom->mnSetwidth != pTo->mnSetwidth )
        return (int)pFrom->mnSetwidth - (int)pTo->mnSetwidth;

    // Addstyle name is futile tricky. it may contain encoding information
    // (like "ansi_1251") which Compares equal, or it may contain style
    // information (like "serif") which Compares unequal
    if ( pFrom->mnAddstyle == pTo->mnAddstyle )
        return 0;

    AttributeProvider *pFactory = pFrom->mpFactory;
    Attribute *pFromAttr = pFactory->RetrieveAddstyle( pFrom->mnAddstyle );
    Attribute *pToAttr   = pFactory->RetrieveAddstyle( pTo->mnAddstyle );

    // if both addstyles denote encodings or if one denotes an
    // encoding and the other denotes a style which really
    // duplicates weight and slant information

    int nFromCompare =     (pFromAttr->GetValue() != RTL_TEXTENCODING_DONTKNOW)
                        || (pFromAttr->HasFeature(XLFD_FEATURE_REDUNDANTSTYLE)) ?
                         -1 : pFrom->mnAddstyle;
    int nToCompare   =     (pToAttr->GetValue()   != RTL_TEXTENCODING_DONTKNOW)
                        || (pToAttr->HasFeature(XLFD_FEATURE_REDUNDANTSTYLE))   ?
                        -1 : pTo->mnAddstyle;

    return nFromCompare - nToCompare;
}

// check whether two fonts are identical as appearance is concerned
// this does not Compare the actual scaling of two fonts
Bool
Xlfd::SameFontoutline( const Xlfd* pComparedTo ) const
{
    void* pThis = (void*)this;
    return XlfdCompare( (void*)pThis, (void*)pComparedTo ) == 0 ;
}

unsigned short
Xlfd::GetEncoding() const
{
    Attribute *pAddstyle = mpFactory->RetrieveAddstyle( mnAddstyle );
    if ( pAddstyle->GetValue() != RTL_TEXTENCODING_DONTKNOW )
        return pAddstyle->GetValue();

    Attribute *pEncoding = mpFactory->RetrieveCharset( mnCharset );
    return pEncoding->GetValue();
}

XlfdFonttype
Xlfd::Fonttype() const
{
    if ( (mnAverageWidth == 0) && (mnPixelSize == 0) && (mnPointSize == 0) )
    {
        if ( (mnResolutionX == 0) && (mnResolutionY == 0) )
            return mpFactory->GetDevice() == eDevicePrinter ?
                            eTypePrinterDownload : eTypeScalable;
        else
            return mpFactory->GetDevice() == eDevicePrinter ?
                            eTypePrinterBuiltIn : eTypeScalableBitmap;
    }

    return mpFactory->GetDevice() == eDevicePrinter ?
                    eTypeUnknown : eTypeBitmap;
}

void
Advance( const char** pFrom, const char** pTo )
{
    const char *pTmp = *pTo;

    for( ; (*pTmp != '\0') && (*pTmp++ != '-'); )
        {}
    *pFrom = *pTo;
    *pTo   = pTmp;
}

// this is the real workhorse function. Since this is called for every font
// in the fontpath it has to be as fast a possible
Bool
Xlfd::FromString( const char* pXlfdstring, AttributeProvider *pFactory )
{
    mpFactory = pFactory;

    const char*     pTo;
    const char*     pFrom;

    pFrom = pXlfdstring;
    // first char must be '-'
    if(*pFrom++ != '-')
        return False;

    pTo = pFrom;

    Advance( &pFrom, &pTo ); //-foundry-*
    mnFoundry = mpFactory->InsertFoundry( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); // -*-family-*
    mnFamily = mpFactory->InsertFamily( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); // -*-*-weight-*
    mnWeight = mpFactory->InsertWeight( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); //-*-*-*-slant-*
    mnSlant = mpFactory->InsertSlant( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); //-*-*-*-*-setwidth-*
    mnSetwidth = mpFactory->InsertSetwidth( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-Addstyle-*
    mnAddstyle = mpFactory->InsertAddstyle( pFrom, pTo - pFrom - 1 );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-height-*
    mnPixelSize = atoi( pFrom );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-pt height-*
    mnPointSize = atoi( pFrom );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-*-x resolution-*
    mnResolutionX = atoi( pFrom );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-*-*-y resolution-*
    mnResolutionY = atoi( pFrom );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-*-*-*-spacing-*
    mcSpacing = pFrom == pTo ? '\0' : *pFrom;

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-*-*-*-*-average-*
    mnAverageWidth = atoi( pFrom );

    Advance( &pFrom, &pTo ); //-*-*-*-*-*-*-*-*-*-*-*-*-registry-encoding
    const char* pTmp = pFrom;
    Advance( &pTmp, &pTo );
    mnCharset = mpFactory->InsertCharset( pFrom, pTo - pFrom );

    // sanity check whether we have really found a valid XLFD, if not
    // throw away the whole font, since we have no idea what parts of
    // the XLFD contains the error.
    if ( !(pTo > pFrom) )
        return False;

     // a non-empty family name is essential, since otherwise the font
    // would match the "default font" #52299#
    Attribute*  pFamily     = mpFactory->RetrieveFamily( mnFamily );
    const char* pFamilyName = pFamily->GetName();
    if ( pFamilyName[0] == '\0' )
        return False;

    // well done
    return True;
}

#ifdef DEBUG
// pure debug for now: this is only to inspect/pretty print a Xlfd struct
const char*
Xlfd::ToString( ByteString &rString ) const
{
    AppendAttribute( mpFactory->RetrieveFoundry(mnFoundry),   rString );
    AppendAttribute( mpFactory->RetrieveFamily(mnFamily),     rString );
    AppendAttribute( mpFactory->RetrieveWeight(mnWeight),     rString );
    AppendAttribute( mpFactory->RetrieveSlant(mnSlant),       rString );
    AppendAttribute( mpFactory->RetrieveSetwidth(mnSetwidth), rString );
    AppendAttribute( mpFactory->RetrieveAddstyle(mnAddstyle), rString );

    rString += '-'; rString += ByteString::CreateFromInt32( mnPixelSize );
    rString += '-'; rString += ByteString::CreateFromInt32( mnPointSize );
    rString += '-'; rString += ByteString::CreateFromInt32( mnResolutionX );
    rString += '-'; rString += ByteString::CreateFromInt32( mnResolutionY );
    rString += '-'; rString += mcSpacing;
    rString += '-'; rString += ByteString::CreateFromInt32( mnAverageWidth );

    AppendAttribute( mpFactory->RetrieveCharset(mnCharset), rString );

    return rString.GetBuffer() ;
}

void
Xlfd::Dump() const
{
    ByteString aString;
    fprintf(stderr, "Xlfd: %s\n", ToString(aString) );
}
#endif

