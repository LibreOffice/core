/*************************************************************************
 *
 *  $RCSfile: bibload.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2000-11-15 15:54:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>   // helper for factories
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATAFIELD_HPP_
#include <com/sun/star/text/BibliographyDataField.hpp>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen wg. form
#include <vcl/group.hxx>
#endif

#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#ifndef BIB_HRC
#include "bib.hrc"
#endif
#ifndef ADDRCONT_HXX
#include "bibcont.hxx"
#endif
#ifndef ADRBEAM_HXX
#include "bibbeam.hxx"
#endif
#ifndef BIBMOD_HXX
#include "bibmod.hxx"
#endif
#ifndef _BIB_VIEW_HXX
#include "bibview.hxx"
#endif
#ifndef _BIB_FRAMECTR_HXX
#include "framectr.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef _BIBCONFIG_HXX
#include <bibconfig.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx> // helper for implementations
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;

#define C2U(cChar) OUString::createFromAscii(cChar)


//-----------------------------------------------------------------------------

#define PROPERTY_FRAME                      1

class BibliographyLoader : public cppu::WeakImplHelper4
<
    XServiceInfo,
    container::XNameAccess,
    XPropertySet,
    frame::XFrameLoader
>
{
    HdlBibModul                                     m_pBibMod;
    Reference< XPropertyChangeListener >            m_xDatMan;
    BibDataManager*                                 m_pDatMan;
    Reference< container::XNameAccess >             m_xColumns;
    Reference< XResultSet >                         m_xCursor;

private:

    void                    loadView(const Reference< frame::XFrame > & aFrame, const rtl::OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< frame::XLoadEventListener > & aListener);

    BibDataManager*         GetDataManager()const;
    Reference< container::XNameAccess >             GetDataColumns() const;
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
    virtual Any SAL_CALL getByName(const rtl::OUString& aName) throw ( container::NoSuchElementException, WrappedTargetException, RuntimeException );
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
    virtual void            SAL_CALL load(const Reference< frame::XFrame > & aFrame, const rtl::OUString& aURL,
                                const Sequence< PropertyValue >& aArgs,
                                const Reference< frame::XLoadEventListener > & aListener);
    virtual void            SAL_CALL cancel(void);
};

BibliographyLoader::BibliographyLoader() :
    m_pBibMod(0),
    m_pDatMan(0)
{
}

BibliographyLoader::~BibliographyLoader()
{
    if(m_pBibMod)
        CloseBibModul(m_pBibMod);
}


Reference< XInterface >  SAL_CALL BibliographyLoader_CreateInstance( const Reference< XMultiServiceFactory >  & rSMgr ) throw( Exception )
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
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( void * //pServiceManager
                                        , registry::XRegistryKey * pRegistryKey )
    {
        try
        {
            rtl::OUString sKey = rtl::OUString::createFromAscii("/");
            sKey += BibliographyLoader::getImplementationName_Static();
            //creates the impl key:  /com.sun.star.extensions.Bibliography
            Reference< registry::XRegistryKey > xImplKey = pRegistryKey->createKey(sKey);


            //sKey += rtl::OUString::createFromAscii("/UNO/SERVICES");
            Reference< registry::XRegistryKey > xNewKey = xImplKey->createKey(  C2U("/UNO/SERVICES") );
            Sequence< rtl::OUString > aServices = BibliographyLoader::getSupportedServiceNames_Static();
            for( INT32 i = 0; i < aServices.getLength(); i++ )
                xNewKey->createKey( aServices.getConstArray()[i]);
            xNewKey = xImplKey->createKey(  C2U("/UNO/Loader"));

            Reference< registry::XRegistryKey >  xPatternKey = xImplKey->createKey( C2U("/Loader/Pattern" ));
            xPatternKey->setAsciiValue( C2U( ".component:Bibliography/*" ) );

            return sal_True;
        }
        catch (Exception &)
        {
            return sal_False;
        }
    }


    void * SAL_CALL component_getFactory(
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
void BibliographyLoader::cancel(void)
{
    //!
    //!
}
// -----------------------------------------------------------------------
void BibliographyLoader::load(const Reference< frame::XFrame > & rFrame, const rtl::OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< frame::XLoadEventListener > & rListener)
{
    //!

    m_pBibMod = OpenBibModul();

    String aURLStr( rURL );
    String aPartName = aURLStr.GetToken( 1, '/' );

    if(aPartName.EqualsAscii("View") || aPartName.EqualsAscii("View1"))
    {
        loadView(rFrame, rURL, rArgs, rListener);
    }
}

// -----------------------------------------------------------------------
void BibliographyLoader::loadView(const Reference< frame::XFrame > & rFrame, const rtl::OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< frame::XLoadEventListener > & rListener)
{
    //!
    if(!m_pBibMod)
        m_pBibMod = OpenBibModul();

    BibDataManager* pDatMan=(*m_pBibMod)->createDataManager();
    m_xDatMan = m_pDatMan;
    BibDBDescriptor aBibDesc = BibModul::GetConfig()->GetBibliographyURL();

    Reference< form::XForm >  xForm = pDatMan->createDatabaseForm(aBibDesc);

    if(xForm.is())
    {
        Reference< awt::XWindow >  aWindow = rFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(aWindow);
        pParentComponent->setVisible(sal_True);

        Window* pParent = VCLUnoHelper::GetWindow( aWindow );

        BibBookContainer *pMyWindow = new BibBookContainer( pParent, pDatMan );
        pMyWindow->Show();

        BibView* pView = new BibView(pMyWindow,pDatMan,WB_SECTION_STYLE|WB_3DLOOK);
        pView->Show();
        pDatMan->SetView(pView);

        BibBeamer* pBeamer=new BibBeamer( pMyWindow,pDatMan);
        pBeamer->Show();
        pMyWindow->createTopFrame(pBeamer);

        pMyWindow->createBottomFrame(pView);

        Reference< awt::XWindow >  xWin ( pMyWindow->GetComponentInterface(), UNO_QUERY );

        Reference< frame::XController >  xCtrRef(new BibFrameController_Impl( xWin,pDatMan));

        xCtrRef->attachFrame(rFrame);
        rFrame->setComponent( xWin, xCtrRef);
        pBeamer->SetXController(xCtrRef);
        //!

        long nBeamerSize = pDatMan->getBeamerSize();

        pDatMan->loadDatabase();

        ResMgr* pResMgr = (*m_pBibMod)->GetResMgr();
        INetURLObject aEntry( URIHelper::SmartRelToAbs(pResMgr->GetFileName()) );
        String aResFile = String::CreateFromAscii(".component:");
        ( aResFile += aEntry.GetName() ) += '/';
        String aMenuRes = aResFile;
        aMenuRes+=String::CreateFromInt32(RID_MAIN_MENU);

        util::URL aURL;
        aURL.Complete = aMenuRes;

        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        Reference< util::XURLTransformer >  xTrans ( xMgr->createInstance( C2U("com.sun.star.util.URLTransformer") ), UNO_QUERY );
        if( xTrans.is() )
        {
            // Datei laden
            xTrans->parseStrict( aURL );

            Reference< frame::XDispatchProvider >  xProv( rFrame, UNO_QUERY );
            if ( xProv.is() )
            {
                Reference< frame::XDispatch >  aDisp = xProv->queryDispatch( aURL,  C2U("_tool:_menubar"), 12 );
                if ( aDisp.is() )
                    aDisp->dispatch( aURL, Sequence<PropertyValue>() );
            }
        }
        if ( rListener.is() )
            rListener->loadFinished( this );
    }
    else
    {
        if ( rListener.is() )
            rListener->loadCancelled( this );
    }
}
/* -----------------06.12.99 14:37-------------------

 --------------------------------------------------*/
