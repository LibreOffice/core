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

#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svl/itemprop.hxx>
#include <uno/environment.h>
#include <svl/urihelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/window.hxx>
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>

#include "bibresid.hxx"
#include "bib.hrc"
#include "bibcont.hxx"
#include "bibbeam.hxx"
#include "bibmod.hxx"
#include "bibview.hxx"
#include "framectr.hxx"
#include "datman.hxx"
#include <bibconfig.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;

class BibliographyLoader : public cppu::WeakImplHelper
                            < XServiceInfo, XNameAccess, XPropertySet, XFrameLoader >
{
    HdlBibModul                                     m_pBibMod;
    rtl::Reference<BibDataManager>                  m_xDatMan;
    Reference< XNameAccess >                        m_xColumns;
    Reference< XResultSet >                         m_xCursor;

private:

    void                    loadView(const Reference< XFrame > & aFrame, const OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< XLoadEventListener > & aListener);

    BibDataManager*         GetDataManager()const;
    Reference< XNameAccess > const &    GetDataColumns() const;
    Reference< XResultSet > const &     GetDataCursor() const;
    Reference< sdb::XColumn >           GetIdentifierColumn() const;

public:
                            BibliographyLoader();
                            virtual ~BibliographyLoader() override;

    // XServiceInfo
    OUString               SAL_CALL getImplementationName() override;
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >   SAL_CALL getSupportedServiceNames() override;
    static OUString                getImplementationName_Static() throw(  )

                            {
                                //!
                                return OUString("com.sun.star.extensions.Bibliography");
                                //!
                            }

    //XNameAccess
    virtual Any SAL_CALL getByName(const OUString& aName) override;
    virtual Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& aName) override;

    //XElementAccess
    virtual Type  SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    //XPropertySet
    virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& PropertyName, const Any& aValue) override;
    virtual Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) override;

    static Sequence<OUString>  SAL_CALL getSupportedServiceNames_Static() throw(  );

    /// @throws Exception
    friend  Reference< XInterface >     SAL_CALL BibliographyLoader_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr );

    // XLoader
    virtual void            SAL_CALL load(const Reference< XFrame > & aFrame, const OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< XLoadEventListener > & aListener) override;
    virtual void            SAL_CALL cancel() override;
};

BibliographyLoader::BibliographyLoader() :
    m_pBibMod(nullptr)
{
}

