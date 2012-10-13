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
#include "vbafield.hxx"
#include "vbarange.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <comphelper/string.hxx>
#include <ooo/vba/word/WdFieldType.hpp>
#include <swtypes.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// *** SwVbaField ***********************************************

SwVbaField::SwVbaField(  const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rDocument, const  uno::Reference< css::text::XTextField >& xTextField) throw ( uno::RuntimeException ) : SwVbaField_BASE( rParent, rContext ), mxTextDocument( rDocument )
{
    mxTextField.set( xTextField, uno::UNO_QUERY_THROW );
}

sal_Bool SAL_CALL SwVbaField::Update() throw (uno::RuntimeException)
{
    uno::Reference< util::XUpdatable > xUpdatable( mxTextField, uno::UNO_QUERY );
    if( xUpdatable.is() )
    {
        xUpdatable->update();
        return sal_True;
    }
    return sal_False;
}

// XHelperInterface
rtl::OUString
SwVbaField::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaField"));
}

uno::Sequence<rtl::OUString>
SwVbaField::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Field" ) );
    }
    return aServiceNames;
}

// *** _ReadFieldParams ***********************************************
// the codes are copied from ww8par5.cxx
class _ReadFieldParams
{
private:
    String aData;
    xub_StrLen nLen, nFnd, nNext, nSavPtr;
    String aFieldName;
public:
    _ReadFieldParams( const String& rData );
    ~_ReadFieldParams();

    xub_StrLen GoToTokenParam();
    long SkipToNextToken();
    xub_StrLen GetTokenSttPtr() const   { return nFnd;  }

    xub_StrLen FindNextStringPiece( xub_StrLen _nStart = STRING_NOTFOUND );
    bool GetTokenSttFromTo(xub_StrLen* _pFrom, xub_StrLen* _pTo,
        xub_StrLen _nMax);

    String GetResult() const;
    String GetFieldName()const { return aFieldName; }
};


_ReadFieldParams::_ReadFieldParams( const String& _rData )
    : aData( _rData ), nLen( _rData.Len() ), nNext( 0 )
{
    /*
        erstmal nach einer oeffnenden Klammer oder einer Leerstelle oder einem
        Anfuehrungszeichen oder einem Backslash suchen, damit der Feldbefehl
        (also INCLUDEPICTURE bzw EINFUeGENGRAFIK bzw ...) ueberlesen wird
    */
    while( (nLen > nNext) && (aData.GetChar( nNext ) == ' ') )
        ++nNext;

    sal_Unicode c;
    while(     nLen > nNext
            && (c = aData.GetChar( nNext )) != ' '
            && c != '"'
            && c != '\\'
            && c != 132
            && c != 0x201c )
        ++nNext;

    nFnd      = nNext;
    nSavPtr   = nNext;
    aFieldName = aData.Copy( 0, nFnd );
}


_ReadFieldParams::~_ReadFieldParams()
{
}


String _ReadFieldParams::GetResult() const
{
    return    (STRING_NOTFOUND == nFnd)
            ? aEmptyStr
            : aData.Copy( nFnd, (nSavPtr - nFnd) );
}


xub_StrLen _ReadFieldParams::GoToTokenParam()
{
    xub_StrLen nOld = nNext;
    if( -2 == SkipToNextToken() )
        return GetTokenSttPtr();
    nNext = nOld;
    return STRING_NOTFOUND;
}

// ret: -2: NOT a '\' parameter but normal Text
long _ReadFieldParams::SkipToNextToken()
{
    long nRet = -1;     // Ende
    if (
         (STRING_NOTFOUND != nNext) && (nLen > nNext) &&
         STRING_NOTFOUND != (nFnd = FindNextStringPiece(nNext))
       )
    {
        nSavPtr = nNext;

        if ('\\' == aData.GetChar(nFnd) && '\\' != aData.GetChar(nFnd + 1))
        {
            nRet = aData.GetChar(++nFnd);
            nNext = ++nFnd;             // und dahinter setzen
        }
        else
        {
            nRet = -2;
            if (
                 (STRING_NOTFOUND != nSavPtr ) &&
                 (
                   ('"' == aData.GetChar(nSavPtr - 1)) ||
                   (0x201d == aData.GetChar(nSavPtr - 1))
                 )
               )
            {
                --nSavPtr;
            }
        }
    }
    return nRet;
}

