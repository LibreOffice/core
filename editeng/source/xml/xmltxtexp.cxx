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



class SvxEditEngineSourceImpl;



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






SvxEditEngineSourceImpl::SvxEditEngineSourceImpl( EditEngine* pEditEngine )
: maRefCount(0),
  mpEditEngine( pEditEngine ),
  mpTextForwarder(NULL)
{
}



SvxEditEngineSourceImpl::~SvxEditEngineSourceImpl()
{
    delete mpTextForwarder;
}



void SAL_CALL SvxEditEngineSourceImpl::acquire()
{
    osl_atomic_increment( &maRefCount );
}



void SAL_CALL SvxEditEngineSourceImpl::release()
{
    if( ! osl_atomic_decrement( &maRefCount ) )
        delete this;
}



SvxTextForwarder* SvxEditEngineSourceImpl::GetTextForwarder()
{
    if (!mpTextForwarder)
        mpTextForwarder = new SvxEditEngineForwarder( *mpEditEngine );

    return mpTextForwarder;
}


// SvxTextEditSource


SvxEditEngineSource::SvxEditEngineSource( EditEngine* pEditEngine )
{
    mpImpl = new SvxEditEngineSourceImpl( pEditEngine );
    mpImpl->acquire();
}



SvxEditEngineSource::SvxEditEngineSource( SvxEditEngineSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}



SvxEditEngineSource::~SvxEditEngineSource()
{
    mpImpl->release();
}



SvxEditSource* SvxEditEngineSource::Clone() const
{
    return new SvxEditEngineSource( mpImpl );
}



SvxTextForwarder* SvxEditEngineSource::GetTextForwarder()
{
    return mpImpl->GetTextForwarder();
}



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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAnyCompareFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

SvxSimpleUnoModel::SvxSimpleUnoModel()
{
}

SvxSimpleUnoModel::~SvxSimpleUnoModel()
{
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException, std::exception)
{
    if( aServiceSpecifier == "com.sun.star.text.NumberingRules" )
    {
        return uno::Reference< uno::XInterface >(
            SvxCreateNumRule(), uno::UNO_QUERY );
    }
    if (   aServiceSpecifier == "com.sun.star.text.textfield.DateTime"
        || aServiceSpecifier == "com.sun.star.text.TextField.DateTime"
       )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( text::textfield::Type::DATE );
    }

    return SvxUnoTextCreateTextField( aServiceSpecifier );

}

uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception)
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL SvxSimpleUnoModel::getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    Sequence< OUString > aSeq;
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< com::sun::star::ucb::XAnyCompare > SAL_CALL SvxSimpleUnoModel::createAnyCompareByName( const OUString& PropertyName )
    throw(uno::RuntimeException, std::exception)
{
    (void)PropertyName;
    return SvxCreateNumRuleCompare();
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SvxSimpleUnoModel::getStyleFamilies(  )
    throw(uno::RuntimeException, std::exception)
{
    uno::Reference< container::XNameAccess > xStyles;
    return xStyles;
}

// XModel
sal_Bool SAL_CALL SvxSimpleUnoModel::attachResource( const OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aURL;
    (void)aArgs;
    return sal_False;
}

OUString SAL_CALL SvxSimpleUnoModel::getURL(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    OUString aStr;
    return aStr;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SvxSimpleUnoModel::getArgs(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    Sequence< beans::PropertyValue > aSeq;
    return aSeq;
}

void SAL_CALL SvxSimpleUnoModel::connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::lockControllers(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL SvxSimpleUnoModel::hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL SvxSimpleUnoModel::getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Reference< frame::XController > xRet;
    return xRet;
}

void SAL_CALL SvxSimpleUnoModel::setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception)
{
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxSimpleUnoModel::getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Reference< XInterface > xRet;
    return xRet;
}


// XComponent
void SAL_CALL SvxSimpleUnoModel::dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}



class SvxXMLTextExportComponent : public SvXMLExport
{
public:
    SvxXMLTextExportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        EditEngine* pEditEngine,
        const ESelection& rSel,
        const OUString& rFileName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler );

    ~SvxXMLTextExportComponent();

    // methods without content:
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;

private:
    com::sun::star::uno::Reference< com::sun::star::text::XText > mxText;
    ESelection maSelection;
};



SvxXMLTextExportComponent::SvxXMLTextExportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
    EditEngine* pEditEngine,
    const ESelection& rSel,
    const OUString& rFileName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler)
:   SvXMLExport( xContext, "", rFileName, xHandler, ((frame::XModel*)new SvxSimpleUnoModel()), MAP_CM ),
    maSelection( rSel )
{
    SvxEditEngineSource aEditSource( pEditEngine );

    static const SfxItemPropertyMapEntry SvxXMLTextExportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
//      SVX_UNOEDIT_OUTLINER_PROPERTIES,
        {OUString(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace>*)0), 0, 0 },
        {OUString(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,::getBooleanCppuType(), 0, 0 },
        {OUString(UNO_NAME_NUMBERING_LEVEL),        EE_PARA_OUTLLEVEL,  ::getCppuType((const sal_Int16*)0), 0, 0 },
        SVX_UNOEDIT_PARA_PROPERTIES,
        { OUString(), 0, css::uno::Type(), 0, 0 }
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
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

            // create document handler
            uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

            // create output stream and active data source
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( rStream ) );

/* testcode
            const OUString aURL( "file:///e:/test.xml" );
            SfxMedium aMedium( aURL, STREAM_WRITE | STREAM_TRUNC, sal_True );
            aMedium.IsRemote();
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );
*/


            xWriter->setOutputStream( xOut );

            // export text
            const OUString aName;

            // SvxXMLTextExportComponent aExporter( &rEditEngine, rSel, aName, xHandler );
            uno::Reference< xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
            SvxXMLTextExportComponent aExporter( xContext, &rEditEngine, rSel, aName, xHandler );

            aExporter.exportDoc();

/* testcode
            aMedium.Commit();
*/

        }
        while( false );
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