BibDataManager* BibliographyLoader::GetDataManager()const
{
    if(!m_pDatMan)
    {
        if(!m_pBibMod)
            ((BibliographyLoader*)this)->m_pBibMod = OpenBibModul();
        ((BibliographyLoader*)this)->m_pDatMan = (*m_pBibMod)->createDataManager();
        ((BibliographyLoader*)this)->m_xDatMan = m_pDatMan;
    }
    return m_pDatMan;
}
/* -----------------06.12.99 14:39-------------------

 --------------------------------------------------*/
Reference< container::XNameAccess >  BibliographyLoader::GetDataColumns() const
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
        catch(SQLException& e)
        {
#ifdef DBG_UTIL
            // TODO : show a real error message
            String sMsg(String::CreateFromAscii("BibliographyLoader::GetDataCursor : could not execute the result set (catched an SQL-exception"));
            sMsg += String(e.Message);
            sMsg.AppendAscii(") !");
            DBG_ERROR( ByteString(sMsg, RTL_TEXTENCODING_ASCII_US ).GetBuffer());
#endif
        }
        catch(Exception& rEx)
        {
            DBG_ERROR("BibliographyLoader::GetDataCursor : could not execute the result set !");
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
/* -----------------17.12.99 12:29-------------------

 --------------------------------------------------*/
Reference< sdb::XColumn >  BibliographyLoader::GetIdentifierColumn() const
{
    BibDataManager* pDatMan = GetDataManager();
    Reference< container::XNameAccess >  xColumns = GetDataColumns();
    rtl::OUString sIdentifierColumnName = pDatMan->GetIdentifierMapping();

    Reference< sdb::XColumn >  xReturn;
    if (xColumns.is() && xColumns->hasByName(sIdentifierColumnName))
    {
        xReturn = Reference< XColumn > (*(Reference< XInterface > *)
                xColumns->getByName(sIdentifierColumnName).getValue(), UNO_QUERY);
    }
    return xReturn;
}

/* -----------------06.12.99 15:05-------------------

 --------------------------------------------------*/
Reference< XResultSet >  BibliographyLoader::GetDataCursor() const
{
    if (!m_xCursor.is())
        GetDataColumns();
    if (m_xCursor.is())
        m_xCursor->first();
    return m_xCursor;
}

/*-- 17.11.99 12:51:38---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString lcl_AddProperty(Reference< container::XNameAccess >  xColumns,
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
                        ( container::NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    try
    {
        BibDataManager* pDatMan = ((BibliographyLoader*)this)->GetDataManager();
        Reference< XResultSet >  xCursor = GetDataCursor();
        Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xCursor, UNO_QUERY);
        Reference< container::XNameAccess >  xColumns;
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
    catch(Exception& rEx)
    {
        DBG_ERROR("Exception in BibliographyLoader::getByName")
    }
    return aRet;
}
/*-- 17.11.99 12:51:39---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< rtl::OUString > BibliographyLoader::getElementNames(void) throw ( RuntimeException )
{
    Sequence< rtl::OUString > aRet(10);
    int nRealNameCount = 0;
    try
    {
        BibDataManager* pDatMan = ((BibliographyLoader*)this)->GetDataManager();

        Reference< XResultSet >  xCursor(GetDataCursor());
        Reference< sdb::XColumn >  xIdColumn(GetIdentifierColumn());
        if (xIdColumn.is()) // implies xCursor.is()
        {
            do
            {
                rtl::OUString sTemp = xIdColumn->getString();
                if (sTemp.len() && !xIdColumn->wasNull())
                {
                    int nLen = aRet.getLength();
                    if (nLen = nRealNameCount)
                        aRet.realloc(nLen + 10);
                    rtl::OUString* pArray = aRet.getArray();
                    pArray[nRealNameCount] = sTemp;
                    nRealNameCount++;
                }
            }
            while (xCursor->next());
        }
    }
    catch(Exception& rEx)
    {
        DBG_ERROR("Exception in BibliographyLoader::getElementNames")
    }

    aRet.realloc(nRealNameCount);
    return aRet;
}
/*-- 17.11.99 12:51:39---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool BibliographyLoader::hasByName(const rtl::OUString& rName) throw ( RuntimeException )
{
    sal_Bool bRet = sal_False;
    try
    {
        BibDataManager* pDatMan = GetDataManager();
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
    catch(Exception& rEx)
    {
        DBG_ERROR("Exception in BibliographyLoader::getElementNames")
    }
    return bRet;
}
/*-- 17.11.99 12:51:39---------------------------------------------------

  -----------------------------------------------------------------------*/
Type  BibliographyLoader::getElementType(void) throw ( RuntimeException )
{
    return ::getCppuType((Sequence<PropertyValue>*)0);
}
/*-- 17.11.99 12:51:40---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool BibliographyLoader::hasElements(void) throw ( RuntimeException )
{
    Reference< XResultSet >  xCursor = GetDataCursor();
    Reference< container::XNameAccess >  xColumns = GetDataColumns();
    return xColumns.is() && (xColumns->getElementNames().getLength() != 0);
}

/*-- 07.12.99 14:28:39---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  BibliographyLoader::getPropertySetInfo(void) throw
                                            ( RuntimeException )
{
    static SfxItemPropertyMap aBibProps_Impl[] =
    {
        { MAP_CHAR_LEN("BibliographyDataFieldNames"), 0, &::getCppuType((Sequence<PropertyValue>*)0), PropertyAttribute::READONLY, 0},
        {0,0,0,0}
    };
    static Reference< XPropertySetInfo >  xRet =
        SfxItemPropertySet(aBibProps_Impl).getPropertySetInfo();
    return xRet;
}
/*-- 07.12.99 14:28:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibliographyLoader::setPropertyValue(const rtl::OUString& PropertyName,
                                        const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
         IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw UnknownPropertyException();
    //no changeable properties
}
/*-- 07.12.99 14:28:39---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        BibDataManager* pDatMan = GetDataManager();
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
/*-- 07.12.99 14:28:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibliographyLoader::addPropertyChangeListener(
        const rtl::OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no bound properties
}
/*-- 07.12.99 14:28:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibliographyLoader::removePropertyChangeListener(
        const rtl::OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no bound properties
}
/*-- 07.12.99 14:28:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibliographyLoader::addVetoableChangeListener(
    const rtl::OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no vetoable properties
}
/*-- 07.12.99 14:28:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibliographyLoader::removeVetoableChangeListener(
    const rtl::OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    //no vetoable properties
}



