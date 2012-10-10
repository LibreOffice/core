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


/** this file implements an export of a selected EditEngine content into
    a xml stream. See editeng/source/inc/xmledit.hxx for interface */
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <svl/itemprop.hxx>
#include <svl/brdcst.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sot/storage.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlmetae.hxx>
#include <cppuhelper/implbase4.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <xmloff/xmlexp.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/unofored.hxx>
#include <editeng/unotext.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unofield.hxx>
#include <editeng/editeng.hxx>
#include "editsource.hxx"
#include <editeng/unonrule.hxx>
#include <editeng/unoipset.hxx>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::rtl;
using namespace cppu;

///////////////////////////////////////////////////////////////////////

class SvxEditEngineSourceImpl;

///////////////////////////////////////////////////////////////////////

class SvxEditEngineSourceImpl
{
private:
    oslInterlockedCount maRefCount;

    EditEngine*             mpEditEngine;
    SvxTextForwarder*       mpTextForwarder;

    ~SvxEditEngineSourceImpl();

public:
    SvxEditEngineSourceImpl( EditEngine* pEditEngine );

    void SAL_CALL acquire();
    void SAL_CALL release();

    SvxTextForwarder*       GetTextForwarder();
};

///////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------

SvxEditEngineSourceImpl::SvxEditEngineSourceImpl( EditEngine* pEditEngine )
: maRefCount(0),
  mpEditEngine( pEditEngine ),
  mpTextForwarder(NULL)
{
}

//------------------------------------------------------------------------

SvxEditEngineSourceImpl::~SvxEditEngineSourceImpl()
{
    delete mpTextForwarder;
}

//------------------------------------------------------------------------

