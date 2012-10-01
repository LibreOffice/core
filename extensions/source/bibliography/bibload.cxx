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

#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/weak.hxx>
#include <svl/itemprop.hxx>
#include <uno/environment.h>
#include <svl/urihelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
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
#include <cppuhelper/implbase4.hxx> // helper for implementations

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;

#define C2U(cChar) OUString::createFromAscii(cChar)


//-----------------------------------------------------------------------------

#define PROPERTY_FRAME                      1

class BibliographyLoader : public cppu::WeakImplHelper4
                            < XServiceInfo, XNameAccess, XPropertySet, XFrameLoader >
{
    HdlBibModul                                     m_pBibMod;
    Reference< XLoadable >                          m_xDatMan;
    BibDataManager*                                 m_pDatMan;
    Reference< XNameAccess >                        m_xColumns;
    Reference< XResultSet >                         m_xCursor;

private:

    void                    loadView(const Reference< XFrame > & aFrame, const rtl::OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< XLoadEventListener > & aListener);

    BibDataManager*         GetDataManager()const;
    Reference< XNameAccess >            GetDataColumns() const;
    Reference< XResultSet >             GetDataCursor() const;
    Reference< sdb::XColumn >               GetIdentifierColumn() const;

public:
                            BibliographyLoader();
                            ~BibliographyLoader();

    // XServiceInfo
    rtl::OUString               SAL_CALL getImplementationName() throw(  );
    sal_Bool                    SAL_CALL supportsService(const rtl::OUString& ServiceName) throw(  );
    Sequence< rtl::OUString >   SAL_CALL getSupportedServiceNames(void) throw(  );
    static rtl::OUString                getImplementationName_Static() throw(  )

                            {
                                //!
                                return C2U("com.sun.star.extensions.Bibliography");
                                //!
                            }

    //XNameAccess
    virtual Any SAL_CALL getByName(const rtl::OUString& aName) throw ( NoSuchElementException, WrappedTargetException, RuntimeException );
    virtual Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw ( RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& aName) throw ( RuntimeException );

    //XElementAccess
    virtual Type  SAL_CALL getElementType(void) throw ( RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(void) throw ( RuntimeException );

    //XPropertySet
    virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo(void) throw ( RuntimeException );
    virtual void SAL_CALL setPropertyValue(const rtl::OUString& PropertyName, const Any& aValue) throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException );
    virtual Any SAL_CALL getPropertyValue(const rtl::OUString& PropertyName) throw ( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const rtl::OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const rtl::OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const rtl::OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const rtl::OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException );

    static Sequence<rtl::OUString>  SAL_CALL getSupportedServiceNames_Static(void) throw(  );

    friend  Reference< XInterface >     SAL_CALL BibliographyLoader_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw( Exception );

    // XLoader
    virtual void            SAL_CALL load(const Reference< XFrame > & aFrame, const rtl::OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< XLoadEventListener > & aListener) throw (::com::sun::star::uno::RuntimeException);
    virtual void            SAL_CALL cancel(void) throw (::com::sun::star::uno::RuntimeException);
};

BibliographyLoader::BibliographyLoader() :
    m_pBibMod(0),
    m_pDatMan(0)
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


Reference< XInterface >  SAL_CALL BibliographyLoader_CreateInstance( const Reference< XMultiServiceFactory >  & /*rSMgr*/ ) throw( Exception )
{
    return *(new BibliographyLoader);
}


// XServiceInfo
rtl::OUString BibliographyLoader::getImplementationName() throw(  )

