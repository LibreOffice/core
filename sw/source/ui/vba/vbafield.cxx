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
#include "vbafield.hxx"
#include "vbarange.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <ooo/vba/word/WdFieldType.hpp>
#include <basic/sberrors.hxx>
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>
#include <tools/long.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaField::SwVbaField(  const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const  uno::Reference< css::text::XTextField >& xTextField) : SwVbaField_BASE( rParent, rContext )
{
    mxTextField.set( xTextField, uno::UNO_SET_THROW );
}

sal_Bool SAL_CALL SwVbaField::Update()
{
    uno::Reference< util::XUpdatable > xUpdatable( mxTextField, uno::UNO_QUERY );
    if( xUpdatable.is() )
    {
        xUpdatable->update();
        return true;
    }
    return false;
}

// XHelperInterface
OUString
SwVbaField::getServiceImplName()
{
    return u"SwVbaField"_ustr;
}

uno::Sequence<OUString>
SwVbaField::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Field"_ustr
    };
    return aServiceNames;
}

namespace {

// FIXME? copy and paste code
// the codes are copied from ww8par5.cxx
class SwVbaReadFieldParams
{
private:
    OUString m_aData;
    sal_Int32 m_nLen, m_nFnd, m_nNext, m_nSavPtr;
    OUString m_aFieldName;
public:
    explicit SwVbaReadFieldParams( const OUString& rData );

    tools::Long SkipToNextToken();

    sal_Int32 FindNextStringPiece( sal_Int32 _nStart );

    OUString GetResult() const;
    const OUString& GetFieldName()const { return m_aFieldName; }
};

}

SwVbaReadFieldParams::SwVbaReadFieldParams( const OUString& _rData )
    : m_aData( _rData ), m_nLen( _rData.getLength() ), m_nNext( 0 )
{
    // First search for an opening parenthesis or a space or a quotation mark
    // or a backslash, so that the field command
    // (thus INCLUDEPICTURE or ...) is ignored.
    while( (m_nLen > m_nNext) && (m_aData[ m_nNext ] == ' ') )
        ++m_nNext;

    sal_Unicode c;
    while(     m_nLen > m_nNext
            && (c = m_aData[ m_nNext ]) != ' '
            && c != '"'
            && c != '\\'
            && c != 132
            && c != 0x201c )
        ++m_nNext;

    m_nFnd      = m_nNext;
    m_nSavPtr   = m_nNext;
    m_aFieldName = m_aData.copy( 0, m_nFnd );
}

OUString SwVbaReadFieldParams::GetResult() const
{
    return    (-1 == m_nFnd)
            ? OUString()
            : m_aData.copy( m_nFnd, (m_nSavPtr - m_nFnd) );
}

// ret: -2: NOT a '\' parameter but normal Text
tools::Long SwVbaReadFieldParams::SkipToNextToken()
{
    tools::Long nRet = -1;     // end
    if (
         (-1 != m_nNext) && (m_nLen > m_nNext) &&
         -1 != (m_nFnd = FindNextStringPiece(m_nNext))
       )
    {
        m_nSavPtr = m_nNext;

        if ('\\' == m_aData[m_nFnd] && '\\' != m_aData[m_nFnd + 1])
        {
            nRet = m_aData[++m_nFnd];
            m_nNext = ++m_nFnd;             // and set behind
        }
        else
        {
            nRet = -2;
            if (
                 (-1 != m_nSavPtr ) &&
                 (
                   ('"' == m_aData[m_nSavPtr - 1]) ||
                   (0x201d == m_aData[m_nSavPtr - 1])
                 )
               )
            {
                --m_nSavPtr;
            }
        }
    }
    return nRet;
}

// FindNextPara is searching for the next Backslash-Parameter or the next string
// until blank or the next "\" or until the closing quotation mark
// or until the string end of pStr.

// Output ppNext (if ppNext != 0) beginning of the search for the next parameter or 0

// Return value: 0 if String-End reached, otherwise begin of the parameter or the string