BibliographyLoader::~BibliographyLoader()
{
    Reference< lang::XComponent >  xComp(m_xCursor, UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    if(m_pBibMod)
        CloseBibModul(m_pBibMod);
}


Reference< XInterface >  SAL_CALL BibliographyLoader_CreateInstance( const Reference< XMultiServiceFactory >  & /*rSMgr*/ )
{
    return *(new BibliographyLoader);
}


// XServiceInfo
OUString BibliographyLoader::getImplementationName()

{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool BibliographyLoader::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > BibliographyLoader::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > BibliographyLoader::getSupportedServiceNames_Static() throw(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = "com.sun.star.frame.FrameLoader";
    //!
    aSNS.getArray()[1] = "com.sun.star.frame.Bibliography";
    //!
    return aSNS;
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL bib_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        void * pRet = nullptr;
        if (BibliographyLoader::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            // create the factory
            Reference< XSingleServiceFactory > xFactory =
                cppu::createSingleFactory(
                    static_cast<css::lang::XMultiServiceFactory *>(pServiceManager),
                    BibliographyLoader::getImplementationName_Static(),
                    BibliographyLoader_CreateInstance,
                    BibliographyLoader::getSupportedServiceNames_Static() );
            // acquire, because we return an interface pointer instead of a reference
            xFactory->acquire();
            pRet = xFactory.get();
        }
        return pRet;
    }

}

void BibliographyLoader::cancel()
{
    //!
    //!
}

void BibliographyLoader::load(const Reference< XFrame > & rFrame, const OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener)
{

    SolarMutexGuard aGuard;

    m_pBibMod = OpenBibModul();

    OUString aPartName = rURL.getToken( 1, '/' );
    Reference<XPropertySet> xPrSet(rFrame, UNO_QUERY);
    if(xPrSet.is())
    {
        Any aTitle;
        aTitle <<= BibResId(RID_BIB_STR_FRAME_TITLE).toString();
        xPrSet->setPropertyValue("Title", aTitle);
    }
    if(aPartName == "View" || aPartName == "View1")
    {
        loadView(rFrame, rURL, rArgs, rListener);
    }
}


void BibliographyLoader::loadView(const Reference< XFrame > & rFrame, const OUString& /*rURL*/,
        const Sequence< PropertyValue >& /*rArgs*/,
        const Reference< XLoadEventListener > & rListener)
{
    SolarMutexGuard aGuard;
    //!
    if(!m_pBibMod)
        m_pBibMod = OpenBibModul();

    m_xDatMan = BibModul::createDataManager();
    BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();

    if(aBibDesc.sDataSource.isEmpty())
    {
        DBChangeDialogConfig_Impl aConfig;
        const Sequence<OUString> aSources = aConfig.GetDataSourceNames();
        if(aSources.getLength())
            aBibDesc.sDataSource = aSources.getConstArray()[0];
    }

    Reference< XForm > xForm = m_xDatMan->createDatabaseForm( aBibDesc );

    Reference< awt::XWindow >  aWindow = rFrame->getContainerWindow();
    VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(aWindow);
    assert(pParentComponent);

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( aWindow );

    VclPtrInstance<BibBookContainer> pMyWindow( pParent );
    pMyWindow->Show();

    VclPtrInstance< ::bib::BibView> pView( pMyWindow, m_xDatMan.get(), WB_VSCROLL | WB_HSCROLL | WB_3DLOOK );
    pView->Show();
    m_xDatMan->SetView( pView );

    VclPtrInstance< ::bib::BibBeamer> pBeamer( pMyWindow, m_xDatMan.get() );
    pBeamer->Show();
    pMyWindow->createTopFrame(pBeamer);

    pMyWindow->createBottomFrame(pView);

    Reference< awt::XWindow >  xWin ( pMyWindow->GetComponentInterface(), UNO_QUERY );

    Reference< XController >  xCtrRef( new BibFrameController_Impl( xWin, m_xDatMan.get() ) );

    xCtrRef->attachFrame(rFrame);
    rFrame->setComponent( xWin, xCtrRef);
    pBeamer->SetXController(xCtrRef);

    if (pParentComponent)
    {
        // not earlier because SetFocus() is triggered in setVisible()
        pParentComponent->setVisible(true);
    }

    Reference<XLoadable>(m_xDatMan.get())->load();
    m_xDatMan->RegisterInterceptor(pBeamer);

    if ( rListener.is() )
        rListener->loadFinished( this );

    // attach menu bar
    Reference< XPropertySet > xPropSet( rFrame, UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            Any a = xPropSet->getPropertyValue("LayoutManager");
            a >>= xLayoutManager;
        }
        catch ( const uno::Exception& )
        {
        }
    }

    if ( xLayoutManager.is() )
        xLayoutManager->createElement( "private:resource/menubar/menubar" );
}

BibDataManager* BibliographyLoader::GetDataManager()const
{
    if(!m_xDatMan.is())
    {
        if(!m_pBibMod)
            const_cast< BibliographyLoader* >( this )->m_pBibMod = OpenBibModul();
        const_cast< BibliographyLoader* >( this )->m_xDatMan = BibModul::createDataManager();
    }
    return m_xDatMan.get();
}

Reference< XNameAccess > const & BibliographyLoader::GetDataColumns() const
{
    if (!m_xColumns.is())
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        Reference< XRowSet >  xRowSet(xMgr->createInstance("com.sun.star.sdb.RowSet"), UNO_QUERY);
        Reference< XPropertySet >  xResultSetProps(xRowSet, UNO_QUERY);
        DBG_ASSERT(xResultSetProps.is() , "BibliographyLoader::GetDataCursor : invalid row set (no XResultSet or no XPropertySet) !");

        BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();

        Any aBibUrlAny; aBibUrlAny <<= aBibDesc.sDataSource;
        xResultSetProps->setPropertyValue("DataSourceName", aBibUrlAny);
        Any aCommandType; aCommandType <<= aBibDesc.nCommandType;
        xResultSetProps->setPropertyValue("CommandType", aCommandType);
        Any aTableName; aTableName <<= aBibDesc.sTableOrQuery;
        xResultSetProps->setPropertyValue("Command", aTableName);
        Any aResultSetType; aResultSetType <<= (sal_Int32)(ResultSetType::SCROLL_INSENSITIVE);
        xResultSetProps->setPropertyValue("ResultSetType", aResultSetType);
        Any aResultSetCurrency; aResultSetCurrency <<= (sal_Int32)(ResultSetConcurrency::UPDATABLE);
        xResultSetProps->setPropertyValue("ResultSetConcurrency", aResultSetCurrency);

        bool bSuccess = false;
        try
        {
            xRowSet->execute();
            bSuccess = true;
        }
        catch(const SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = false;
        }

        if (!bSuccess)
        {
            Reference< XComponent >  xSetComp(xRowSet, UNO_QUERY);
            if (xSetComp.is())
                xSetComp->dispose();
            xRowSet = nullptr;
        }
        else
            const_cast<BibliographyLoader*>(this)->m_xCursor = xRowSet.get();

        Reference< sdbcx::XColumnsSupplier >  xSupplyCols(m_xCursor, UNO_QUERY);
        if (xSupplyCols.is())
            const_cast<BibliographyLoader*>(this)->m_xColumns = xSupplyCols->getColumns();
    }

    return m_xColumns;
}

