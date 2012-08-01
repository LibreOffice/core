/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svl/cntnrsrt.hxx>
#include <tools/fontenum.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

class XMLFontAutoStylePoolEntry_Impl
{
    OUString    sName;
    OUString    sFamilyName;
    OUString    sStyleName;
    sal_Int16   nFamily;
    sal_Int16   nPitch;
    rtl_TextEncoding eEnc;

public:

    inline XMLFontAutoStylePoolEntry_Impl(
            const ::rtl::OUString& rName,
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc );

    inline XMLFontAutoStylePoolEntry_Impl(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc );

    const OUString& GetName() const { return sName; }
    const OUString& GetFamilyName() const { return sFamilyName; }
    const OUString& GetStyleName() const { return sStyleName; }
    sal_Int16 GetFamily() const {   return nFamily; }
    sal_Int16 GetPitch() const { return nPitch; }
    rtl_TextEncoding GetEncoding() const { return eEnc; }
};


inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        const ::rtl::OUString& rName,
        const ::rtl::OUString& rFamilyName,
        const ::rtl::OUString& rStyleName,
        sal_Int16 nFam,
        sal_Int16 nP,
        rtl_TextEncoding eE ) :
    sName( rName ),
    sFamilyName( rFamilyName ),
    sStyleName( rStyleName ),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}

inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        const ::rtl::OUString& rFamilyName,
        const ::rtl::OUString& rStyleName,
        sal_Int16 nFam,
        sal_Int16 nP,
        rtl_TextEncoding eE ) :
    sFamilyName( rFamilyName ),
    sStyleName( rStyleName ),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}
int XMLFontAutoStylePoolEntryCmp_Impl(
        const XMLFontAutoStylePoolEntry_Impl& r1,
        const XMLFontAutoStylePoolEntry_Impl& r2 )
{
    sal_Int8 nEnc1(r1.GetEncoding() != RTL_TEXTENCODING_SYMBOL);
    sal_Int8 nEnc2(r2.GetEncoding() != RTL_TEXTENCODING_SYMBOL);
    if( nEnc1 != nEnc2 )
        return nEnc1 - nEnc2;
    else if( r1.GetPitch() != r2.GetPitch() )
        return (int)r1.GetPitch() - (int)r2.GetPitch();
    else if( r1.GetFamily() != r2.GetFamily() )
        return (int)r1.GetFamily() - (int)r2.GetFamily();
    else
    {
        sal_Int32 nCmp = r1.GetFamilyName().compareTo( r2.GetFamilyName() );
        if( 0 == nCmp )
            return (int)r1.GetStyleName().compareTo( r2.GetStyleName() );
        else
            return (int)nCmp;
    }
}

DECLARE_CONTAINER_SORT_DEL( XMLFontAutoStylePool_Impl,
                            XMLFontAutoStylePoolEntry_Impl )
IMPL_CONTAINER_SORT( XMLFontAutoStylePool_Impl,
                     XMLFontAutoStylePoolEntry_Impl,
                     XMLFontAutoStylePoolEntryCmp_Impl )

XMLFontAutoStylePool::XMLFontAutoStylePool( SvXMLExport& rExp ) :
    rExport( rExp ),
    pPool( new XMLFontAutoStylePool_Impl( 5, 5 ) )
{
}

XMLFontAutoStylePool::~XMLFontAutoStylePool()
{
    delete pPool;
}

OUString XMLFontAutoStylePool::Add(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc )
{
    OUString sPoolName;
    XMLFontAutoStylePoolEntry_Impl aTmp( rFamilyName, rStyleName, nFamily,
                                          nPitch, eEnc );
    sal_uLong nPos;
    if( pPool->Seek_Entry( &aTmp, &nPos ) )
    {
        sPoolName = pPool->GetObject( nPos )->GetName();
    }
    else
    {
        OUString sName;
        sal_Int32 nLen = rFamilyName.indexOf( sal_Unicode(';'), 0 );
        if( -1 == nLen )
        {
            sName = rFamilyName;
        }
        else if( nLen > 0 )
        {
            sName = rFamilyName.copy( 0, nLen );
            sName.trim();
        }

        if( sName.isEmpty() )
            sName = OUString::valueOf( sal_Unicode( 'F' ) );

        if( m_aNames.find(sName) != m_aNames.end() )
        {
            sal_Int32 nCount = 1;
            OUString sPrefix( sName );
            sName += OUString::valueOf( nCount );
            while( m_aNames.find(sName) != m_aNames.end() )
            {
                sName = sPrefix;
                sName += OUString::valueOf( ++nCount );
            }
        }

        XMLFontAutoStylePoolEntry_Impl *pEntry =
            new XMLFontAutoStylePoolEntry_Impl( sName, rFamilyName, rStyleName,
                                                nFamily, nPitch, eEnc );
        pPool->Insert( pEntry );
        m_aNames.insert(sName);
    }

    return sPoolName;
}

::rtl::OUString XMLFontAutoStylePool::Find(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc ) const
{
    OUString sName;
    XMLFontAutoStylePoolEntry_Impl aTmp( rFamilyName, rStyleName, nFamily,
                                          nPitch, eEnc );
    sal_uLong nPos;
    if( pPool->Seek_Entry( &aTmp, &nPos ) )
    {
        sName = pPool->GetObject( nPos )->GetName();
    }

    return sName;
}


void XMLFontAutoStylePool::exportXML()
{
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_OFFICE,
                              XML_FONT_FACE_DECLS,
                              sal_True, sal_True );
    Any aAny;
    OUString sTmp;
    XMLFontFamilyNamePropHdl aFamilyNameHdl;
    XMLFontFamilyPropHdl aFamilyHdl;
    XMLFontPitchPropHdl aPitchHdl;
    XMLFontEncodingPropHdl aEncHdl;
    const SvXMLUnitConverter& rUnitConv = GetExport().GetMM100UnitConverter();

    sal_uInt32 nCount = pPool->Count();
    for( sal_uInt32 i=0; i<nCount; i++ )
    {
        const XMLFontAutoStylePoolEntry_Impl *pEntry = pPool->GetObject( i );

        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                  XML_NAME, pEntry->GetName() );

        aAny <<= pEntry->GetFamilyName();
        if( aFamilyNameHdl.exportXML( sTmp, aAny, rUnitConv ) )
            GetExport().AddAttribute( XML_NAMESPACE_SVG,
                                      XML_FONT_FAMILY, sTmp );

        const OUString& rStyleName = pEntry->GetStyleName();
        if( !rStyleName.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_FONT_ADORNMENTS,
                                      rStyleName );

        aAny <<= (sal_Int16)pEntry->GetFamily();
        if( aFamilyHdl.exportXML( sTmp, aAny, rUnitConv  ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_FONT_FAMILY_GENERIC, sTmp );

        aAny <<= (sal_Int16)pEntry->GetPitch();
        if( aPitchHdl.exportXML( sTmp, aAny, rUnitConv  ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_FONT_PITCH, sTmp );

        aAny <<= (sal_Int16)pEntry->GetEncoding();
        if( aEncHdl.exportXML( sTmp, aAny, rUnitConv  ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_FONT_CHARSET, sTmp );

        SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                  XML_FONT_FACE,
                                  sal_True, sal_True );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
