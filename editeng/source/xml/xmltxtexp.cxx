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
#include <memory>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <svl/itemprop.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sot/storage.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlmetae.hxx>
#include <cppuhelper/implbase4.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/streamwrap.hxx>
#include <xmloff/xmlexp.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/unofored.hxx>
#include <editeng/unotext.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unofield.hxx>
#include <editeng/editeng.hxx>
#include "editsource.hxx"
#include <editxml.hxx>
#include <editeng/unonrule.hxx>
#include <editeng/unoipset.hxx>
#include <unomodel.hxx>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace cppu;

class SvxEditEngineSourceImpl;

class SvxEditEngineSourceImpl : public salhelper::SimpleReferenceObject
{
private:
    EditEngine*                        mpEditEngine;
    std::unique_ptr<SvxTextForwarder>  mpTextForwarder;

    virtual ~SvxEditEngineSourceImpl() override;

public:
    explicit SvxEditEngineSourceImpl( EditEngine* pEditEngine );

    SvxTextForwarder*       GetTextForwarder();
};

SvxEditEngineSourceImpl::SvxEditEngineSourceImpl( EditEngine* pEditEngine )
: mpEditEngine( pEditEngine )
{
}

SvxEditEngineSourceImpl::~SvxEditEngineSourceImpl()
{
}

SvxTextForwarder* SvxEditEngineSourceImpl::GetTextForwarder()
{
    if (!mpTextForwarder)
        mpTextForwarder.reset( new SvxEditEngineForwarder( *mpEditEngine ) );

    return mpTextForwarder.get();
}

// SvxTextEditSource
SvxEditEngineSource::SvxEditEngineSource( EditEngine* pEditEngine )
   : mxImpl( new SvxEditEngineSourceImpl( pEditEngine ) )
{
}

SvxEditEngineSource::SvxEditEngineSource( SvxEditEngineSourceImpl* pImpl )
   : mxImpl(pImpl)
{
}

SvxEditEngineSource::~SvxEditEngineSource()
{
}

std::unique_ptr<SvxEditSource> SvxEditEngineSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new SvxEditEngineSource( mxImpl.get() ));
}

SvxTextForwarder* SvxEditEngineSource::GetTextForwarder()
{
    return mxImpl->GetTextForwarder();
}


void SvxEditEngineSource::UpdateData()
{
}

// class SvxSimpleUnoModel

SvxSimpleUnoModel::SvxSimpleUnoModel()
{
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstance( const OUString& aServiceSpecifier )
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

    if( aServiceSpecifier == "com.sun.star.text.TextField.URL" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField(text::textfield::Type::URL));
    }

    return SvxUnoTextCreateTextField( aServiceSpecifier );

}

uno::Reference< css::uno::XInterface > SAL_CALL SvxSimpleUnoModel::createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& )
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL SvxSimpleUnoModel::getAvailableServiceNames(  )
{
    Sequence< OUString > aSeq;
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SvxSimpleUnoModel::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SvxSimpleUnoModel::getStyleFamilies(  )
{
    uno::Reference< container::XNameAccess > xStyles;
    return xStyles;
}

// XModel
sal_Bool SAL_CALL SvxSimpleUnoModel::attachResource( const OUString&, const css::uno::Sequence< css::beans::PropertyValue >& )
{
    return false;
}

OUString SAL_CALL SvxSimpleUnoModel::getURL(  )
{
    return OUString();
}

css::uno::Sequence< css::beans::PropertyValue > SAL_CALL SvxSimpleUnoModel::getArgs(  )
{
    Sequence< beans::PropertyValue > aSeq;
    return aSeq;
}

void SAL_CALL SvxSimpleUnoModel::connectController( const css::uno::Reference< css::frame::XController >& )
{
}

void SAL_CALL SvxSimpleUnoModel::disconnectController( const css::uno::Reference< css::frame::XController >& )
{
}

void SAL_CALL SvxSimpleUnoModel::lockControllers(  )
{
}

void SAL_CALL SvxSimpleUnoModel::unlockControllers(  )
{
}

sal_Bool SAL_CALL SvxSimpleUnoModel::hasControllersLocked(  )
{
    return true;
}

css::uno::Reference< css::frame::XController > SAL_CALL SvxSimpleUnoModel::getCurrentController(  )
{
    uno::Reference< frame::XController > xRet;
    return xRet;
}

void SAL_CALL SvxSimpleUnoModel::setCurrentController( const css::uno::Reference< css::frame::XController >& )
{
}

css::uno::Reference< css::uno::XInterface > SAL_CALL SvxSimpleUnoModel::getCurrentSelection(  )
{
    uno::Reference< XInterface > xRet;
    return xRet;
}


// XComponent
void SAL_CALL SvxSimpleUnoModel::dispose(  )
{
}

void SAL_CALL SvxSimpleUnoModel::addEventListener( const css::uno::Reference< css::lang::XEventListener >& )
{
}

void SAL_CALL SvxSimpleUnoModel::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
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

    // methods without content:
    virtual void ExportAutoStyles_() override;
    virtual void ExportMasterStyles_() override;
    virtual void ExportContent_() override;

private:
    css::uno::Reference< css::text::XText > mxText;
};


SvxXMLTextExportComponent::SvxXMLTextExportComponent(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    EditEngine* pEditEngine,
    const ESelection& rSel,
    const OUString& rFileName,
    const css::uno::Reference< css::xml::sax::XDocumentHandler > & xHandler)
:   SvXMLExport( xContext, "", rFileName, xHandler, static_cast<frame::XModel*>(new SvxSimpleUnoModel()), FieldUnit::CM,
    SvXMLExportFlags::OASIS  |  SvXMLExportFlags::AUTOSTYLES  |  SvXMLExportFlags::CONTENT )
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
            SvFileStream aStream(aURL, StreamMode::WRITE | StreamMode::TRUNC);
            xOut = new utl::OOutputStreamWrapper(aStream);
*/


            xWriter->setOutputStream( xOut );

            // export text
            const OUString aName;

            // SvxXMLTextExportComponent aExporter( &rEditEngine, rSel, aName, xHandler );
            uno::Reference< xml::sax::XDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
            rtl::Reference< SvxXMLTextExportComponent > xExporter( new SvxXMLTextExportComponent( xContext, &rEditEngine, rSel, aName, xHandler ) );

            xExporter->exportDoc();

/* testcode
            aStream.Close();
*/

        }
        while( false );
    }
    catch( const uno::Exception& )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("editeng", "exception during xml export: " << exceptionToString(ex));
    }
}

// methods without content:
void SvxXMLTextExportComponent::ExportAutoStyles_()
{
    rtl::Reference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->collectTextAutoStyles( mxText );
    xTextExport->exportTextAutoStyles();
}

void SvxXMLTextExportComponent::ExportContent_()
{
    rtl::Reference< XMLTextParagraphExport > xTextExport( GetTextParagraphExport() );

    xTextExport->exportText( mxText );
}

void SvxXMLTextExportComponent::ExportMasterStyles_() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