// FindNextPara sucht naechsten Backslash-Parameter oder naechste Zeichenkette
// bis zum Blank oder naechsten "\" oder zum schliessenden Anfuehrungszeichen
// oder zum String-Ende von pStr.
//
// Ausgabe ppNext (falls ppNext != 0) Suchbeginn fuer naechsten Parameter bzw. 0
//
// Returnwert: 0 falls String-Ende erreicht,
//             ansonsten Anfang des Paramters bzw. der Zeichenkette
//
xub_StrLen _ReadFieldParams::FindNextStringPiece(const xub_StrLen nStart)
{
    xub_StrLen  n = ( STRING_NOTFOUND == nStart ) ? nFnd : nStart;  // Anfang
    xub_StrLen n2;          // Ende

    nNext = STRING_NOTFOUND;        // Default fuer nicht gefunden

    while( (nLen > n) && (aData.GetChar( n ) == ' ') )
        ++n;

    if( nLen == n )
        return STRING_NOTFOUND;     // String End reached!

    if(     (aData.GetChar( n ) == '"')     // Anfuehrungszeichen vor Para?
        ||  (aData.GetChar( n ) == 0x201c)
        ||  (aData.GetChar( n ) == 132) )
    {
        n++;                        // Anfuehrungszeichen ueberlesen
        n2 = n;                     // ab hier nach Ende suchen
        while(     (nLen > n2)
                && (aData.GetChar( n2 ) != '"')
                && (aData.GetChar( n2 ) != 0x201d)
                && (aData.GetChar( n2 ) != 147) )
            n2++;                   // Ende d. Paras suchen
    }
    else                        // keine Anfuehrungszeichen
    {
        n2 = n;                     // ab hier nach Ende suchen
        while( (nLen > n2) && (aData.GetChar( n2 ) != ' ') ) // Ende d. Paras suchen
        {
            if( aData.GetChar( n2 ) == '\\' )
            {
                if( aData.GetChar( n2+1 ) == '\\' )
                    n2 += 2;        // Doppel-Backslash -> OK
                else
                {
                    if( n2 > n )
                        n2--;
                    break;          // einfach-Backslash -> Ende
                }
            }
            else
                n2++;               // kein Backslash -> OK
        }
    }
    if( nLen > n2 )
    {
        if(aData.GetChar( n2 ) != ' ') n2++;
        nNext = n2;
    }
    return n;
}



// read parameters "1-3" or 1-3 with both values between 1 and nMax
bool _ReadFieldParams::GetTokenSttFromTo(sal_uInt16* pFrom, sal_uInt16* pTo, sal_uInt16 nMax)
{
    sal_uInt16 nStart = 0;
    sal_uInt16 nEnd   = 0;
    xub_StrLen n = GoToTokenParam();
    if( STRING_NOTFOUND != n )
    {

        String sParams( GetResult() );

        xub_StrLen nIndex = 0;
        String sStart( sParams.GetToken(0, '-', nIndex) );
        if( STRING_NOTFOUND != nIndex )
        {
            nStart = static_cast<sal_uInt16>(sStart.ToInt32());
            nEnd   = static_cast<sal_uInt16>(sParams.Copy(nIndex).ToInt32());
        }
    }
    if( pFrom ) *pFrom = nStart;
    if( pTo )   *pTo   = nEnd;

    return nStart && nEnd && (nMax >= nStart) && (nMax >= nEnd);
}

// *** SwVbaFields ***********************************************

static uno::Any lcl_createField( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel, const uno::Any& aSource )
{
    uno::Reference< text::XTextField > xTextField( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< word::XField > xField( new SwVbaField( xParent, xContext, xTextDocument, xTextField ) );
    return uno::makeAny( xField );
}

typedef ::cppu::WeakImplHelper1< css::container::XEnumeration > FieldEnumeration_BASE;
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > FieldCollectionHelper_BASE;

class FieldEnumeration : public FieldEnumeration_BASE
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< container::XEnumeration > mxEnumeration;
public:
    FieldEnumeration(  const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< container::XEnumeration >& xEnumeration ) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel ), mxEnumeration( xEnumeration )
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return mxEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return lcl_createField( mxParent, mxContext, mxModel, mxEnumeration->nextElement() );
    }
};

