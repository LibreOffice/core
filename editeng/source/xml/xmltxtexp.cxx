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
#include "editxml.hxx"
#include <editeng/unonrule.hxx>
#include <editeng/unoipset.hxx>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
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
    explicit SvxEditEngineSourceImpl( EditEngine* pEditEngine );

    void SAL_CALL acquire();
    void SAL_CALL release();

    SvxTextForwarder*       GetTextForwarder();
};

SvxEditEngineSourceImpl::SvxEditEngineSourceImpl( EditEngine* pEditEngine )
: maRefCount(0),
  mpEditEngine( pEditEngine ),
  mpTextForwarder(nullptr)
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
                                    css::frame::XModel,
                                    css::ucb::XAnyCompareFactory,
                                    css::style::XStyleFamiliesSupplier,
                                    css::lang::XMultiServiceFactory >
{
public:
    SvxSimpleUnoModel();
    virtual ~SvxSimpleUnoModel();


    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XAnyCompareFactory
    virtual css::uno::Reference< css::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const OUString& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getURL(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& xController ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& xController ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL lockControllers(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unlockControllers(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

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
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::DATE ));
    }

    return SvxUnoTextCreateTextField( aServiceSpecifier );

}

uno::Reference< css::uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL SvxSimpleUnoModel::getAvailableServiceNames(  ) throw(css::uno::RuntimeException, std::exception)
{
    Sequence< OUString > aSeq;
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SvxSimpleUnoModel::createAnyCompareByName( const OUString& PropertyName )
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
sal_Bool SAL_CALL SvxSimpleUnoModel::attachResource( const OUString& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) throw (css::uno::RuntimeException, std::exception)
{
    (void)aURL;
    (void)aArgs;
    return false;
}

OUString SAL_CALL SvxSimpleUnoModel::getURL(  ) throw (css::uno::RuntimeException, std::exception)
{
    OUString aStr;
    return aStr;
}

css::uno::Sequence< css::beans::PropertyValue > SAL_CALL SvxSimpleUnoModel::getArgs(  ) throw (css::uno::RuntimeException, std::exception)
{
    Sequence< beans::PropertyValue > aSeq;
    return aSeq;
}

void SAL_CALL SvxSimpleUnoModel::connectController( const css::uno::Reference< css::frame::XController >& ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::disconnectController( const css::uno::Reference< css::frame::XController >& ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::lockControllers(  ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::unlockControllers(  ) throw (css::uno::RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL SvxSimpleUnoModel::hasControllersLocked(  ) throw (css::uno::RuntimeException, std::exception)
{
    return true;
}

css::uno::Reference< css::frame::XController > SAL_CALL SvxSimpleUnoModel::getCurrentController(  ) throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< frame::XController > xRet;
    return xRet;
}

void SAL_CALL SvxSimpleUnoModel::setCurrentController( const css::uno::Reference< css::frame::XController >& ) throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception)
{
}

css::uno::Reference< css::uno::XInterface > SAL_CALL SvxSimpleUnoModel::getCurrentSelection(  ) throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< XInterface > xRet;
    return xRet;
}


// XComponent
void SAL_CALL SvxSimpleUnoModel::dispose(  ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::addEventListener( const css::uno::Reference< css::lang::XEventListener >& ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxSimpleUnoModel::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& ) throw (css::uno::RuntimeException, std::exception)
{
}


class SvxXMLTextExportComponent : public SvXMLExport
{
public:
    SvxXMLTextExportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        EditEngine* pEditEngine,
        const ESelection& rSel,
        const OUString& rFileName,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& rHandler );

    virtual ~SvxXMLTextExportComponent();

    // methods without content:
    virtual void _ExportAutoStyles() override;
    virtual void _ExportMasterStyles() override;
    virtual void _ExportContent() override;

private:
    css::uno::Reference< css::text::XText > mxText;
};


SvxXMLTextExportComponent::SvxXMLTextExportComponent(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    EditEngine* pEditEngine,
    const ESelection& rSel,
    const OUString& rFileName,
    const css::uno::Reference< css::xml::sax::XDocumentHandler > & xHandler)
:   SvXMLExport( xContext, "", rFileName, xHandler, (static_cast<frame::XModel*>(new SvxSimpleUnoModel())), FUNIT_CM )
{
    SvxEditEngineSource aEditSource( pEditEngine );

    static const SfxItemPropertyMapEntry SvxXMLTextExportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        { OUString(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  cppu::UnoType<css::container::XIndexReplace>::get(), 0, 0 },
        { OUString(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(UNO_NAME_NUMBERING_LEVEL),        EE_PARA_OUTLLEVEL,  ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        SVX_UNOEDIT_PARA_PROPERTIES,
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aSvxXMLTextExportComponentPropertySet( SvxXMLTextExportComponentPropertyMap, EditEngine::GetGlobalItemPool() );

    SvxUnoText* pUnoText = new SvxUnoText( &aEditSource, &aSvxXMLTextExportComponentPropertySet, mxText );
    pUnoText->SetSelection( rSel );
    mxText = pUnoText;

    setExportFlags( SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT );
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
            SfxMedium aMedium( aURL, StreamMode::WRITE | StreamMode::TRUNC, sal_True );
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );
*/


            xWriter->setOutputStream( xOut );

            // export text
            const OUString aName;

            // SvxXMLTextExportComponent aExporter( &rEditEngine, rSel, aName, xHandler );
            uno::Reference< xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
            uno::Reference< SvxXMLTextExportComponent > xExporter( new SvxXMLTextExportComponent( xContext, &rEditEngine, rSel, aName, xHandler ) );

            xExporter->exportDoc();

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
    rtl::Reference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->collectTextAutoStyles( mxText );
    xTextExport->exportTextAutoStyles();
}

void SvxXMLTextExportComponent::_ExportContent()
{
    rtl::Reference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->exportText( mxText );
}

void SvxXMLTextExportComponent::_ExportMasterStyles() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