sal_Int32 SwVbaReadFieldParams::FindNextStringPiece(const sal_Int32 nStart)
{
    sal_Int32  n = ( -1 == nStart ) ? m_nFnd : nStart;  // Start
    sal_Int32 n2;          // End

    m_nNext = -1;        // Default for not found

    while( (m_nLen > n) && (m_aData[ n ] == ' ') )
        ++n;

    if( m_nLen == n )
        return -1;     // String End reached!

    if(     (m_aData[ n ] == '"')     // quotation marks are in front of parenthesis?
        ||  (m_aData[ n ] == 0x201c)
        ||  (m_aData[ n ] == 132) )
    {
        n++;                        // ignore quotation marks
        n2 = n;                     // From here search for the end
        while(     (m_nLen > n2)
                && (m_aData[ n2 ] != '"')
                && (m_aData[ n2 ] != 0x201d)
                && (m_aData[ n2 ] != 147) )
            n2++;                   // Search for the end of the parenthesis
    }
    else                        // no quotation marks
    {
        n2 = n;                     // from here search for the end
        while( (m_nLen > n2) && (m_aData[ n2 ] != ' ') ) // Search for the end of the parenthesis
        {
            if( m_aData[ n2 ] == '\\' )
            {
                if( m_aData[ n2+1 ] == '\\' )
                    n2 += 2;        // double-backslash -> OK
                else
                {
                    if( n2 > n )
                        n2--;
                    break;          // single-backslash -> End
                }
            }
            else
                n2++;               // no backslash -> OK
        }
    }
    if( m_nLen > n2 )
    {
        if(m_aData[ n2 ] != ' ') n2++;
        m_nNext = n2;
    }
    return n;
}

// SwVbaFields

static uno::Any lcl_createField( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel, const uno::Any& aSource )
{
    uno::Reference< text::XTextField > xTextField( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< word::XField > xField( new SwVbaField( xParent, xContext, xTextField ) );
    return uno::Any( xField );
}

namespace {

class FieldEnumeration : public ::cppu::WeakImplHelper< css::container::XEnumeration >
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< container::XEnumeration > mxEnumeration;
public:
    FieldEnumeration(  uno::Reference< XHelperInterface >  xParent, uno::Reference< uno::XComponentContext > xContext, uno::Reference< frame::XModel >  xModel, uno::Reference< container::XEnumeration >  xEnumeration ) : mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxModel(std::move( xModel )), mxEnumeration(std::move( xEnumeration ))
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return mxEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return lcl_createField( mxParent, mxContext, mxModel, mxEnumeration->nextElement() );
    }
};

class FieldCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                             container::XEnumerationAccess >
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< container::XEnumerationAccess > mxEnumerationAccess;
public:
    /// @throws css::uno::RuntimeException
    FieldCollectionHelper( uno::Reference< XHelperInterface >  xParent, uno::Reference< uno::XComponentContext > xContext, const uno::Reference< frame::XModel >& xModel ) : mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxModel( xModel )
    {
        uno::Reference< text::XTextFieldsSupplier > xSupp( xModel, uno::UNO_QUERY_THROW );
        mxEnumerationAccess.set( xSupp->getTextFields(), uno::UNO_SET_THROW );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  mxEnumerationAccess->getElementType(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return mxEnumerationAccess->hasElements(); }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        uno::Reference< container::XEnumeration > xEnumeration =  mxEnumerationAccess->createEnumeration();
        sal_Int32 nCount = 0;
        while( xEnumeration->hasMoreElements() )
        {
            ++nCount;
            xEnumeration->nextElement();
        }
        return nCount;
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        uno::Reference< container::XEnumeration > xEnumeration =  mxEnumerationAccess->createEnumeration();
        sal_Int32 nCount = 0;
        while( xEnumeration->hasMoreElements() )
        {
            if( nCount == Index )
            {
                return xEnumeration->nextElement();
            }
            ++nCount;
        }
        throw lang::IndexOutOfBoundsException();
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        uno::Reference< container::XEnumeration > xEnumeration =  mxEnumerationAccess->createEnumeration();
        return uno::Reference< container::XEnumeration >( new FieldEnumeration( mxParent, mxContext, mxModel, xEnumeration ) );
    }
};

}

