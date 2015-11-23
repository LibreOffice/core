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
#include "vbastyles.hxx"
#include "vbastyle.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <ooo/vba/word/WdBuiltinStyle.hpp>
#include <ooo/vba/word/WdStyleType.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

struct BuiltinStyleTable
{
    sal_Int32 wdBuiltinStyle;
    const sal_Char* pOOoStyleName;
    sal_Int32 wdStyleType;
};

static const BuiltinStyleTable aBuiltinStyleTable[] =
{
    { word::WdBuiltinStyle::wdStyleBlockQuotation, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyText, "Text body", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyText2, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyText3, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyTextFirstIndent, "First line indent", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyTextFirstIndent2, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyTextIndent, "Text body indent", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyTextIndent2, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleBodyTextIndent3, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleCaption, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleClosing, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleCommentReference, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleCommentText, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleDate, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleDefaultParagraphFont, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleEmphasis, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleEndnoteReference, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleEndnoteText, "Endnote", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleEnvelopeAddress, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleEnvelopeReturn, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleFooter, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleFootnoteReference, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleFootnoteText, "Footnote", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeader, "Header", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading1, "Heading 1", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading2, "Heading 2", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading3, "Heading 3", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading4, "Heading 4", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading5, "Heading 5", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading6, "Heading 6", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading7, "Heading 7", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading8, "Heading 8", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHeading9, "Heading 9", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlAcronym, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlAddress, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlCite, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlCode, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlDfn, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlKbd, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlNormal, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlPre, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlSamp, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlTt, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHtmlVar, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHyperlink, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleHyperlinkFollowed, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex1, "Index 1", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex2, "Index 2", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex3, "Index 3", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex4, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex5, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex6, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex7, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex8, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndex9, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleIndexHeading, "Index Heading", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleLineNumber, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleList, "List", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleList2, "List 2", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleList3, "List 3", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleList4, "List 4", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleList5, "List 5", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListBullet, "List 1", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListBullet2, "List 2", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListBullet3, "List 3", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListBullet4, "List 4", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListBullet5, "List 5", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListContinue, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListContinue2, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListContinue3, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListContinue4, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListContinue5, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleListNumber, "Numbering 1", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListNumber2, "Numbering 2", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListNumber3, "Numbering 3", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListNumber4, "Numbering 4", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleListNumber5, "Numbering 5", word::WdStyleType::wdStyleTypeList },
    { word::WdBuiltinStyle::wdStyleMacroText, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleMessageHeader, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleNavPane, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleNormal, "Default", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleNormalIndent, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleNormalTable, "Table", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleNoteHeading, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStylePageNumber, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStylePlainText, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleSalutation, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleSignature, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleStrong, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleSubtitle, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTableOfAuthorities, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTableOfFigures, "", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTitle, "Title", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOAHeading, "Contents Heading", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC1, "Contents 1", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC2, "Contents 2", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC3, "Contents 3", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC4, "Contents 4", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC5, "Contents 5", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC6, "Contents 6", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC7, "Contents 7", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC8, "Contents 8", word::WdStyleType::wdStyleTypeParagraph },
    { word::WdBuiltinStyle::wdStyleTOC9, "Contents 9", word::WdStyleType::wdStyleTypeParagraph },
    { 0, nullptr, 0 }
};

struct MSOStyleNameTable
{
    const sal_Char* pMSOStyleName;
    const sal_Char* pOOoStyleName;
};

static const MSOStyleNameTable aMSOStyleNameTable[] =
{
    { "Normal", "Default" },
    { nullptr, nullptr }
};

class StyleCollectionHelper : public ::cppu::WeakImplHelper< container::XNameAccess,
                                                             container::XIndexAccess,
                                                             container::XEnumerationAccess >
{
private:
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< container::XNameAccess > mxParaStyles;
    uno::Any cachePos;
public:
    explicit StyleCollectionHelper( const uno::Reference< frame::XModel >& _xModel ) : mxModel( _xModel )
    {
        // we only concern about the Paragraph styles
        uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( _xModel, uno::UNO_QUERY_THROW);
        uno::Reference< container::XNameAccess > xStyleFamilies = xStyleSupplier->getStyleFamilies();
        mxParaStyles.set( xStyleFamilies->getByName("ParagraphStyles"), uno::UNO_QUERY_THROW  );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override { return  cppu::UnoType<style::XStyle>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override { return getCount() > 0; }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return cachePos;
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException, std::exception) override
    {
        return mxParaStyles->getElementNames();
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException, std::exception) override
    {
        // search in the MSOStyleName table first
        for( const MSOStyleNameTable* pTable = aMSOStyleNameTable; pTable->pMSOStyleName != nullptr; pTable++ )
        {
            if( aName.equalsIgnoreAsciiCaseAscii( pTable->pMSOStyleName ) )
            {
                //Found it
                OUString sStyleName = OUString::createFromAscii( pTable->pOOoStyleName );
                if( mxParaStyles->hasByName( sStyleName ) )
                {
                    cachePos = mxParaStyles->getByName( sStyleName );
                    return sal_True;
                }
                return sal_False;
            }
        }

        if( mxParaStyles->hasByName( aName ) )
        {
            cachePos = mxParaStyles->getByName( aName );
            return sal_True;
        }
        else
        {
            uno::Sequence< OUString > sElementNames = mxParaStyles->getElementNames();
            for( sal_Int32 j = 0; j < sElementNames.getLength(); j++ )
            {
                OUString aStyleName = sElementNames[j];
                if( aStyleName.equalsIgnoreAsciiCase( aName ) )
                {
                    cachePos = mxParaStyles->getByName( aStyleName );
                    return sal_True;
                }
            }
        }
        return sal_False;
    }

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( mxParaStyles, uno::UNO_QUERY_THROW );
        return xIndexAccess->getCount();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        uno::Reference< container::XIndexAccess > xIndexAccess( mxParaStyles, uno::UNO_QUERY_THROW );
        return xIndexAccess->getByIndex( Index );
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException, std::exception) override
    {
        throw uno::RuntimeException("Not implemented" );
    }
};

class StylesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaStyles* pStyles;
    sal_Int32 nIndex;
public:
    explicit StylesEnumWrapper( SwVbaStyles* _pStyles ) : pStyles( _pStyles ), nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex <= pStyles->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( nIndex <= pStyles->getCount() )
            return pStyles->Item( uno::makeAny( nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaStyles::SwVbaStyles( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel )
    throw ( script::BasicErrorException, uno::RuntimeException )
    : SwVbaStyles_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new StyleCollectionHelper( xModel )  ) ), mxModel( xModel )
{
    mxMSF.set( mxModel, uno::UNO_QUERY_THROW );
}

uno::Any
SwVbaStyles::createCollectionObject(const uno::Any& aObject)
{
    uno::Reference< beans::XPropertySet > xStyleProp( aObject, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XStyle >( new SwVbaStyle( this, mxContext, mxModel, xStyleProp ) ) );
}

uno::Type SAL_CALL
SwVbaStyles::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XStyle>::get();
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaStyles::createEnumeration() throw (uno::RuntimeException)
{
    return new StylesEnumWrapper( this );
}

uno::Any SAL_CALL
SwVbaStyles::Item( const uno::Any& Index1, const uno::Any& Index2 )
    throw (lang::IndexOutOfBoundsException, script::BasicErrorException, uno::RuntimeException)
{
    //handle WdBuiltinStyle
    sal_Int32 nIndex = 0;
    if( ( Index1 >>= nIndex ) && ( nIndex < 0 ) )
    {
        for( const BuiltinStyleTable* pTable = aBuiltinStyleTable; pTable != nullptr; pTable++ )
        {
            if( nIndex == pTable->wdBuiltinStyle )
            {
                OUString aStyleName = OUString::createFromAscii( pTable->pOOoStyleName );
                if( !aStyleName.isEmpty() )
                {
                    OUString aStyleType;
                    switch( pTable->wdStyleType )
                    {
                        case word::WdStyleType::wdStyleTypeParagraph:
                        case word::WdStyleType::wdStyleTypeTable:
                        {
                            aStyleType = "ParagraphStyles";
                            break;
                        }
                        case word::WdStyleType::wdStyleTypeCharacter:
                        {
                            aStyleType = "CharacterStyles";
                            break;
                        }
                        case word::WdStyleType::wdStyleTypeList:
                        {
                            // should use Paragraph style and set the property "NumberingStyleName"
                            aStyleType = "ParagraphStyles";
                            break;
                        }
                        default:
                            DebugHelper::basicexception( ERRCODE_BASIC_INTERNAL_ERROR, OUString() );
                    }
                    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxModel, uno::UNO_QUERY_THROW);
                    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( aStyleType ), uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xStyleProps( xStylesAccess->getByName( aStyleName ), uno::UNO_QUERY_THROW );
                    // set the property "NumberingStyleName" if it is a listbullet
                    if( pTable->wdStyleType == word::WdStyleType::wdStyleTypeList )
                    {
                        xStyleProps->setPropertyValue("NumberingStyleName", uno::makeAny( aStyleName ) );
                    }
                    return uno::makeAny( uno::Reference< word::XStyle >( new SwVbaStyle( this, mxContext, mxModel, xStyleProps ) ) );
                }
                else
                {
                    OSL_TRACE("SwVbaStyles::Item: the builtin style type is not implemented");
                    throw uno::RuntimeException("Not implemented" );
                }
            }
        }
    }
    return SwVbaStyles_BASE::Item( Index1, Index2 );
}

OUString
SwVbaStyles::getServiceImplName()
{
    return OUString("SwVbaStyles");
}

uno::Sequence< OUString >
SwVbaStyles::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.XStyles";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