Reference< sdb::XColumn >  BibliographyLoader::GetIdentifierColumn() const
{
    BibDataManager* pDatMan = GetDataManager();
    Reference< XNameAccess >  xColumns = GetDataColumns();
    OUString sIdentifierColumnName = pDatMan->GetIdentifierMapping();

    Reference< sdb::XColumn >  xReturn;
    if (xColumns.is() && xColumns->hasByName(sIdentifierColumnName))
    {
        xReturn.set(xColumns->getByName(sIdentifierColumnName), UNO_QUERY);
    }
    return xReturn;
}

Reference< XResultSet > const &  BibliographyLoader::GetDataCursor() const
{
    if (!m_xCursor.is())
        GetDataColumns();
    if (m_xCursor.is())
        m_xCursor->first();
    return m_xCursor;
}

static OUString lcl_AddProperty(const Reference< XNameAccess >&  xColumns,
        const Mapping* pMapping, const OUString& rColumnName)
{
    OUString sColumnName(rColumnName);
    if(pMapping)
    {
        for(const auto & aColumnPair : pMapping->aColumnPairs)
        {
            if(aColumnPair.sLogicalColumnName == rColumnName)
            {
                sColumnName = aColumnPair.sRealColumnName;
                break;
            }
        }
    }
    OUString uColumnName(sColumnName);
    OUString uRet;
    Reference< sdb::XColumn >  xCol;
    if (xColumns->hasByName(uColumnName))
        xCol.set(xColumns->getByName(uColumnName), UNO_QUERY);
    if (xCol.is())
        uRet = xCol->getString();
    return uRet;
}

