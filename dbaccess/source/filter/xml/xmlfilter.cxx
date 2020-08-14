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

#include <sal/config.h>
#include <sal/log.hxx>

#include <vcl/errinf.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include "xmlfilter.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <xmloff/ProgressBarHelper.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include "xmlDatabase.hxx"
#include "xmlEnums.hxx"
#include <strings.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include "xmlStyleImport.hxx"
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <svtools/sfxecode.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <connectivity/DriversConfig.hxx>
#include <rtl/uri.hxx>

using namespace ::com::sun::star;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sdb_DBFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::dbaxml::ODBFilter(context));
}


namespace dbaxml
{
    using namespace ::com::sun::star::util;
    /// read a component (file + filter version)
static ErrCode ReadThroughComponent(
    const uno::Reference<XInputStream>& xInputStream,
    const uno::Reference<XComponent>& xModelComponent,
    const uno::Reference<XComponentContext> & rxContext,
    ODBFilter& _rFilter )
{
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rxContext.is(), "factory missing");

    // prepare ParserInputSrouce
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // connect model and filter
    _rFilter.setTargetDocument( xModelComponent );

    // finally, parser the stream
    try
    {
        _rFilter.parseStream( aParserInput );
    }
    catch (const SAXParseException&)
    {
#if OSL_DEBUG_LEVEL > 0
        TOOLS_WARN_EXCEPTION("dbaccess", "SAX parse exception caught while importing");
#endif
        return ErrCode(1);
    }
    catch (const SAXException&)
    {
        return ErrCode(1);
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    // success!
    return ERRCODE_NONE;
}


/// read a component (storage version)
static ErrCode ReadThroughComponent(
    const uno::Reference< embed::XStorage >& xStorage,
    const uno::Reference<XComponent>& xModelComponent,
    const char* pStreamName,
    const uno::Reference<XComponentContext> & rxContext,
    ODBFilter& _rFilter)
{
    OSL_ENSURE( xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    if ( xStorage.is() )
    {
        uno::Reference< io::XStream > xDocStream;

        try
        {
            // open stream (and set parser input)
            OUString sStreamName = OUString::createFromAscii(pStreamName);
            if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
            {
                // stream name not found! return immediately with OK signal
                return ERRCODE_NONE;
            }

            // get input stream
            xDocStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
        }
        catch (const packages::WrongPasswordException&)
        {
            return ERRCODE_SFX_WRONGPASSWORD;
        }
        catch (const uno::Exception&)
        {
            return ErrCode(1); // TODO/LATER: error handling
        }

        uno::Reference< XInputStream > xInputStream = xDocStream->getInputStream();
        // read from the stream
        return ReadThroughComponent( xInputStream
                                    ,xModelComponent
                                    ,rxContext
                                    ,_rFilter );
    }

    // TODO/LATER: better error handling
    return ErrCode(1);
}


ODBFilter::ODBFilter( const uno::Reference< XComponentContext >& _rxContext )
    : SvXMLImport(_rxContext, getImplementationName_Static())
    , m_bNewFormat(false)
{

    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_10TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);
    GetNamespaceMap().Add( "_db",
                        GetXMLToken(XML_N_DB),
                        XML_NAMESPACE_DB );

    GetNamespaceMap().Add( "__db",
                        GetXMLToken(XML_N_DB_OASIS),
                        XML_NAMESPACE_DB );
}


ODBFilter::~ODBFilter() throw()
{

}


OUString ODBFilter::getImplementationName_Static()
{
    return "com.sun.star.comp.sdb.DBFilter";
}


namespace {
class FocusWindowWaitGuard
{
public:
    FocusWindowWaitGuard()
    {
        SolarMutexGuard aGuard;
        mpWindow.set(Application::GetFocusWindow());
        if (mpWindow)
            mpWindow->EnterWait();
    }
    ~FocusWindowWaitGuard()
    {
        if (mpWindow)
        {
            SolarMutexGuard aGuard;
            mpWindow->LeaveWait();
        }
    }
private:
    VclPtr<vcl::Window> mpWindow;
};
}

sal_Bool SAL_CALL ODBFilter::filter( const Sequence< PropertyValue >& rDescriptor )
{
    FocusWindowWaitGuard aWindowFocusGuard;
    bool    bRet = false;

    if ( GetModel().is() )
        bRet = implImport( rDescriptor );

    return bRet;
}


bool ODBFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
{
    OUString sFileName;
    ::comphelper::NamedValueCollection aMediaDescriptor( rDescriptor );

    uno::Reference<embed::XStorage> xStorage = GetSourceStorage();

    bool bRet = true;
    if (!xStorage.is())
    {
        if (aMediaDescriptor.has("URL"))
            sFileName = aMediaDescriptor.getOrDefault("URL", OUString());
        if (sFileName.isEmpty() && aMediaDescriptor.has("FileName"))
            sFileName = aMediaDescriptor.getOrDefault("FileName", sFileName);

        OSL_ENSURE(!sFileName.isEmpty(), "ODBFilter::implImport: no URL given!");
        bRet = !sFileName.isEmpty();
    }

    if ( bRet )
    {

        tools::SvRef<SfxMedium> pMedium;
        if (!xStorage.is())
        {
            OUString sStreamRelPath;
            if (sFileName.startsWithIgnoreAsciiCase("vnd.sun.star.pkg:"))
            {
                // In this case the authority contains the real path, and the path is the embedded stream name.
                auto const uri = css::uri::UriReferenceFactory::create(GetComponentContext())
                    ->parse(sFileName);
                if (uri.is() && uri->isAbsolute()
                    && uri->hasAuthority() && !uri->hasQuery() && !uri->hasFragment())
                {
                    auto const auth = uri->getAuthority();
                    auto const decAuth = rtl::Uri::decode(
                        auth, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
                    auto path = uri->getPath();
                    if (!path.isEmpty()) {
                        assert(path[0] == '/');
                        path = path.copy(1);
                    }
                    auto const decPath = rtl::Uri::decode(
                        path, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
                        //TODO: really decode path?
                    if (auth.isEmpty() == decAuth.isEmpty() && path.isEmpty() == decPath.isEmpty())
                    {
                        // Decoding of auth and path to UTF-8 succeeded:
                        sFileName = decAuth;
                        sStreamRelPath = decPath;
                    } else {
                        SAL_WARN(
                            "dbaccess",
                            "<" << sFileName << "> cannot be parse as vnd.sun.star.pkg URL");
                    }
                } else {
                    SAL_WARN(
                        "dbaccess",
                        "<" << sFileName << "> cannot be parse as vnd.sun.star.pkg URL");
                }
            }

            pMedium = new SfxMedium(sFileName, (StreamMode::READ | StreamMode::NOCREATE));
            try
            {
                xStorage.set(pMedium->GetStorage(false), UNO_SET_THROW);

                if (!sStreamRelPath.isEmpty())
                    xStorage = xStorage->openStorageElement(sStreamRelPath, embed::ElementModes::READ);
            }
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
            {
                Any aError = ::cppu::getCaughtException();
                throw lang::WrappedTargetRuntimeException(OUString(), *this, aError);
            }
        }

        uno::Reference<sdb::XOfficeDatabaseDocument> xOfficeDoc(GetModel(),UNO_QUERY_THROW);
        m_xDataSource.set(xOfficeDoc->getDataSource(),UNO_QUERY_THROW);
        uno::Reference< XNumberFormatsSupplier > xNum(m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
        SetNumberFormatsSupplier(xNum);

        uno::Reference<XComponent> xModel(GetModel());
        ErrCode nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"settings.xml"
                                    ,GetComponentContext()
                                    ,*this
                                    );

        if ( nRet == ERRCODE_NONE )
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"content.xml"
                                    ,GetComponentContext()
                                    ,*this
                                    );

        bRet = nRet == ERRCODE_NONE;

        if ( bRet )
        {
            uno::Reference< XModifiable > xModi(GetModel(),UNO_QUERY);
            if ( xModi.is() )
                xModi->setModified(false);
        }
        else
        {
            if ( nRet == ERRCODE_IO_BROKENPACKAGE )
                    ;// TODO/LATER: no way to transport the error outside from the filter!
            else
            {
                // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly! But for now this is the only way!
                ErrorHandler::HandleError( nRet );
                if( nRet.IsWarning() )
                    bRet = true;
            }
        }
    }

    return bRet;
}

namespace {

class DBXMLDocumentSettingsContext : public SvXMLImportContext
{
public:
    DBXMLDocumentSettingsContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        if (nElement == XML_ELEMENT(OFFICE, XML_SETTINGS))
        {
            return new XMLDocumentSettingsContext(GetImport());
        }
        return nullptr;
    }
};

class DBXMLDocumentStylesContext : public SvXMLImportContext
{
public:
    DBXMLDocumentStylesContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual void SAL_CALL startFastElement(
            sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override {}

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        ODBFilter & rImport(static_cast<ODBFilter&>(GetImport()));
        switch (nElement)
        {
            case XML_ELEMENT(OFFICE, XML_STYLES):
            case XML_ELEMENT(OOO, XML_STYLES):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateStylesContext(false);
                break;
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
            case XML_ELEMENT(OOO, XML_AUTOMATIC_STYLES):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateStylesContext(true);
                break;
        }
        return nullptr;
    }
};

class DBXMLDocumentBodyContext : public SvXMLImportContext
{
public:
    DBXMLDocumentBodyContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        ODBFilter & rImport(static_cast<ODBFilter&>(GetImport()));
        switch (nElement)
        {
            case XML_ELEMENT(OFFICE, XML_DATABASE):
            case XML_ELEMENT(OOO, XML_DATABASE):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return new OXMLDatabase(rImport);
            default: break;
        }
        return nullptr;
    }
};

class DBXMLDocumentContentContext : public SvXMLImportContext
{
public:
    DBXMLDocumentContentContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        ODBFilter & rImport(static_cast<ODBFilter&>(GetImport()));
        switch (nElement)
        {
            case XML_ELEMENT(OFFICE, XML_BODY):
            case XML_ELEMENT(OOO, XML_BODY):
                return new DBXMLDocumentBodyContext(rImport);
                break;
            case XML_ELEMENT(OFFICE, XML_SCRIPTS):
                return new XMLScriptContext(GetImport(), rImport.GetModel());
                break;
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
            case XML_ELEMENT(OOO, XML_AUTOMATIC_STYLES):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateStylesContext(true);
                break;
            default: break;
        }
        return nullptr;
    }
};

}