void SAL_CALL SvxEditEngineSourceImpl::acquire()
{
    osl_atomic_increment( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL SvxEditEngineSourceImpl::release()
{
    if( ! osl_atomic_decrement( &maRefCount ) )
        delete this;
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxEditEngineSourceImpl::GetTextForwarder()
{
    if (!mpTextForwarder)
        mpTextForwarder = new SvxEditEngineForwarder( *mpEditEngine );

    return mpTextForwarder;
}

// --------------------------------------------------------------------
// SvxTextEditSource
// --------------------------------------------------------------------

SvxEditEngineSource::SvxEditEngineSource( EditEngine* pEditEngine )
{
    mpImpl = new SvxEditEngineSourceImpl( pEditEngine );
    mpImpl->acquire();
}

// --------------------------------------------------------------------

SvxEditEngineSource::SvxEditEngineSource( SvxEditEngineSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}

//------------------------------------------------------------------------

SvxEditEngineSource::~SvxEditEngineSource()
{
    mpImpl->release();
}

//------------------------------------------------------------------------

SvxEditSource* SvxEditEngineSource::Clone() const
{
    return new SvxEditEngineSource( mpImpl );
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxEditEngineSource::GetTextForwarder()
{
    return mpImpl->GetTextForwarder();
}

//------------------------------------------------------------------------

void SvxEditEngineSource::UpdateData()
{
}

class SvxSimpleUnoModel : public cppu::WeakAggImplHelper4<
                                    ::com::sun::star::frame::XModel,
                                    ::com::sun::star::ucb::XAnyCompareFactory,
                                    ::com::sun::star::style::XStyleFamiliesSupplier,
                                    ::com::sun::star::lang::XMultiServiceFactory >
{
public:
    SvxSimpleUnoModel();
    virtual ~SvxSimpleUnoModel();


    // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw(::com::sun::star::uno::RuntimeException);

    // XAnyCompareFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

};

SvxSimpleUnoModel::SvxSimpleUnoModel()
{
}

SvxSimpleUnoModel::~SvxSimpleUnoModel()
{
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException)
{
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.NumberingRules" ) ) )
    {
        return uno::Reference< uno::XInterface >(
            SvxCreateNumRule(), uno::UNO_QUERY );
    }
    if (   (0 == aServiceSpecifier.reverseCompareToAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.textfield.DateTime")))
        || (0 == aServiceSpecifier.reverseCompareToAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextField.DateTime")))
       )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( text::textfield::Type::DATE );
    }

    return SvxUnoTextCreateTextField( aServiceSpecifier );

}

uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

Sequence< ::rtl::OUString > SAL_CALL SvxSimpleUnoModel::getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSeq;
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< com::sun::star::ucb::XAnyCompare > SAL_CALL SvxSimpleUnoModel::createAnyCompareByName( const OUString& PropertyName )
    throw(uno::RuntimeException)
{
    (void)PropertyName;
    return SvxCreateNumRuleCompare();
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SvxSimpleUnoModel::getStyleFamilies(  )
    throw(uno::RuntimeException)
{
    uno::Reference< container::XNameAccess > xStyles;
    return xStyles;
}

// XModel
sal_Bool SAL_CALL SvxSimpleUnoModel::attachResource( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)aURL;
    (void)aArgs;
    return sal_False;
}

::rtl::OUString SAL_CALL SvxSimpleUnoModel::getURL(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OUString aStr;
    return aStr;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SvxSimpleUnoModel::getArgs(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< beans::PropertyValue > aSeq;
    return aSeq;
}

void SAL_CALL SvxSimpleUnoModel::connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::lockControllers(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

sal_Bool SAL_CALL SvxSimpleUnoModel::hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL SvxSimpleUnoModel::getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< frame::XController > xRet;
    return xRet;
}

void SAL_CALL SvxSimpleUnoModel::setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< XInterface > xRet;
    return xRet;
}


// XComponent
void SAL_CALL SvxSimpleUnoModel::dispose(  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxSimpleUnoModel::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& ) throw (::com::sun::star::uno::RuntimeException)
{
}

///////////////////////////////////////////////////////////////////////

class SvxXMLTextExportComponent : public SvXMLExport
{
public:
    SvxXMLTextExportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        EditEngine* pEditEngine,
        const ESelection& rSel,
        const ::rtl::OUString& rFileName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler );

    ~SvxXMLTextExportComponent();

    // methods without content:
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

private:
    com::sun::star::uno::Reference< com::sun::star::text::XText > mxText;
    ESelection maSelection;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTextExportComponent::SvxXMLTextExportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    EditEngine* pEditEngine,
    const ESelection& rSel,
    const ::rtl::OUString& rFileName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler)
:   SvXMLExport( xServiceFactory, rFileName, xHandler, ((frame::XModel*)new SvxSimpleUnoModel()), MAP_CM ),
    maSelection( rSel )
{
    SvxEditEngineSource aEditSource( pEditEngine );

    static const SfxItemPropertyMapEntry SvxXMLTextExportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
//      SVX_UNOEDIT_OUTLINER_PROPERTIES,
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,&::getBooleanCppuType(), 0, 0 },
        {MAP_CHAR_LEN(UNO_NAME_NUMBERING_LEVEL),        EE_PARA_OUTLLEVEL,  &::getCppuType((const sal_Int16*)0), 0, 0 },
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aSvxXMLTextExportComponentPropertySet( SvxXMLTextExportComponentPropertyMap, EditEngine::GetGlobalItemPool() );

    SvxUnoText* pUnoText = new SvxUnoText( &aEditSource, &aSvxXMLTextExportComponentPropertySet, mxText );
    pUnoText->SetSelection( rSel );
    mxText = pUnoText;

    setExportFlags( EXPORT_AUTOSTYLES|EXPORT_CONTENT );
}

SvxXMLTextExportComponent::~SvxXMLTextExportComponent()
{
}

void SvxWriteXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel )
{
    try
    {
        do
        {
            // create service factory

            uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

            if( !xServiceFactory.is() )
            {
                OSL_FAIL( "got no service manager" );
                break;
            }

            // create document handler
            uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

            // create output stream and active data source
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( rStream ) );

/* testcode
            const OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "file:///e:/test.xml" ) );
            SfxMedium aMedium( aURL, STREAM_WRITE | STREAM_TRUNC, sal_True );
            aMedium.IsRemote();
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );
*/


            xWriter->setOutputStream( xOut );

            // export text
            const OUString aName;

            // SvxXMLTextExportComponent aExporter( &rEditEngine, rSel, aName, xHandler );
            uno::Reference< xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
            SvxXMLTextExportComponent aExporter( xServiceFactory, &rEditEngine, rSel, aName, xHandler );

            aExporter.exportDoc();

/* testcode
            aMedium.Commit();
*/

        }
        while( 0 );
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL("exception during xml export");
    }
}

// methods without content:
void SvxXMLTextExportComponent::_ExportAutoStyles()
{
    UniReference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->collectTextAutoStyles( mxText );
    xTextExport->exportTextAutoStyles();
}

void SvxXMLTextExportComponent::_ExportContent()
{
    UniReference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->exportText( mxText );
}

void SvxXMLTextExportComponent::_ExportMasterStyles() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