SwVbaFields::SwVbaFields( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaFields_BASE( xParent, xContext , uno::Reference< container::XIndexAccess >( new FieldCollectionHelper( xParent, xContext, xModel ) ) ), mxModel( xModel )
{
    mxMSF.set( mxModel, uno::UNO_QUERY_THROW );
}

uno::Reference< word::XField > SAL_CALL
SwVbaFields::Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const css::uno::Any& Type, const css::uno::Any& Text, const css::uno::Any& /*PreserveFormatting*/ )
{
    sal_Int32 nType = word::WdFieldType::wdFieldEmpty;
    Type >>= nType;
    OUString sText;
    Text >>= sText;

    OUString sFieldName;
    if( ( nType == word::WdFieldType::wdFieldEmpty ) && !sText.isEmpty() )
    {
        SwVbaReadFieldParams aReadParam(sText);
        sFieldName = aReadParam.GetFieldName();
        SAL_INFO("sw.vba", "the field name is " << sFieldName );
    }

    uno::Reference< text::XTextContent > xTextField;
    if( nType == word::WdFieldType::wdFieldFileName || sFieldName.equalsIgnoreAsciiCase("FILENAME") )
    {
        xTextField.set( Create_Field_FileName( sText ), uno::UNO_QUERY_THROW );
    }
    else if( nType == word::WdFieldType::wdFieldDocProperty || sFieldName.equalsIgnoreAsciiCase("DOCPROPERTY") )
    {
        xTextField.set( Create_Field_DocProperty( sText ), uno::UNO_QUERY_THROW );
    }
    else
    {
        throw uno::RuntimeException(u"Not implemented"_ustr );
    }

    SwVbaRange& rVbaRange = dynamic_cast<SwVbaRange&>(*Range);
    uno::Reference< text::XTextRange > xTextRange = rVbaRange.getXTextRange();
    uno::Reference< text::XText > xText = xTextRange->getText();
    xText->insertTextContent( xTextRange, xTextField, true );
    return uno::Reference< word::XField >( new SwVbaField( mxParent, mxContext, uno::Reference< text::XTextField >( xTextField, uno::UNO_QUERY_THROW ) ) );
}

uno::Reference< text::XTextField > SwVbaFields::Create_Field_FileName( const OUString& _text )
{
    uno::Reference< text::XTextField > xTextField( mxMSF->createInstance(u"com.sun.star.text.TextField.FileName"_ustr), uno::UNO_QUERY_THROW );
    sal_Int16 nFileFormat = text::FilenameDisplayFormat::NAME_AND_EXT;
    if( !_text.isEmpty() )
    {
        tools::Long nRet;
        SwVbaReadFieldParams aReadParam( _text );
        while (-1 != (nRet = aReadParam.SkipToNextToken()))
        {
            switch (nRet)
            {
                case 'p':
                    nFileFormat = text::FilenameDisplayFormat::FULL;
                    break;
                case '*':
                    //Skip over MERGEFORMAT
                    aReadParam.SkipToNextToken();
                    break;
                default:
                    DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, {});
                    break;
            }
        }
    }

    uno::Reference< beans::XPropertySet > xProps( xTextField, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue(u"FileFormat"_ustr, uno::Any( nFileFormat ) );

    return xTextField;
}

namespace {

struct DocPropertyTable
{
    const char* sDocPropertyName;
    const char* sFieldService;
};

}