SvXMLImportContext* ODBFilter::CreateFastContext(sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_SETTINGS):
        case XML_ELEMENT(OOO, XML_DOCUMENT_SETTINGS):
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new DBXMLDocumentSettingsContext(*this);
            break;
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_STYLES):
        case XML_ELEMENT(OOO, XML_DOCUMENT_STYLES):
            pContext = new DBXMLDocumentStylesContext(*this);
            break;
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_CONTENT):
        case XML_ELEMENT(OOO, XML_DOCUMENT_CONTENT):
            pContext = new DBXMLDocumentContentContext(*this);
            break;
    }

    return pContext;
}


void ODBFilter::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
{
    const PropertyValue *pIter = aViewProps.getConstArray();
    const PropertyValue *pEnd = pIter + aViewProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name == "Queries" )
        {
            fillPropertyMap(pIter->Value,m_aQuerySettings);
        }
        else if ( pIter->Name == "Tables" )
        {
            fillPropertyMap(pIter->Value,m_aTablesSettings);
        }
    }
}


void ODBFilter::SetConfigurationSettings(const Sequence<PropertyValue>& aConfigProps)
{
    const PropertyValue *pIter = aConfigProps.getConstArray();
    const PropertyValue *pEnd = pIter + aConfigProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name == "layout-settings" )
        {
            Sequence<PropertyValue> aWindows;
            pIter->Value >>= aWindows;
            uno::Reference<XPropertySet> xProp(getDataSource());
            if ( xProp.is() )
                xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aWindows));
        }
    }
}