{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool BibliographyLoader::supportsService(const rtl::OUString& ServiceName) throw(  )
{
    Sequence< rtl::OUString > aSNL = getSupportedServiceNames();
    const rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< rtl::OUString > BibliographyLoader::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< rtl::OUString > BibliographyLoader::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = C2U("com.sun.star.frame.FrameLoader");
    //!
    aSNS.getArray()[1] = C2U("com.sun.star.frame.Bibliography");
    //!
    return aSNS;
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL bib_component_getFactory(
        const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, void * /*pRegistryKey*/ )
    {
        void * pRet = 0;
        if (!BibliographyLoader::getImplementationName_Static().compareToAscii( pImplName ) )
        {
            // create the factory
            Reference< XSingleServiceFactory > xFactory =
                cppu::createSingleFactory(
                    pServiceManager,
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
// -----------------------------------------------------------------------
void BibliographyLoader::cancel(void) throw (::com::sun::star::uno::RuntimeException)
{
    //!
    //!
}

// -----------------------------------------------------------------------
namespace
{
    // lp#527938, debian#602953, fdo#33266, i#105408
    static bool lcl_isBaseAvailable()
    {
        try
        {
            Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
            Reference< XAggregation > xAggregate = Reference< XAggregation >( xMgr->createInstance(C2U("com.sun.star.sdb.RowSet")), UNO_QUERY);
            return xAggregate.is();
        }
        catch(...)
        {
            return false;
        }
    }
}
void BibliographyLoader::load(const Reference< XFrame > & rFrame, const rtl::OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw (::com::sun::star::uno::RuntimeException)
{
    // lp#527938, debian#602953, fdo#33266, i#105408
    // make sure we actually can instanciate services from base first
    if(!lcl_isBaseAvailable())
        return;

    SolarMutexGuard aGuard;
    
    m_pBibMod = OpenBibModul();

    String aURLStr( rURL );
    String aPartName = aURLStr.GetToken( 1, '/' );
    Reference<XPropertySet> xPrSet(rFrame, UNO_QUERY);
    if(xPrSet.is())
    {
        Any aTitle;
        aTitle <<= OUString(String(BibResId(RID_BIB_STR_FRAME_TITLE)));
        xPrSet->setPropertyValue(C2U("Title"), aTitle);
    }
    if(aPartName.EqualsAscii("View") || aPartName.EqualsAscii("View1"))
    {
        loadView(rFrame, rURL, rArgs, rListener);
    }
}

// -----------------------------------------------------------------------
void BibliographyLoader::loadView(const Reference< XFrame > & rFrame, const rtl::OUString& /*rURL*/,
        const Sequence< PropertyValue >& /*rArgs*/,
        const Reference< XLoadEventListener > & rListener)
{
    SolarMutexGuard aGuard;
    //!
    if(!m_pBibMod)
        m_pBibMod = OpenBibModul();

    m_pDatMan = (*m_pBibMod)->createDataManager();
    m_xDatMan = m_pDatMan;
    BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();

    if(aBibDesc.sDataSource.isEmpty())
    {
        DBChangeDialogConfig_Impl aConfig;
        const Sequence<OUString> aSources = aConfig.GetDataSourceNames();
        if(aSources.getLength())
            aBibDesc.sDataSource = aSources.getConstArray()[0];
    }

    Reference< XForm > xForm = m_pDatMan->createDatabaseForm( aBibDesc );

    Reference< awt::XWindow >  aWindow = rFrame->getContainerWindow();
    VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(aWindow);

    Window* pParent = VCLUnoHelper::GetWindow( aWindow );

    BibBookContainer *pMyWindow = new BibBookContainer( pParent );
    pMyWindow->Show();

    ::bib::BibView* pView = new ::bib::BibView( pMyWindow, m_pDatMan, WB_VSCROLL | WB_HSCROLL | WB_3DLOOK );
    pView->Show();
    m_pDatMan->SetView( pView );

    ::bib::BibBeamer* pBeamer = new ::bib::BibBeamer( pMyWindow, m_pDatMan );
    pBeamer->Show();
    pMyWindow->createTopFrame(pBeamer);

    pMyWindow->createBottomFrame(pView);

    Reference< awt::XWindow >  xWin ( pMyWindow->GetComponentInterface(), UNO_QUERY );

    Reference< XController >  xCtrRef( new BibFrameController_Impl( xWin, m_pDatMan ) );

    xCtrRef->attachFrame(rFrame);
    rFrame->setComponent( xWin, xCtrRef);
    pBeamer->SetXController(xCtrRef);
    //!

    // not earlier because SetFocus() is triggered in setVisible()
    pParentComponent->setVisible(sal_True);

    m_xDatMan->load();
    // #100312# ----------
    m_pDatMan->RegisterInterceptor(pBeamer);

    if ( rListener.is() )
        rListener->loadFinished( this );

    // attach menu bar
    Reference< XPropertySet > xPropSet( rFrame, UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            Any a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
            a >>= xLayoutManager;
        }
        catch ( const uno::Exception& )
        {
        }
    }

    if ( xLayoutManager.is() )
        xLayoutManager->createElement( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" )));
}

BibDataManager* BibliographyLoader::GetDataManager()const
{
    if(!m_pDatMan)
    {
        if(!m_pBibMod)
            const_cast< BibliographyLoader* >( this )->m_pBibMod = OpenBibModul();
        const_cast< BibliographyLoader* >( this )->m_pDatMan = (*m_pBibMod)->createDataManager();
        const_cast< BibliographyLoader* >( this )->m_xDatMan = m_pDatMan;
    }
    return m_pDatMan;
}

Reference< XNameAccess >  BibliographyLoader::GetDataColumns() const
{
    if (!m_xColumns.is())
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        Reference< XRowSet >  xRowSet(xMgr->createInstance(C2U("com.sun.star.sdb.RowSet")), UNO_QUERY);
        Reference< XPropertySet >  xResultSetProps(xRowSet, UNO_QUERY);
        DBG_ASSERT(xResultSetProps.is() , "BibliographyLoader::GetDataCursor : invalid row set (no XResultSet or no XPropertySet) !");

        BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();

        Any aBibUrlAny; aBibUrlAny <<= aBibDesc.sDataSource;
        xResultSetProps->setPropertyValue(C2U("DataSourceName"), aBibUrlAny);
        Any aCommandType; aCommandType <<= aBibDesc.nCommandType;
        xResultSetProps->setPropertyValue(C2U("CommandType"), aCommandType);
        Any aTableName; aTableName <<= aBibDesc.sTableOrQuery;
        xResultSetProps->setPropertyValue(C2U("Command"), aTableName);
        Any aResultSetType; aResultSetType <<= (sal_Int32)(ResultSetType::SCROLL_INSENSITIVE);
        xResultSetProps->setPropertyValue(C2U("ResultSetType"), aResultSetType);
        Any aResultSetCurrency; aResultSetCurrency <<= (sal_Int32)(ResultSetConcurrency::UPDATABLE);
        xResultSetProps->setPropertyValue(C2U("ResultSetConcurrency"), aResultSetCurrency);

        sal_Bool bSuccess = sal_False;
        try
        {
            xRowSet->execute();
            bSuccess = sal_True;
        }
        catch(const SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = sal_False;
        }

        if (!bSuccess)
        {
            Reference< XComponent >  xSetComp(xRowSet, UNO_QUERY);
            if (xSetComp.is())
                xSetComp->dispose();
            xRowSet = NULL;
        }
        else
            ((BibliographyLoader*)this)->m_xCursor = xRowSet.get();

        Reference< sdbcx::XColumnsSupplier >  xSupplyCols(m_xCursor, UNO_QUERY);
        if (xSupplyCols.is())
            ((BibliographyLoader*)this)->m_xColumns = xSupplyCols->getColumns();
    }

    return m_xColumns;
}

Reference< sdb::XColumn >  BibliographyLoader::GetIdentifierColumn() const
{
    BibDataManager* pDatMan = GetDataManager();
    Reference< XNameAccess >  xColumns = GetDataColumns();
    rtl::OUString sIdentifierColumnName = pDatMan->GetIdentifierMapping();

    Reference< sdb::XColumn >  xReturn;
    if (xColumns.is() && xColumns->hasByName(sIdentifierColumnName))
    {
        xReturn = Reference< XColumn > (*(Reference< XInterface > *)
                xColumns->getByName(sIdentifierColumnName).getValue(), UNO_QUERY);
    }
    return xReturn;
}

Reference< XResultSet >  BibliographyLoader::GetDataCursor() const
{
    if (!m_xCursor.is())
        GetDataColumns();
    if (m_xCursor.is())
        m_xCursor->first();
    return m_xCursor;
}

rtl::OUString lcl_AddProperty(Reference< XNameAccess >  xColumns,
        const Mapping* pMapping, const String& rColumnName)
{
    String sColumnName(rColumnName);
    if(pMapping)
    {
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            if(pMapping->aColumnPairs[nEntry].sLogicalColumnName == OUString(rColumnName))
            {
                sColumnName = pMapping->aColumnPairs[nEntry].sRealColumnName;
                break;
            }
        }
    }
    rtl::OUString uColumnName(sColumnName);
    rtl::OUString uRet;
    Reference< sdb::XColumn >  xCol;
    if (xColumns->hasByName(uColumnName))
        xCol = Reference< sdb::XColumn > (*(Reference< XInterface > *)xColumns->getByName(uColumnName).getValue(), UNO_QUERY);
    if (xCol.is())
        uRet = xCol->getString();
    return uRet;
}
//-----------------------------------------------------------------------------
Any BibliographyLoader::getByName(const rtl::OUString& rName) throw
                        ( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    try
    {
        BibDataManager* pDatMan = ((BibliographyLoader*)this)->GetDataManager();
        Reference< XResultSet >  xCursor = GetDataCursor();
        Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xCursor, UNO_QUERY);
        Reference< XNameAccess >  xColumns;
        if (!xSupplyCols.is())
            return aRet;
        xColumns = xSupplyCols->getColumns();
        DBG_ASSERT(xSupplyCols.is(), "BibliographyLoader::getByName : invalid columns returned by the data cursor (may be the result set is not alive ?) !");
        if (!xColumns.is())
            return aRet;

        String sIdentifierMapping = pDatMan->GetIdentifierMapping();
        rtl::OUString sId = sIdentifierMapping;
        Reference< sdb::XColumn >  xColumn;
        if (xColumns->hasByName(sId))
            xColumn = Reference< sdb::XColumn > (*(Reference< XInterface > *)xColumns->getByName(sId).getValue(), UNO_QUERY);
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
                        const String sColName = pConfig->GetDefColumnName(
                                                    nEntry);
                        pValues[nEntry].Name = sColName;
                        pValues[nEntry].Value <<= lcl_AddProperty(xColumns, pMapping, sColName);
                    }
                    aRet.setValue(&aPropSequ, ::getCppuType((Sequence<PropertyValue>*)0));

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

Sequence< rtl::OUString > BibliographyLoader::getElementNames(void) throw ( RuntimeException )
{
    Sequence< rtl::OUString > aRet(10);
    int nRealNameCount = 0;
    try
    {
        Reference< XResultSet >  xCursor(GetDataCursor());
        Reference< sdb::XColumn >  xIdColumn(GetIdentifierColumn());
        if (xIdColumn.is()) // implies xCursor.is()
        {
            do
            {
                rtl::OUString sTemp = xIdColumn->getString();
                if (!sTemp.isEmpty() && !xIdColumn->wasNull())
                {
                    int nLen = aRet.getLength();
                    if(nLen == nRealNameCount)
                        aRet.realloc(nLen + 10);
                    rtl::OUString* pArray = aRet.getArray();
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

sal_Bool BibliographyLoader::hasByName(const rtl::OUString& rName) throw ( RuntimeException )
{
    sal_Bool bRet = sal_False;
    try
    {
        Reference< XResultSet >  xCursor = GetDataCursor();
        Reference< sdb::XColumn >  xIdColumn = GetIdentifierColumn();

        if (xIdColumn.is())     // implies xCursor.is()
        {
            do
            {
                rtl::OUString sCurrentId = xIdColumn->getString();
                if (!xIdColumn->wasNull() && (rName.compareTo(sCurrentId) == COMPARE_EQUAL))
                {
                    bRet = sal_True;
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

Type  BibliographyLoader::getElementType(void) throw ( RuntimeException )
{
    return ::getCppuType((Sequence<PropertyValue>*)0);
}

sal_Bool BibliographyLoader::hasElements(void) throw ( RuntimeException )
{
    Reference< XResultSet >  xCursor = GetDataCursor();
    Reference< XNameAccess >  xColumns = GetDataColumns();
    return xColumns.is() && (xColumns->getElementNames().getLength() != 0);
}

Reference< XPropertySetInfo >  BibliographyLoader::getPropertySetInfo(void) throw
                                            ( RuntimeException )
{
    static SfxItemPropertyMapEntry aBibProps_Impl[] =
    {
        { MAP_CHAR_LEN("BibliographyDataFieldNames"), 0, &::getCppuType((Sequence<PropertyValue>*)0), PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}
    };
    static Reference< XPropertySetInfo >  xRet =
        SfxItemPropertySet(aBibProps_Impl).getPropertySetInfo();
    return xRet;
}

void BibliographyLoader::setPropertyValue(const rtl::OUString& /*PropertyName*/,
                                        const Any& /*aValue*/)
    throw( UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw UnknownPropertyException();
    //no changeable properties
}

Any BibliographyLoader::getPropertyValue(const rtl::OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
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
    if(C2U("BibliographyDataFieldNames") == rPropertyName)
    {
        Sequence<PropertyValue> aSeq(COLUMN_COUNT);
        PropertyValue* pArray = aSeq.getArray();
        BibConfig* pConfig = BibModul::GetConfig();
        for(sal_uInt16 i = 0; i <= text::BibliographyDataField::ISBN ; i++)
        {
            pArray[i].Name = pConfig->GetDefColumnName(aInternalMapping[i]);
            pArray[i].Value <<= (sal_Int16) i;
        }
        aRet.setValue(&aSeq, ::getCppuType((Sequence<PropertyValue>*)0));
    }
    else
        throw UnknownPropertyException();
    return aRet;
}

void BibliographyLoader::addPropertyChangeListener(
        const rtl::OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no bound properties
}

void BibliographyLoader::removePropertyChangeListener(
        const rtl::OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no bound properties
}

void BibliographyLoader::addVetoableChangeListener(
    const rtl::OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no vetoable properties
}

void BibliographyLoader::removeVetoableChangeListener(
    const rtl::OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no vetoable properties
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
