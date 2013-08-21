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
#include <vcl/embeddedfontshelper.hxx>
#include <osl/file.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

class XMLFontAutoStylePoolEntry_Impl
{
    OUString    sName;
    OUString    sFamilyName;
    OUString    sStyleName;
    FontFamily  nFamily;
    FontPitch   nPitch;
    rtl_TextEncoding eEnc;

public:

    inline XMLFontAutoStylePoolEntry_Impl(
            const OUString& rName,
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    inline XMLFontAutoStylePoolEntry_Impl(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    const OUString& GetName() const { return sName; }
    const OUString& GetFamilyName() const { return sFamilyName; }
    const OUString& GetStyleName() const { return sStyleName; }
    FontFamily GetFamily() const {   return nFamily; }
    FontPitch GetPitch() const { return nPitch; }
    rtl_TextEncoding GetEncoding() const { return eEnc; }
};


inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        const OUString& rName,
        const OUString& rFamilyName,
        const OUString& rStyleName,
        FontFamily nFam,
        FontPitch nP,
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
        const OUString& rFamilyName,
        const OUString& rStyleName,
        FontFamily nFam,
        FontPitch nP,
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

XMLFontAutoStylePool::XMLFontAutoStylePool( SvXMLExport& rExp, bool _tryToEmbedFonts ) :
    rExport( rExp ),
    pPool( new XMLFontAutoStylePool_Impl ),
    tryToEmbedFonts( _tryToEmbedFonts )
{
}

XMLFontAutoStylePool::~XMLFontAutoStylePool()
{
    delete pPool;
}

OUString XMLFontAutoStylePool::Add(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
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
        sal_Int32 nLen = rFamilyName.indexOf( ';', 0 );
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
            sName = OUString( sal_Unicode( 'F' ) );

        if( m_aNames.find(sName) != m_aNames.end() )
        {
            sal_Int32 nCount = 1;
            OUString sPrefix( sName );
            sName += OUString::number( nCount );
            while( m_aNames.find(sName) != m_aNames.end() )
            {
                sName = sPrefix;
                sName += OUString::number( ++nCount );
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

OUString XMLFontAutoStylePool::Find(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
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

    std::map< OUString, OUString > fontFilesMap; // our url to document url
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

        if( tryToEmbedFonts )
        {
            std::vector< OUString > fileUrls;
            static const FontWeight weight[] = { WEIGHT_NORMAL, WEIGHT_BOLD, WEIGHT_NORMAL, WEIGHT_BOLD };
            static const FontItalic italic[] = { ITALIC_NONE, ITALIC_NONE, ITALIC_NORMAL, ITALIC_NORMAL };
            assert( SAL_N_ELEMENTS( weight ) == SAL_N_ELEMENTS( italic ));
            for( unsigned int j = 0;
                 j < SAL_N_ELEMENTS( weight );
                 ++j )
            {
                // Embed font if at least viewing is allowed (in which case the opening app must check
                // the font license rights too and open either read-only or not use the font for editing).
                OUString fileUrl = EmbeddedFontsHelper::fontFileUrl( pEntry->GetFamilyName(), pEntry->GetFamily(),
                    italic[ j ], weight[ j ], pEntry->GetPitch(), pEntry->GetEncoding(),
                    EmbeddedFontsHelper::ViewingAllowed );
                if( fileUrl.isEmpty())
                    continue;
                if( !fontFilesMap.count( fileUrl ))
                {
                    OUString docUrl = embedFontFile( fileUrl );
                    if( !docUrl.isEmpty())
                        fontFilesMap[ fileUrl ] = docUrl;
                    else
                        continue; // --> failed to embed
                }
                fileUrls.push_back( fileUrl );
            }
            if( !fileUrls.empty())
            {
                SvXMLElementExport fontFaceSrc( GetExport(), XML_NAMESPACE_SVG,
                    XML_FONT_FACE_SRC, true, true );
                for( std::vector< OUString >::const_iterator it = fileUrls.begin();
                     it != fileUrls.end();
                     ++it )
                {
                    if( fontFilesMap.count( *it ))
                    {
                        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, fontFilesMap[ *it ] );
                        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, "simple" );
                        SvXMLElementExport fontFaceUri( GetExport(), XML_NAMESPACE_SVG,
                            XML_FONT_FACE_URI, true, true );
                    }
                }
            }
        }
    }
}

OUString XMLFontAutoStylePool::embedFontFile( const OUString& fileUrl )
{
    try
    {
        osl::File file( fileUrl );
        if( file.open( osl_File_OpenFlag_Read ) != osl::File::E_None )
            return OUString();
        uno::Reference< embed::XStorage > storage;
        storage.set( GetExport().GetTargetStorage()->openStorageElement( OUString( "Fonts" ),
            ::embed::ElementModes::WRITE ), uno::UNO_QUERY_THROW );
        int index = 0;
        OUString name;
        do
        {
            name = "font" + OUString::number( ++index ) + ".ttf";
        } while( storage->hasByName( name ) );
        uno::Reference< io::XOutputStream > outputStream;
        outputStream.set( storage->openStreamElement( name, ::embed::ElementModes::WRITE ), UNO_QUERY_THROW );
        uno::Reference < beans::XPropertySet > propertySet( outputStream, uno::UNO_QUERY );
        assert( propertySet.is());
        propertySet->setPropertyValue( "MediaType", uno::makeAny( OUString( "application/x-font-ttf" ))); // TODO
        for(;;)
        {
            char buffer[ 4096 ];
            sal_uInt64 readSize;
            sal_Bool eof;
            if( file.isEndOfFile( &eof ) != osl::File::E_None )
            {
                SAL_WARN( "xmloff", "Error reading font file " << fileUrl );
                outputStream->closeOutput();
                return OUString();
            }
            if( eof )
                break;
            if( file.read( buffer, 4096, readSize ) != osl::File::E_None )
            {
                SAL_WARN( "xmloff", "Error reading font file " << fileUrl );
                outputStream->closeOutput();
                return OUString();
            }
            if( readSize == 0 )
                break;
            outputStream->writeBytes( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( buffer ), readSize ));
        }
        outputStream->closeOutput();
        if( storage.is() )
        {
            Reference< embed::XTransactedObject > transaction( storage, UNO_QUERY );
            if( transaction.is())
            {
                transaction->commit();
                return "Fonts/" + name;
            }
        }
    } catch( const Exception& e )
    {
        SAL_WARN( "xmloff", "Exception when embedding a font file:" << e.Message );
    }
    return OUString();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
