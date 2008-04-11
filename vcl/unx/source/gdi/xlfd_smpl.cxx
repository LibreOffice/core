/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlfd_smpl.cxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xlfd_attr.hxx"
#include "xlfd_smpl.hxx"

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
    // information (like "serif") which Compares unequal, anyway if the font
    // is "interface user" or "interface system" then compare equal anyway to
    // build fontsets as large as possible
    if ( pFrom->mnAddstyle == pTo->mnAddstyle )
        return 0;

    AttributeProvider *pFactory = pFrom->mpFactory;
    Attribute *pFamily = pFactory->RetrieveFamily( pFrom->mnFamily );
    if ( pFamily->HasFeature(XLFD_FEATURE_APPLICATION_FONT) )
        return 0;

    Attribute *pFromAddStyle = pFactory->RetrieveAddstyle( pFrom->mnAddstyle );
    Attribute *pToAddStyle   = pFactory->RetrieveAddstyle( pTo->mnAddstyle );

    // if both addstyles denote encodings or if one denotes an
    // encoding and the other denotes a style which really
    // duplicates weight and slant information

    int nFromCompare =     (pFromAddStyle->GetValue() != RTL_TEXTENCODING_DONTKNOW)
                        || (pFromAddStyle->HasFeature(XLFD_FEATURE_REDUNDANTSTYLE)) ?
                         -1 : pFrom->mnAddstyle;
    int nToCompare   =     (pToAddStyle->GetValue()   != RTL_TEXTENCODING_DONTKNOW)
                        || (pToAddStyle->HasFeature(XLFD_FEATURE_REDUNDANTSTYLE))   ?
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
        return     (mnResolutionX == 0)
                && (mnResolutionY == 0) ? eTypeScalable : eTypeScalableBitmap;
    }

    return eTypeBitmap;
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

Bool
Xlfd::IsConformant (const char* pXlfd) const
{
    // X FontNameRegistry prefix "-"
    if (*pXlfd++ != '-')
        return False;

    // All Xlfd FontName fields are defined
    int nNumFields = 1;
    while (*pXlfd != '\0')
    {
        if (*pXlfd++ == '-')
            nNumFields++;
    }
    // enough entries ?
    if (nNumFields != 14)
        return False;
    // and the last one is not empty as well ?
    if (*(pXlfd - 1) == '-')
        return False;

    return True;
}

// this is the real workhorse function. Since this is called for every font
// in the fontpath it has to be as fast a possible
Bool
Xlfd::FromString( const char* pXlfdstring, AttributeProvider *pFactory )
{
    if (!IsConformant(pXlfdstring))
        return False;

    const char* pFrom = pXlfdstring + 1;
    const char* pTo   = pFrom;
    mpFactory         = pFactory;

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

#if OSL_DEBUG_LEVEL > 1
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

    rString.Append("-"); rString.Append( ByteString::CreateFromInt32( mnPixelSize ) );
    rString.Append("-"); rString.Append( ByteString::CreateFromInt32( mnPointSize ) );
    rString.Append("-"); rString.Append( ByteString::CreateFromInt32( mnResolutionX ) );
    rString.Append("-"); rString.Append( ByteString::CreateFromInt32( mnResolutionY ) );
    rString.Append("-"); rString.Append( (char)mcSpacing );
    rString.Append("-"); rString.Append( ByteString::CreateFromInt32( mnAverageWidth ) );

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