const DocPropertyTable aDocPropertyTables[] =
{
    { "Author", "com.sun.star.text.textfield.docinfo.CreateAuthor" },
    { "Bytes", nullptr },
    { "Category", nullptr },
    { "Characters",nullptr },
    { "CharactersWithSpaces", nullptr },
    { "Comments", "com.sun.star.text.textfield.docinfo.Description" },
    { "Company", nullptr },
    { "CreateTime", "com.sun.star.text.textfield.docinfo.CreateDateTime" },
    { "HyperlinkBase", nullptr },
    { "Keywords", "com.sun.star.text.textfield.docinfo.Keywords" },
    { "LastPrinted", "com.sun.star.text.textfield.docinfo.PrintDateTime" },
    { "LastSavedBy", "com.sun.star.text.textfield.docinfo.ChangeAuthor" },
    { "LastSavedTime", "com.sun.star.text.textfield.docinfo.ChangeDateTime" },
    { "Lines", nullptr },
    { "Manager", nullptr },
    { "NameofApplication", nullptr },
    { "ODMADocID", nullptr },
    { "Pages", "com.sun.star.text.textfield.PageCount" },
    { "Paragraphs", "com.sun.star.text.textfield.ParagraphCount" },
    { "RevisionNumber", "com.sun.star.text.textfield.docinfo.Revision" },
    { "Security", nullptr },
    { "Subject", "com.sun.star.text.textfield.docinfo.Subject" },
    { "Template", "com.sun.star.text.textfield.TemplateName" },
    { "Title", "com.sun.star.text.textfield.docinfo.Title" },
    { "TotalEditingTime", "com.sun.star.text.textfield.docinfo.EditTime" },
    { "Words", "com.sun.star.text.textfield.WordCount" },
    { nullptr, nullptr }
};

uno::Reference< text::XTextField > SwVbaFields::Create_Field_DocProperty( const OUString& _text )
{
    OUString aDocProperty;
    SwVbaReadFieldParams aReadParam( _text );
    tools::Long nRet;
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
            case -2:
                if( aDocProperty.isEmpty() )
                    aDocProperty = aReadParam.GetResult();
                break;
            case '*':
                //Skip over MERGEFORMAT
                aReadParam.SkipToNextToken();
                break;
        }
    }
    aDocProperty = aDocProperty.replaceAll("\"", "");
    SAL_INFO("sw.vba", "SwVbaFields::Create_Field_DocProperty, the document property name is " << aDocProperty );
    if( aDocProperty.isEmpty() )
    {
        throw uno::RuntimeException();
    }

    bool bCustom = true;
    OUString sFieldService;
    // find the build in document properties
    for( const DocPropertyTable* pTable = aDocPropertyTables; pTable->sDocPropertyName != nullptr; pTable++ )
    {
        if( aDocProperty.equalsIgnoreAsciiCaseAscii( pTable->sDocPropertyName ) )
        {
            if( pTable->sFieldService != nullptr )
                sFieldService = OUString::createFromAscii(pTable->sFieldService);
            bCustom = false;
            break;
        }
    }

    if( bCustom )
    {
        sFieldService = "com.sun.star.text.textfield.docinfo.Custom";
    }
    else if( sFieldService.isEmpty() )
    {
        throw uno::RuntimeException(u"Not implemented"_ustr );
    }

    uno::Reference< text::XTextField > xTextField( mxMSF->createInstance( sFieldService ), uno::UNO_QUERY_THROW );

    if( bCustom )
    {
        uno::Reference< beans::XPropertySet > xProps( xTextField, uno::UNO_QUERY_THROW );
        xProps->setPropertyValue(u"Name"_ustr, uno::Any( aDocProperty ) );
    }

    return xTextField;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaFields::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

// ScVbaCollectionBaseImpl
uno::Any
SwVbaFields::createCollectionObject( const uno::Any& aSource )
{
    return lcl_createField( mxParent, mxContext, mxModel, aSource );
}

sal_Int32 SAL_CALL SwVbaFields::Update()
{
    sal_Int32 nUpdate = 1;
    try
    {
        uno::Reference< text::XTextFieldsSupplier > xSupp( mxModel, uno::UNO_QUERY_THROW );
        uno::Reference< util::XRefreshable > xRef( xSupp->getTextFields(), uno::UNO_QUERY_THROW );
        xRef->refresh();
        nUpdate = 0;
    }
    catch(const uno::Exception&)
    {
        nUpdate = 1;
    }
    return nUpdate;
}

// XHelperInterface
OUString
SwVbaFields::getServiceImplName()
{
    return u"SwVbaFields"_ustr;
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaFields::getElementType()
{
    return  cppu::UnoType<word::XField>::get();
}

uno::Sequence<OUString>
SwVbaFields::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Fields"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
