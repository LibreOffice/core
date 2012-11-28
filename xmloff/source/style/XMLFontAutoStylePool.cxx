/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <o3tl/sorted_vector.hxx>
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

struct XMLFontAutoStylePoolEntryCmp_Impl {
    bool operator()(
        XMLFontAutoStylePoolEntry_Impl* const& r1,
        XMLFontAutoStylePoolEntry_Impl* const& r2 ) const
    {
        sal_Int8 nEnc1(r1->GetEncoding() != RTL_TEXTENCODING_SYMBOL);
        sal_Int8 nEnc2(r2->GetEncoding() != RTL_TEXTENCODING_SYMBOL);
        if( nEnc1 != nEnc2 )
            return nEnc1 < nEnc2;
        else if( r1->GetPitch() != r2->GetPitch() )
            return r1->GetPitch() < r2->GetPitch();
        else if( r1->GetFamily() != r2->GetFamily() )
            return r1->GetFamily() < r2->GetFamily();
        else
        {
            sal_Int32 nCmp = r1->GetFamilyName().compareTo( r2->GetFamilyName() );
            if( 0 == nCmp )
                return r1->GetStyleName().compareTo( r2->GetStyleName() ) < 0;
            else
                return nCmp < 0;
        }
    }
};

class XMLFontAutoStylePool_Impl : public o3tl::sorted_vector<XMLFontAutoStylePoolEntry_Impl*, XMLFontAutoStylePoolEntryCmp_Impl>
{
public:
    ~XMLFontAutoStylePool_Impl() { DeleteAndDestroyAll(); }
};

XMLFontAutoStylePool::XMLFontAutoStylePool( SvXMLExport& rExp ) :
    rExport( rExp ),
    pPool( new XMLFontAutoStylePool_Impl )
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
    XMLFontAutoStylePool_Impl::const_iterator it = pPool->find( &aTmp );
    if( it != pPool->end() )
    {
        sPoolName = (*it)->GetName();
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
            sName = sName.trim();
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
        pPool->insert( pEntry );
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
    XMLFontAutoStylePool_Impl::const_iterator it = pPool->find( &aTmp );
    if( it != pPool->end() )
    {
        sName = (*it)->GetName();
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

    sal_uInt32 nCount = pPool->size();
    for( sal_uInt32 i=0; i<nCount; i++ )
    {
        const XMLFontAutoStylePoolEntry_Impl *pEntry = (*pPool)[ i ];

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