class FieldCollectionHelper : public FieldCollectionHelper_BASE
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< container::XEnumerationAccess > mxEnumerationAccess;
public:
    FieldCollectionHelper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) throw (css::uno::RuntimeException) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel )
    {
        uno::Reference< text::XTextFieldsSupplier > xSupp( xModel, uno::UNO_QUERY_THROW );
        mxEnumerationAccess.set( xSupp->getTextFields(), uno::UNO_QUERY_THROW );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  mxEnumerationAccess->getElementType(); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return mxEnumerationAccess->hasElements(); }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
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
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
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
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        uno::Reference< container::XEnumeration > xEnumeration =  mxEnumerationAccess->createEnumeration();
        return uno::Reference< container::XEnumeration >( new FieldEnumeration( mxParent, mxContext, mxModel, xEnumeration ) );
    }
};

SwVbaFields::SwVbaFields( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaFields_BASE( xParent, xContext , uno::Reference< container::XIndexAccess >( new FieldCollectionHelper( xParent, xContext, xModel ) ) ), mxModel( xModel )
{
    mxMSF.set( mxModel, uno::UNO_QUERY_THROW );
}

uno::Reference< word::XField > SAL_CALL
SwVbaFields::Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const css::uno::Any& Type, const css::uno::Any& Text, const css::uno::Any& /*PreserveFormatting*/ ) throw (css::uno::RuntimeException)
{
    sal_Int32 nType = word::WdFieldType::wdFieldEmpty;
    Type >>= nType;
    rtl::OUString sText;
    Text >>= sText;

    String sFieldName;
    if( ( nType == word::WdFieldType::wdFieldEmpty ) && !sText.isEmpty() )
    {
        _ReadFieldParams aReadParam(sText);
        sFieldName = aReadParam.GetFieldName();
        OSL_TRACE("SwVbaFields::Add, the field name is %s ",rtl::OUStringToOString( sFieldName, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    uno::Reference< text::XTextContent > xTextField;
    if( nType == word::WdFieldType::wdFieldFileName || sFieldName.EqualsIgnoreCaseAscii("FILENAME") )
    {
        xTextField.set( Create_Field_FileName( sText ), uno::UNO_QUERY_THROW );
    }
    else if( nType == word::WdFieldType::wdFieldDocProperty || sFieldName.EqualsIgnoreCaseAscii("DOCPROPERTY") )
    {
        xTextField.set( Create_Field_DocProperty( sText ), uno::UNO_QUERY_THROW );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }

    SwVbaRange* pVbaRange = dynamic_cast< SwVbaRange* >( Range.get() );
    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();
    uno::Reference< text::XText > xText = xTextRange->getText();
    xText->insertTextContent( xTextRange, xTextField, true );
    return uno::Reference< word::XField >( new SwVbaField( mxParent, mxContext, uno::Reference< text::XTextDocument >( mxModel, uno::UNO_QUERY_THROW ), uno::Reference< text::XTextField >( xTextField, uno::UNO_QUERY_THROW ) ) );
}

uno::Reference< text::XTextField > SwVbaFields::Create_Field_FileName( const rtl::OUString _text ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextField > xTextField( mxMSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField.FileName")) ), uno::UNO_QUERY_THROW );
    sal_Int16 nFileFormat = text::FilenameDisplayFormat::NAME_AND_EXT;
    if( !_text.isEmpty() )
    {
        long nRet;
        _ReadFieldParams aReadParam( _text );
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
                    DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
                    break;
            }
        }
    }

    uno::Reference< beans::XPropertySet > xProps( xTextField, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FileFormat") ), uno::makeAny( nFileFormat ) );

    return xTextField;
}

struct DocPropertyTable
{
    const char* sDocPropertyName;
    const char* sFieldService;
};

static const DocPropertyTable aDocPropertyTables[] =
{
    { "Author", "com.sun.star.text.textfield.docinfo.CreateAuthor" },
    { "Bytes", NULL },
    { "Category", NULL },
    { "Characters",NULL },
    { "CharactersWithSpaces", NULL },
    { "Comments", "com.sun.star.text.textfield.docinfo.Description" },
    { "Company", NULL },
    { "CreateTime", "com.sun.star.text.textfield.docinfo.CreateDateTime" },
    { "HyperlinkBase", NULL },
    { "Keywords", "com.sun.star.text.textfield.docinfo.Keywords" },
    { "LastPrinted", "com.sun.star.text.textfield.docinfo.PrintDateTime" },
    { "LastSavedBy", "com.sun.star.text.textfield.docinfo.ChangeAuthor" },
    { "LastSavedTime", "com.sun.star.text.textfield.docinfo.ChangeDateTime" },
    { "Lines", NULL },
    { "Manager", NULL },
    { "NameofApplication", NULL },
    { "ODMADocID", NULL },
    { "Pages", "com.sun.star.text.textfield.PageCount" },
    { "Paragraphs", "com.sun.star.text.textfield.ParagraphCount" },
    { "RevisionNumber", "com.sun.star.text.textfield.docinfo.Revision" },
    { "Security", NULL },
    { "Subject", "com.sun.star.text.textfield.docinfo.Subject" },
    { "Template", "com.sun.star.text.textfield.TemplateName" },
    { "Title", "com.sun.star.text.textfield.docinfo.Title" },
    { "TotalEditingTime", "com.sun.star.text.textfield.docinfo.EditTime" },
    { "Words", "com.sun.star.text.textfield.WordCount" },
    { NULL, NULL }
};

uno::Reference< text::XTextField > SwVbaFields::Create_Field_DocProperty( const rtl::OUString _text ) throw (uno::RuntimeException)
{
    String aDocProperty;
    _ReadFieldParams aReadParam( _text );
    long nRet;
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
            case -2:
                if( !aDocProperty.Len() )
                    aDocProperty = aReadParam.GetResult();
                break;
            case '*':
                //Skip over MERGEFORMAT
                aReadParam.SkipToNextToken();
                break;
        }
    }
    aDocProperty = comphelper::string::remove(aDocProperty, '"');
    OSL_TRACE("SwVbaFields::Create_Field_DocProperty, the document property name is %s ",rtl::OUStringToOString( aDocProperty, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( aDocProperty.Len() == 0 )
    {
        throw uno::RuntimeException();
    }

    sal_Bool bCustom = sal_True;
    rtl::OUString sFieldService;
    // find the build in document properties
    for( const DocPropertyTable* pTable = aDocPropertyTables; pTable->sDocPropertyName != NULL; pTable++ )
    {
        if( aDocProperty.EqualsIgnoreCaseAscii( pTable->sDocPropertyName ) )
        {
            if( pTable->sFieldService != NULL )
                sFieldService = rtl::OUString::createFromAscii(pTable->sFieldService);
            bCustom = sal_False;
            break;
        }
    }

    if( bCustom )
    {
        sFieldService = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.textfield.docinfo.Custom" ) );
    }
    else if( sFieldService.isEmpty() )
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }

    uno::Reference< text::XTextField > xTextField( mxMSF->createInstance( sFieldService ), uno::UNO_QUERY_THROW );

    if( bCustom )
    {
        uno::Reference< beans::XPropertySet > xProps( xTextField, uno::UNO_QUERY_THROW );
        rtl::OUString sDocPropertyName( aDocProperty );
        xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ), uno::makeAny( sDocPropertyName ) );
    }

    return xTextField;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaFields::createEnumeration() throw (uno::RuntimeException)
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

sal_Int32 SAL_CALL SwVbaFields::Update() throw (uno::RuntimeException)
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
rtl::OUString
SwVbaFields::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaFields"));
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaFields::getElementType() throw (uno::RuntimeException)
{
    return  word::XField::static_type(0);
}

uno::Sequence<rtl::OUString>
SwVbaFields::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Fields" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