Any BibliographyLoader::getByName(const OUString& rName)
{
    Any aRet;
    try
    {
        BibDataManager* pDatMan = GetDataManager();
        Reference< XResultSet >  xCursor = GetDataCursor();
        Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xCursor, UNO_QUERY);
        Reference< XNameAccess >  xColumns;
        if (!xSupplyCols.is())
            return aRet;
        xColumns = xSupplyCols->getColumns();
        DBG_ASSERT(xSupplyCols.is(), "BibliographyLoader::getByName : invalid columns returned by the data cursor (may be the result set is not alive ?) !");
        if (!xColumns.is())
            return aRet;

        const OUString sIdentifierMapping = pDatMan->GetIdentifierMapping();
        Reference< sdb::XColumn >  xColumn;
        if (xColumns->hasByName(sIdentifierMapping))
            xColumn.set(xColumns->getByName(sIdentifierMapping), UNO_QUERY);
        if (xColumn.is())
        {
            do
            {
                if ((rName == xColumn->getString()) && !xColumn->wasNull())
                {
                    Sequence<PropertyValue> aPropSequ(COLUMN_COUNT);
                    PropertyValue* pValues = aPropSequ.getArray();
                    BibConfig* pConfig = BibModul::GetConfig();
                    BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();
                    const Mapping* pMapping = pConfig->GetMapping(aBibDesc);
                    for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
                    {
                        const OUString sColName = pConfig->GetDefColumnName(
                                                    nEntry);
                        pValues[nEntry].Name = sColName;
                        pValues[nEntry].Value <<= lcl_AddProperty(xColumns, pMapping, sColName);
                    }
                    aRet <<= aPropSequ;

                    break;
                }
            }
            while(xCursor->next());
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aRet;
}

Sequence< OUString > BibliographyLoader::getElementNames()
{
    Sequence< OUString > aRet(10);
    int nRealNameCount = 0;
    try
    {
        Reference< XResultSet >  xCursor(GetDataCursor());
        Reference< sdb::XColumn >  xIdColumn(GetIdentifierColumn());
        if (xIdColumn.is()) // implies xCursor.is()
        {
            do
            {
                OUString sTemp = xIdColumn->getString();
                if (!sTemp.isEmpty() && !xIdColumn->wasNull())
                {
                    int nLen = aRet.getLength();
                    if(nLen == nRealNameCount)
                        aRet.realloc(nLen + 10);
                    OUString* pArray = aRet.getArray();
                    pArray[nRealNameCount] = sTemp;
                    nRealNameCount++;
                }
            }
            while (xCursor->next());
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    aRet.realloc(nRealNameCount);
    return aRet;
}

sal_Bool BibliographyLoader::hasByName(const OUString& rName)
{
    bool bRet = false;
    try
    {
        Reference< XResultSet >  xCursor = GetDataCursor();
        Reference< sdb::XColumn >  xIdColumn = GetIdentifierColumn();

        if (xIdColumn.is())     // implies xCursor.is()
        {
            do
            {
                OUString sCurrentId = xIdColumn->getString();
                if (!xIdColumn->wasNull() && rName.startsWith(sCurrentId))
                {
                    bRet = true;
                    break;
                }
            }
            while(xCursor->next());
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bRet;
}

Type  BibliographyLoader::getElementType()
{
    return cppu::UnoType<Sequence<PropertyValue>>::get();
}

sal_Bool BibliographyLoader::hasElements()
{
    Reference< XResultSet >  xCursor = GetDataCursor();
    Reference< XNameAccess >  xColumns = GetDataColumns();
    return xColumns.is() && (xColumns->getElementNames().getLength() != 0);
}

Reference< XPropertySetInfo >  BibliographyLoader::getPropertySetInfo()
{
    static const SfxItemPropertyMapEntry aBibProps_Impl[] =
    {
        { OUString("BibliographyDataFieldNames"), 0, cppu::UnoType<Sequence<PropertyValue>>::get(), PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static Reference< XPropertySetInfo >  xRet =
        SfxItemPropertySet(aBibProps_Impl).getPropertySetInfo();
    return xRet;
}

void BibliographyLoader::setPropertyValue(const OUString& /*PropertyName*/,
                                        const Any& /*aValue*/)
{
    throw UnknownPropertyException();
    //no changeable properties
}

Any BibliographyLoader::getPropertyValue(const OUString& rPropertyName)
{
    Any aRet;
    static const sal_uInt16 aInternalMapping[] =
    {
        IDENTIFIER_POS             , // BibliographyDataField_IDENTIFIER
        AUTHORITYTYPE_POS          , // BibliographyDataField_BIBILIOGRAPHIC_TYPE
        ADDRESS_POS                , // BibliographyDataField_ADDRESS
        ANNOTE_POS                 , // BibliographyDataField_ANNOTE
        AUTHOR_POS                 , // BibliographyDataField_AUTHOR
        BOOKTITLE_POS              , // BibliographyDataField_BOOKTITLE
        CHAPTER_POS                , // BibliographyDataField_CHAPTER
        EDITION_POS                , // BibliographyDataField_EDITION
        EDITOR_POS                 , // BibliographyDataField_EDITOR
        HOWPUBLISHED_POS           , // BibliographyDataField_HOWPUBLISHED
        INSTITUTION_POS            , // BibliographyDataField_INSTITUTION
        JOURNAL_POS                , // BibliographyDataField_JOURNAL
        MONTH_POS                  , // BibliographyDataField_MONTH
        NOTE_POS                   , // BibliographyDataField_NOTE
        NUMBER_POS                 , // BibliographyDataField_NUMBER
        ORGANIZATIONS_POS          , // BibliographyDataField_ORGANIZATIONS
        PAGES_POS                  , // BibliographyDataField_PAGES
        PUBLISHER_POS              , // BibliographyDataField_PUBLISHER
        SCHOOL_POS                 , // BibliographyDataField_SCHOOL
        SERIES_POS                 , // BibliographyDataField_SERIES
        TITLE_POS                  , // BibliographyDataField_TITLE
        REPORTTYPE_POS             , // BibliographyDataField_REPORT_TYPE
        VOLUME_POS                 , // BibliographyDataField_VOLUME
        YEAR_POS                   , // BibliographyDataField_YEAR
        URL_POS                    , // BibliographyDataField_URL
        CUSTOM1_POS                , // BibliographyDataField_CUSTOM1
        CUSTOM2_POS                , // BibliographyDataField_CUSTOM2
        CUSTOM3_POS                , // BibliographyDataField_CUSTOM3
        CUSTOM4_POS                , // BibliographyDataField_CUSTOM4
        CUSTOM5_POS                , // BibliographyDataField_CUSTOM5
        ISBN_POS                    //BibliographyDataField_ISBN
    };
    if(rPropertyName == "BibliographyDataFieldNames")
    {
        Sequence<PropertyValue> aSeq(COLUMN_COUNT);
        PropertyValue* pArray = aSeq.getArray();
        BibConfig* pConfig = BibModul::GetConfig();
        for(sal_uInt16 i = 0; i <= text::BibliographyDataField::ISBN ; i++)
        {
            pArray[i].Name = pConfig->GetDefColumnName(aInternalMapping[i]);
            pArray[i].Value <<= (sal_Int16) i;
        }
        aRet <<= aSeq;
    }
    else
        throw UnknownPropertyException();
    return aRet;
}

void BibliographyLoader::addPropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}

void BibliographyLoader::removePropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}

void BibliographyLoader::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}

void BibliographyLoader::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