void ODBFilter::fillPropertyMap(const Any& _rValue,TPropertyNameMap& _rMap)
{
    Sequence<PropertyValue> aWindows;
    _rValue >>= aWindows;
    const PropertyValue *pIter = aWindows.getConstArray();
    const PropertyValue *pEnd = pIter + aWindows.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        Sequence<PropertyValue> aValue;
        pIter->Value >>= aValue;
        _rMap.emplace( pIter->Name,aValue );
    }

}

SvXMLImportContext* ODBFilter::CreateStylesContext( bool bIsAutoStyle )
{
    SvXMLImportContext *pContext = new OTableStylesContext(*this, bIsAutoStyle);
    if (bIsAutoStyle)
        SetAutoStyles(static_cast<SvXMLStylesContext*>(pContext));
    else
        SetStyles(static_cast<SvXMLStylesContext*>(pContext));

    return pContext;
}


rtl::Reference < XMLPropertySetMapper > const & ODBFilter::GetTableStylesPropertySetMapper() const
{
    if ( !m_xTableStylesPropertySetMapper.is() )
    {
        m_xTableStylesPropertySetMapper = OXMLHelper::GetTableStylesPropertySetMapper( false);
    }
    return m_xTableStylesPropertySetMapper;
}


rtl::Reference < XMLPropertySetMapper > const & ODBFilter::GetColumnStylesPropertySetMapper() const
{
    if ( !m_xColumnStylesPropertySetMapper.is() )
    {
        m_xColumnStylesPropertySetMapper = OXMLHelper::GetColumnStylesPropertySetMapper( false);
    }
    return m_xColumnStylesPropertySetMapper;
}


rtl::Reference < XMLPropertySetMapper > const & ODBFilter::GetCellStylesPropertySetMapper() const
{
    if ( !m_xCellStylesPropertySetMapper.is() )
    {
        m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylesPropertySetMapper( false);
    }
    return m_xCellStylesPropertySetMapper;
}


void ODBFilter::setPropertyInfo()
{
    Reference<XPropertySet> xDataSource(getDataSource());
    if ( !xDataSource.is() )
        return;

    ::connectivity::DriversConfig aDriverConfig(GetComponentContext());
    const OUString sURL = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_URL));
    ::comphelper::NamedValueCollection aDataSourceSettings = aDriverConfig.getProperties( sURL );

    Sequence<PropertyValue> aInfo;
    if ( !m_aInfoSequence.empty() )
        aInfo = comphelper::containerToSequence(m_aInfoSequence);
    aDataSourceSettings.merge( ::comphelper::NamedValueCollection( aInfo ), true );

    aDataSourceSettings >>= aInfo;
    if ( aInfo.hasElements() )
    {
        try
        {
            xDataSource->setPropertyValue(PROPERTY_INFO,makeAny(aInfo));
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
