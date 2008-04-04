/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodatbr.cxx,v $
 *
 *  $Revision: 1.197 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:00:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "browserids.hxx"
#include "dbaccess_helpid.hrc"
#include "dbexchange.hxx"
#include "dbtreelistbox.hxx"
#include "dbtreemodel.hxx"
#include "dbtreeview.hxx"
#include "dbu_brw.hrc"
#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "dlgsave.hxx"
#include "HtmlReader.hxx"
#include "imageprovider.hxx"
#include "linkeddocuments.hxx"
#include "listviewitems.hxx"
#include "QEnumTypes.hxx"
#include "RtfReader.hxx"
#include "sbagrid.hrc"
#include "sbagrid.hxx"
#include "sqlmessage.hxx"
#include "TokenWriter.hxx"
#include "UITools.hxx"
#include "unodatbr.hxx"
#include "WColumnSelect.hxx"
#include "WCopyTable.hxx"
#include "WCPage.hxx"
#include "WExtendPages.hxx"
#include "WNameMatch.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
/** === end UNO includes === **/

#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sot/storage.hxx>
#include <svtools/filenotation.hxx>
#include <svtools/intitem.hxx>
#include <svtools/moduleoptions.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <svx/algitem.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/databaseregistrationui.hxx>
#include <svx/gridctrl.hxx>
#include <toolkit/unohlp.hxx>
#include <tools/diagnose_ex.h>
#include <tools/multisel.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/split.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/wrkwin.hxx>

#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::document;
using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::svx;

using ::com::sun::star::frame::XLayoutManager;      // obsolete of interface is moved outside drafts
using ::com::sun::star::ui::XUIElement;             // dito

// .........................................................................
namespace dbaui
{
// .........................................................................

//==================================================================
//= SbaTableQueryBrowser
//==================================================================
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OBrowser()
{
    static OMultiInstanceAutoRegistration< SbaTableQueryBrowser > aAutoRegistration;
}
// -------------------------------------------------------------------------
void SafeAddPropertyListener(const Reference< XPropertySet > & xSet, const ::rtl::OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->addPropertyChangeListener(rPropName, pListener);
}

// -------------------------------------------------------------------------
void SafeRemovePropertyListener(const Reference< XPropertySet > & xSet, const ::rtl::OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->removePropertyChangeListener(rPropName, pListener);
}
//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaTableQueryBrowser::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL SbaTableQueryBrowser::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
::rtl::OUString SbaTableQueryBrowser::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.ODatasourceBrowser");
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SbaTableQueryBrowser::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DataSourceBrowser");
    return aSupported;
}
//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SbaTableQueryBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    return *(new SbaTableQueryBrowser(_rxFactory));
}

DBG_NAME(SbaTableQueryBrowser);
//------------------------------------------------------------------------------
SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aSelectionListeners(m_aMutex)
    ,m_aTableCopyHelper(this)
    ,m_pTreeView(NULL)
    ,m_pSplitter(NULL)
    ,m_pTreeModel(NULL)
    ,m_pCurrentlyDisplayed(NULL)
    ,m_nAsyncDrop(0)
    ,m_nBorder(1)
    ,m_bQueryEscapeProcessing( sal_False )
    ,m_bShowMenu(sal_False)
    ,m_bInSuspend(sal_False)
    ,m_bEnableBrowser(sal_True)
{
    DBG_CTOR(SbaTableQueryBrowser,NULL);
}

//------------------------------------------------------------------------------
SbaTableQueryBrowser::~SbaTableQueryBrowser()
{
    DBG_DTOR(SbaTableQueryBrowser,NULL);
    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        OSL_ENSURE(0,"Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

//------------------------------------------------------------------------------
Any SAL_CALL SbaTableQueryBrowser::queryInterface(const Type& _rType) throw (RuntimeException)
{
    if ( _rType.equals( XScriptInvocationContext::static_type() ) )
    {
        OSL_PRECOND( !!m_aDocScriptSupport, "SbaTableQueryBrowser::queryInterface: did not initialize this, yet!" );
        if ( !!m_aDocScriptSupport && *m_aDocScriptSupport )
            return makeAny( Reference< XScriptInvocationContext >( this ) );
        return Any();
    }

    Any aReturn = SbaXDataBrowserController::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = SbaTableQueryBrowser_Base::queryInterface(_rType);
    return aReturn;
}

//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL SbaTableQueryBrowser::getTypes(  ) throw (RuntimeException)
{
    Sequence< Type > aTypes( ::comphelper::concatSequences(
        SbaXDataBrowserController::getTypes(),
        SbaTableQueryBrowser_Base::getTypes()
    ) );

    OSL_PRECOND( !!m_aDocScriptSupport, "SbaTableQueryBrowser::getTypes: did not initialize this, yet!" );
    if ( !m_aDocScriptSupport || !*m_aDocScriptSupport )
    {
        Sequence< Type > aStrippedTypes( aTypes.getLength() - 1 );
        ::std::remove_copy_if(
            aTypes.getConstArray(),
            aTypes.getConstArray() + aTypes.getLength(),
            aStrippedTypes.getArray(),
            ::std::bind2nd( ::std::equal_to< Type >(), XScriptInvocationContext::static_type() )
        );
        aTypes = aStrippedTypes;
    }
    return aTypes;
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL SbaTableQueryBrowser::getImplementationId(  ) throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//------------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::disposing()
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // kiss our listeners goodbye
    EventObject aEvt(*this);
    m_aSelectionListeners.disposeAndClear(aEvt);

    // reset the content's tree view: it holds a reference to our model which is to be deleted immediately,
    // and it will live longer than we do.
    if (getBrowserView())
        getBrowserView()->setTreeView(NULL);

    clearTreeModel();
    // clear the tree model
    {
        ::std::auto_ptr<DBTreeListModel> aTemp(m_pTreeModel);
        m_pTreeModel = NULL;
    }

    // remove ourself as status listener
    implRemoveStatusListeners();

    // remove the container listener from the database context
    Reference< XContainer > xDatasourceContainer(m_xDatabaseContext, UNO_QUERY);
    if (xDatasourceContainer.is())
        xDatasourceContainer->removeContainerListener(this);

    // check out from all the objects we are listening
    // the frame
    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
    SbaXDataBrowserController::disposing();
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::Construct(Window* pParent)
{
    if (!SbaXDataBrowserController::Construct(pParent))
        return sal_False;

    try
    {
        Reference< XContainer > xDatasourceContainer(m_xDatabaseContext, UNO_QUERY);
        if (xDatasourceContainer.is())
            xDatasourceContainer->addContainerListener(this);
        else
            DBG_ERROR("SbaTableQueryBrowser::Construct: the DatabaseContext should allow us to be a listener!");

        // the collator for the string compares
        m_xCollator = Reference< XCollator >(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.Collator")), UNO_QUERY);
        if (m_xCollator.is())
            m_xCollator->loadDefaultCollator(Application::GetSettings().GetLocale(), 0);
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::Construct: could not create (or start listening at) the database context!");
    }
    // some help ids
    if (getBrowserView() && getBrowserView()->getVclControl())
    {

        // create controls and set sizes
        const long  nFrameWidth = getBrowserView()->LogicToPixel( ::Size( 3, 0 ), MAP_APPFONT ).Width();

        m_pSplitter = new Splitter(getBrowserView(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( ::Point(0,0), ::Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );

        m_pTreeView = new DBTreeView(getBrowserView(),getORB(), WB_TABSTOP | WB_BORDER);
        m_pTreeView->SetPreExpandHandler(LINK(this, SbaTableQueryBrowser, OnExpandEntry));

        m_pTreeView->setCopyHandler(LINK(this, SbaTableQueryBrowser, OnCopyEntry));

        m_pTreeView->getListBox()->setControlActionListener(this);
        m_pTreeView->SetHelpId(HID_CTL_TREEVIEW);

        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel( getBrowserView()->LogicToPixel( ::Size( 80, 0 ), MAP_APPFONT ).Width() );

        getBrowserView()->setSplitter(m_pSplitter);
        getBrowserView()->setTreeView(m_pTreeView);

        // fill view with data
        m_pTreeModel = new DBTreeListModel;
        m_pTreeModel->SetSortMode(SortAscending);
        m_pTreeModel->SetCompareHdl(LINK(this, SbaTableQueryBrowser, OnTreeEntryCompare));
        m_pTreeView->setModel(m_pTreeModel);
        m_pTreeView->setSelectHdl(LINK(this, SbaTableQueryBrowser, OnSelectEntry));

        // TODO
        getBrowserView()->getVclControl()->GetDataWindow().SetUniqueId(UID_DATABROWSE_DATAWINDOW);
        getBrowserView()->getVclControl()->SetHelpId(HID_CTL_TABBROWSER);
        getBrowserView()->SetUniqueId(UID_CTL_CONTENT);
        if (getBrowserView()->getVclControl()->GetHeaderBar())
            getBrowserView()->getVclControl()->GetHeaderBar()->SetHelpId(HID_DATABROWSE_HEADER);
        InvalidateFeature(ID_BROWSER_EXPLORER);
    }

    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::InitializeForm(const Reference< ::com::sun::star::sdbc::XRowSet > & _rxForm)
{
    if(!m_pCurrentlyDisplayed)
        return sal_True;

    // this method set all format settings from the orignal table or query
    try
    {
        // we send all properties at once, maybe the implementation is clever enough to handle one big PropertiesChanged
        // more effective than many small PropertyChanged ;)
        Sequence< ::rtl::OUString> aProperties(3);
        Sequence< Any> aValues(3);

        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        OSL_ENSURE( pData, "SbaTableQueryBrowser::InitializeForm: No user data set at the currently displayed entry!" );
        OSL_ENSURE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeForm: No table available!" );

        if ( pData->xObjectProperties.is() )
        {
            sal_Int32 nPos = 0;
            // is the filter intially applied ?
            aProperties.getArray()[nPos]    = PROPERTY_APPLYFILTER;
            aValues.getArray()[nPos++]      = pData->xObjectProperties->getPropertyValue(PROPERTY_APPLYFILTER);

            // the initial filter
            aProperties.getArray()[nPos]    = PROPERTY_FILTER;
            aValues.getArray()[nPos++]      = pData->xObjectProperties->getPropertyValue(PROPERTY_FILTER);

            if ( pData->xObjectProperties->getPropertySetInfo()->hasPropertyByName(PROPERTY_HAVING_CLAUSE) )
            {
                aProperties.realloc(aProperties.getLength()+1);
                aValues.realloc(aValues.getLength()+1);
                // the initial having clause
                aProperties.getArray()[nPos]    = PROPERTY_HAVING_CLAUSE;
                aValues.getArray()[nPos++]      = pData->xObjectProperties->getPropertyValue(PROPERTY_HAVING_CLAUSE);
            }

            // the initial ordering
            aProperties.getArray()[nPos]    = PROPERTY_ORDER;
            aValues.getArray()[nPos++]      = pData->xObjectProperties->getPropertyValue(PROPERTY_ORDER);

            Reference< XMultiPropertySet >  xFormMultiSet(_rxForm, UNO_QUERY);
            xFormMultiSet->setPropertyValues(aProperties, aValues);
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::InitializeForm : something went wrong !");
        return sal_False;
    }


    return sal_True;
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::initializePreviewMode()
{
    if ( getBrowserView() && getBrowserView()->getVclControl() )
    {
        getBrowserView()->getVclControl()->AlwaysEnableInput( FALSE );
        getBrowserView()->getVclControl()->EnableInput( FALSE );
        getBrowserView()->getVclControl()->ForceHideScrollbars( sal_True );
    }
    Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
    if ( xDataSourceSet.is() )
    {
        xDataSourceSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowInserts")),makeAny(sal_False));
        xDataSourceSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowUpdates")),makeAny(sal_False));
        xDataSourceSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowDeletes")),makeAny(sal_False));
    }
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::InitializeGridModel(const Reference< ::com::sun::star::form::XFormComponent > & xGrid)
{
    try
    {
        Reference< ::com::sun::star::form::XGridColumnFactory >  xColFactory(xGrid, UNO_QUERY);
        Reference< XNameContainer >  xColContainer(xGrid, UNO_QUERY);
        clearGridColumns( xColContainer );

        Reference< XChild > xGridAsChild(xGrid, UNO_QUERY);
        Reference< XLoadable > xFormAsLoadable;
        if (xGridAsChild.is())
            xFormAsLoadable = xFormAsLoadable.query(xGridAsChild->getParent());
        if (xFormAsLoadable.is() && xFormAsLoadable->isLoaded())
        {
            // set the formats from the table
            if(m_pCurrentlyDisplayed)
            {
                Sequence< ::rtl::OUString> aProperties(6 + ( m_bPreview ? 5 : 0 ));
                Sequence< Any> aValues(7 + ( m_bPreview ? 5 : 0 ));

                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                OSL_ENSURE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeGridModel: No table available!" );

                ::rtl::OUString* pStringIter = aProperties.getArray();
                Any* pValueIter = aValues.getArray();
                if ( m_bPreview )
                {
                    *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AlwaysShowCursor"));
                    *pValueIter++   <<= sal_False;
                    *pStringIter++  = PROPERTY_BORDER;
                    *pValueIter++   <<= sal_Int16(0);
                }

                *pStringIter++  = PROPERTY_FONT;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_FONT);
                *pStringIter++  = PROPERTY_TEXTEMPHASIS;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTEMPHASIS);
                *pStringIter++  = PROPERTY_TEXTRELIEF;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTRELIEF);
                if ( m_bPreview )
                {
                    *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasNavigationBar"));
                    *pValueIter++       <<= sal_False;
                    *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasRecordMarker"));
                    *pValueIter++       <<= sal_False;
                }
                *pStringIter++  = PROPERTY_ROW_HEIGHT;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_ROW_HEIGHT);
                if ( m_bPreview )
                {
                    *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tabstop"));
                    *pValueIter++       <<= sal_False;
                }
                *pStringIter++  = PROPERTY_TEXTCOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTCOLOR);
                *pStringIter++  = PROPERTY_TEXTLINECOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTLINECOLOR);

                Reference< XMultiPropertySet >  xFormMultiSet(xGrid, UNO_QUERY);
                xFormMultiSet->setPropertyValues(aProperties, aValues);
            }


            // get the formats supplier of the database we're working with
            Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = getNumberFormatter()->getNumberFormatsSupplier();

            Reference<XConnection> xConnection;
            Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
            xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            OSL_ENSURE(xConnection.is(),"A ActiveConnection should normaly exists!");

            Reference<XChild> xChild(xConnection,UNO_QUERY);
            Reference<XPropertySet> xDataSourceProp(xChild->getParent(),UNO_QUERY);
            sal_Bool bSupress = ::cppu::any2bool(xDataSourceProp->getPropertyValue(PROPERTY_SUPPRESSVERSIONCL));

            // insert the column into the gridcontrol so that we see something :-)
            ::rtl::OUString aCurrentModelType;
            Reference<XColumnsSupplier> xSupCols(getRowSet(),UNO_QUERY);
            Reference<XNameAccess> xColumns     = xSupCols->getColumns();
            Sequence< ::rtl::OUString> aNames   = xColumns->getElementNames();
            const ::rtl::OUString* pIter        = aNames.getConstArray();
            const ::rtl::OUString* pEnd         = pIter + aNames.getLength();

            ::rtl::OUString sPropertyName;
            Reference<XPropertySet> xColumn;
            for (sal_uInt16 i=0; pIter != pEnd; ++i,++pIter)
            {
                // Typ
                // first get type to determine wich control we need
                ::cppu::extractInterface(xColumn,xColumns->getByName(*pIter));

                // ignore the column when it is a rowversion one
                if(bSupress && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISROWVERSION)
                            && ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISROWVERSION)))
                    continue;

                sal_Bool bFormattedIsNumeric    = sal_True;
                sal_Int32 nType = comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_TYPE));

                ::std::vector< NamedValue > aInitialValues;
                ::std::vector< ::rtl::OUString > aCopyProperties;

                switch(nType)
                {
                    case DataType::BIT:
                    case DataType::BOOLEAN:
                        aCurrentModelType = ::rtl::OUString::createFromAscii("CheckBox");
                        aInitialValues.push_back( NamedValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VisualEffect" ) ), makeAny( VisualEffect::FLAT ) ) );
                        sPropertyName = PROPERTY_DEFAULTSTATE;
                        break;

                    case DataType::LONGVARCHAR:
                        aInitialValues.push_back( NamedValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiLine" ) ), makeAny( (sal_Bool)sal_True ) ) );
                        // NO break!
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                    case DataType::LONGVARBINARY:
                        aCurrentModelType = ::rtl::OUString::createFromAscii("TextField");
                        sPropertyName = PROPERTY_DEFAULTTEXT;
                        break;

                    case DataType::VARCHAR:
                    case DataType::CHAR:
                        bFormattedIsNumeric = sal_False;
                        // NO break!
                    default:
                        aCurrentModelType = ::rtl::OUString::createFromAscii("FormattedField");
                        sPropertyName = PROPERTY_EFFECTIVEDEFAULT;

                        if ( xSupplier.is() )
                            aInitialValues.push_back( NamedValue( ::rtl::OUString::createFromAscii( "FormatsSupplier" ), makeAny( xSupplier ) ) );
                        aInitialValues.push_back( NamedValue( ::rtl::OUString::createFromAscii( "TreatAsNumber" ), makeAny( (sal_Bool)bFormattedIsNumeric ) ) );
                        aCopyProperties.push_back( PROPERTY_FORMATKEY );
                        break;
                }

                aInitialValues.push_back( NamedValue( PROPERTY_CONTROLSOURCE, makeAny( *pIter ) ) );
                aInitialValues.push_back( NamedValue( PROPERTY_LABEL, makeAny( *pIter ) ) );

                Reference< XPropertySet > xCurrentCol = xColFactory->createColumn(aCurrentModelType);
                Reference< XPropertySetInfo > xColumnPropertyInfo = xCurrentCol->getPropertySetInfo();

                // calculate the default
                Any aDefault;
                sal_Bool bDefault = xColumnPropertyInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT);
                if ( bDefault )
                    aDefault = xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT);

                // default value
                if ( nType == DataType::BIT || nType == DataType::BOOLEAN )
                {
                    if(bDefault && aDefault.hasValue())
                        aDefault <<= (comphelper::getString(aDefault).toInt32() == 0) ? (sal_Int16)STATE_NOCHECK : (sal_Int16)STATE_CHECK;
                    else
                        aDefault <<= ((sal_Int16)STATE_DONTKNOW);

                }

                if ( aDefault.hasValue() )
                    aInitialValues.push_back( NamedValue( sPropertyName, aDefault ) );

                // transfer properties from the definition to the UNO-model :
                aCopyProperties.push_back( PROPERTY_HIDDEN );
                aCopyProperties.push_back( PROPERTY_WIDTH );

                Any aDescription; aDescription <<= ::rtl::OUString();
                if ( xColumn->getPropertySetInfo()->hasPropertyByName( PROPERTY_HELPTEXT ) )
                    aDescription <<= ::comphelper::getString( xColumn->getPropertyValue( PROPERTY_HELPTEXT ) );
                aInitialValues.push_back( NamedValue( PROPERTY_HELPTEXT, aDescription ) );

                // ... horizontal justify
                aInitialValues.push_back( NamedValue( PROPERTY_ALIGN, makeAny( sal_Int16( ::comphelper::getINT32( xColumn->getPropertyValue( PROPERTY_ALIGN ) ) ) ) ) );

                // now set all those values
                for ( ::std::vector< NamedValue >::const_iterator property = aInitialValues.begin();
                      property != aInitialValues.end();
                      ++property
                    )
                {
                    xCurrentCol->setPropertyValue( property->Name, property->Value );
                }
                for ( ::std::vector< ::rtl::OUString >::const_iterator copyPropertyName = aCopyProperties.begin();
                      copyPropertyName != aCopyProperties.end();
                      ++copyPropertyName
                    )
                    xCurrentCol->setPropertyValue( *copyPropertyName, xColumn->getPropertyValue( *copyPropertyName ) );

                xColContainer->insertByName(*pIter, makeAny(xCurrentCol));
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::InitializeGridModel : something went wrong !");
        return sal_False;
    }


    return sal_True;
}
// -----------------------------------------------------------------------------
Reference<XPropertySet> getColumnHelper(SvLBoxEntry* _pCurrentlyDisplayed,const Reference<XPropertySet>& _rxSource)
{
    Reference<XPropertySet> xRet;
    if(_pCurrentlyDisplayed)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pCurrentlyDisplayed->GetUserData());
        Reference<XColumnsSupplier> xColumnsSup(pData->xObjectProperties,UNO_QUERY);
        Reference<XNameAccess> xNames = xColumnsSup->getColumns();
        ::rtl::OUString aName;
        _rxSource->getPropertyValue(PROPERTY_NAME) >>= aName;
        if(xNames.is() && xNames->hasByName(aName))
            ::cppu::extractInterface(xRet,xNames->getByName(aName));
    }
    return xRet;
}

// -----------------------------------------------------------------------
void SbaTableQueryBrowser::transferChangedControlProperty(const ::rtl::OUString& _rProperty, const Any& _rNewValue)
{
    if(m_pCurrentlyDisplayed)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        Reference< XPropertySet > xObjectProps(pData->xObjectProperties, UNO_QUERY);
        OSL_ENSURE(xObjectProps.is(),"SbaTableQueryBrowser::transferChangedControlProperty: no table/query object!");
        if (xObjectProps.is())
            xObjectProps->setPropertyValue(_rProperty, _rNewValue);
    }
}

// -----------------------------------------------------------------------
void SbaTableQueryBrowser::propertyChange(const PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    SbaXDataBrowserController::propertyChange(evt);

    try
    {
        Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
        if (!xSource.is())
            return;

        // one of the many properties which require us to update the definition ?
        // a column's width ?
        else if (evt.PropertyName.equals(PROPERTY_WIDTH))
        {   // a column width has changed -> update the model
            // (the update of the view is done elsewhere)
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
            {
                if(!evt.NewValue.hasValue())
                    xProp->setPropertyValue(PROPERTY_WIDTH,makeAny((sal_Int32)227));
                else
                    xProp->setPropertyValue(PROPERTY_WIDTH,evt.NewValue);
            }
        }

        // a column's 'visible' state ?
        else if (evt.PropertyName.equals(PROPERTY_HIDDEN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_HIDDEN,evt.NewValue);
        }

        // a columns alignment ?
        else if (evt.PropertyName.equals(PROPERTY_ALIGN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            try
            {
                if(xProp.is())
                {
                    if(evt.NewValue.hasValue())
                    {
                        sal_Int16 nAlign = 0;
                        if(evt.NewValue >>= nAlign)
                            xProp->setPropertyValue(PROPERTY_ALIGN,makeAny(sal_Int32(nAlign)));
                        else
                            xProp->setPropertyValue(PROPERTY_ALIGN,evt.NewValue);
                    }
                    else
                        xProp->setPropertyValue(PROPERTY_ALIGN,makeAny(::com::sun::star::awt::TextAlign::LEFT));
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser::propertyChange: caught an exception!");
            }
        }

        // a column's format ?
        else if (   (evt.PropertyName.equals(PROPERTY_FORMATKEY))
            &&  (TypeClass_LONG == evt.NewValue.getValueTypeClass())
            )
        {
            // update the model (means the definition object)
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_FORMATKEY,evt.NewValue);
        }

        // some table definition properties ?
        // the height of the rows in the grid ?
        else if (evt.PropertyName.equals(PROPERTY_ROW_HEIGHT))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                OSL_ENSURE( pData->xObjectProperties.is(), "No table available!" );

                sal_Bool bDefault = !evt.NewValue.hasValue();
                if (bDefault)
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,makeAny((sal_Int32)45));
                else
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,evt.NewValue);
            }
        }

        else if (   evt.PropertyName.equals(PROPERTY_FONT)          // the font ?
                ||  evt.PropertyName.equals(PROPERTY_TEXTCOLOR)     // the text color ?
                ||  evt.PropertyName.equals(PROPERTY_FILTER)        // the filter ?
                ||  evt.PropertyName.equals(PROPERTY_HAVING_CLAUSE) // the having clause ?
                ||  evt.PropertyName.equals(PROPERTY_ORDER)         // the sort ?
                ||  evt.PropertyName.equals(PROPERTY_APPLYFILTER)   // the appliance of the filter ?
                ||  evt.PropertyName.equals(PROPERTY_TEXTLINECOLOR) // the text line color ?
                ||  evt.PropertyName.equals(PROPERTY_TEXTEMPHASIS)  // the text emphasis ?
                ||  evt.PropertyName.equals(PROPERTY_TEXTRELIEF)    // the text relief ?
                )
        {
            transferChangedControlProperty(evt.PropertyName, evt.NewValue);
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::propertyChange: caught an exception!");
    }
}

// -----------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( getView() && getView()->IsInModalMode() )
        return sal_False;
    sal_Bool bRet = sal_False;
    if ( !m_bInSuspend )
    {
        m_bInSuspend = sal_True;
        if ( rBHelper.bDisposed )
            throw DisposedException( ::rtl::OUString(), *this );

        bRet = SbaXDataBrowserController::suspend(bSuspend);

        m_bInSuspend = sal_False;
    }

    return bRet;
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::statusChanged( const FeatureStateEvent& _rEvent ) throw(RuntimeException)
{
    // search the external dispatcher causing this call
    Reference< XDispatch > xSource(_rEvent.Source, UNO_QUERY);
    ExternalFeaturesMap::iterator aLoop;
    for ( aLoop = m_aExternalFeatures.begin();
          aLoop != m_aExternalFeatures.end();
          ++aLoop
        )
    {
        if ( _rEvent.FeatureURL.Complete == aLoop->second.aURL.Complete)
        {
            DBG_ASSERT( xSource.get() == aLoop->second.xDispatcher.get(), "SbaTableQueryBrowser::statusChanged: inconsistent!" );
            // update the enabled state
            aLoop->second.bEnabled = _rEvent.IsEnabled;

            switch ( aLoop->first )
            {
                case ID_BROWSER_DOCUMENT_DATASOURCE:
                {
                    // if it's the slot for the document data source, remember the state
                    Sequence< PropertyValue > aDescriptor;
    #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bProperFormat =
    #endif
                    _rEvent.State >>= aDescriptor;
                    OSL_ENSURE(bProperFormat, "SbaTableQueryBrowser::statusChanged: need a data access descriptor here!");
                    m_aDocumentDataSource.initializeFrom(aDescriptor);

                    OSL_ENSURE( (   m_aDocumentDataSource.has(daDataSource)
                                ||  m_aDocumentDataSource.has(daDatabaseLocation)
                                )
                                &&  m_aDocumentDataSource.has(daCommand)
                                &&  m_aDocumentDataSource.has(daCommandType),
                        "SbaTableQueryBrowser::statusChanged: incomplete descriptor!");

                    // check if we know the object which is set as document data source
                    checkDocumentDataSource();
                }
                break;

                default:
                    // update the toolbox
                    implCheckExternalSlot( aLoop->first );
                    break;
            }
            break;
        }
    }

    DBG_ASSERT(aLoop != m_aExternalFeatures.end(), "SbaTableQueryBrowser::statusChanged: don't know who sent this!");
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::checkDocumentDataSource()
{
    SvLBoxEntry* pDataSourceEntry = NULL;
    SvLBoxEntry* pContainerEntry = NULL;
    SvLBoxEntry* pObjectEntry = getObjectEntry( m_aDocumentDataSource, &pDataSourceEntry, &pContainerEntry, sal_False );
    sal_Bool bKnownDocDataSource = (NULL != pObjectEntry);
    if (!bKnownDocDataSource)
    {
        if (NULL != pDataSourceEntry)
        {   // at least the data source is know
            if (NULL != pContainerEntry)
                bKnownDocDataSource = sal_True; // assume we know it.
                // TODO: should we expand the object container? This may be too expensive just for checking ....
            else
            {
                if ((NULL == pObjectEntry) && m_aDocumentDataSource.has(daCommandType) && m_aDocumentDataSource.has(daCommand))
                {   // maybe we have a command to be displayed ?
                    sal_Int32 nCommandType = CommandType::TABLE;
                    m_aDocumentDataSource[daCommandType] >>= nCommandType;

                    ::rtl::OUString sCommand;
                    m_aDocumentDataSource[daCommand] >>= sCommand;

                    bKnownDocDataSource = (CommandType::COMMAND == nCommandType) && (0 != sCommand.getLength());
                }
            }
        }
    }

    if ( !bKnownDocDataSource )
        m_aExternalFeatures[ ID_BROWSER_DOCUMENT_DATASOURCE ].bEnabled = sal_False;

    // update the toolbox
    implCheckExternalSlot(ID_BROWSER_DOCUMENT_DATASOURCE);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::extractDescriptorProps(const ::svx::ODataAccessDescriptor& _rDescriptor, ::rtl::OUString& _rDataSource, ::rtl::OUString& _rCommand, sal_Int32& _rCommandType, sal_Bool& _rEscapeProcessing)
{
    _rDataSource = _rDescriptor.getDataSource();
    if ( _rDescriptor.has(daCommand) )
        _rDescriptor[daCommand] >>= _rCommand;
    if ( _rDescriptor.has(daCommandType) )
        _rDescriptor[daCommandType] >>= _rCommandType;

    // escape processing is the only one allowed not to be present
    _rEscapeProcessing = sal_True;
    if (_rDescriptor.has(daEscapeProcessing))
        _rEscapeProcessing = ::cppu::any2bool(_rDescriptor[daEscapeProcessing]);
}

// -------------------------------------------------------------------------
namespace
{
    bool getDataSourceDisplayName_isURL( const String& _rDS, String& _rDisplayName, String& _rUniqueId )
    {
        INetURLObject aURL( _rDS );
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        {
            _rDisplayName = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_WITH_CHARSET);
            //  _rDisplayName = aURL.getName(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_WITH_CHARSET);
            _rUniqueId = aURL.GetMainURL( INetURLObject::NO_DECODE );
            return true;
        }
        _rDisplayName = _rDS;
        _rUniqueId = String();
        return false;
    }

    // .....................................................................
    struct FilterByEntryDataId : public IEntryFilter
    {
        String sId;
        FilterByEntryDataId( const String& _rId ) : sId( _rId ) { }

        virtual ~FilterByEntryDataId() {}

        virtual bool    includeEntry( SvLBoxEntry* _pEntry ) const;
    };

    bool FilterByEntryDataId::includeEntry( SvLBoxEntry* _pEntry ) const
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast< DBTreeListModel::DBTreeListUserData* >( _pEntry->GetUserData() );
        return ( !pData || ( pData->sAccessor == sId ) );
    }
}

// -------------------------------------------------------------------------
String SbaTableQueryBrowser::getDataSourceAcessor( SvLBoxEntry* _pDataSourceEntry ) const
{
    DBG_ASSERT( _pDataSourceEntry, "SbaTableQueryBrowser::getDataSourceAcessor: invalid entry!" );

    DBTreeListModel::DBTreeListUserData* pData = static_cast< DBTreeListModel::DBTreeListUserData* >( _pDataSourceEntry->GetUserData() );
    DBG_ASSERT( pData, "SbaTableQueryBrowser::getDataSourceAcessor: invalid entry data!" );
    DBG_ASSERT( pData->eType == etDatasource, "SbaTableQueryBrowser::getDataSourceAcessor: entry does not denote a data source!" );
    return pData->sAccessor.Len() ? pData->sAccessor : GetEntryText( _pDataSourceEntry );
}

// -------------------------------------------------------------------------
SvLBoxEntry* SbaTableQueryBrowser::getObjectEntry(const ::rtl::OUString& _rDataSource, const ::rtl::OUString& _rCommand, sal_Int32 _nCommandType,
        SvLBoxEntry** _ppDataSourceEntry, SvLBoxEntry** _ppContainerEntry, sal_Bool _bExpandAncestors,
        const SharedConnection& _rxConnection )
{
    if (_ppDataSourceEntry)
        *_ppDataSourceEntry = NULL;
    if (_ppContainerEntry)
        *_ppContainerEntry = NULL;

    SvLBoxEntry* pObject = NULL;
    if (m_pTreeView && m_pTreeView->getListBox())
    {
        // look for the data source entry
        String sDisplayName, sDataSourceId;
        bool bIsDataSourceURL = getDataSourceDisplayName_isURL( _rDataSource, sDisplayName, sDataSourceId );
            // the display name may differ from the URL for readability reasons
            // #i33699# - 2004-09-24 - fs@openoffice.org

        FilterByEntryDataId aFilter( sDataSourceId );
        SvLBoxEntry* pDataSource = m_pTreeView->getListBox()->GetEntryPosByName( sDisplayName, NULL, &aFilter );
        if ( !pDataSource ) // check if the data source name is a file location
        {
            if ( bIsDataSourceURL )
            {
                // special case, the data source is a URL
                // add new entries to the list box model
                Image a, b, c;  // not interested in  reusing them
                String e, f;
                implAddDatasource( _rDataSource, a, e, b, f, c, _rxConnection );
                pDataSource = m_pTreeView->getListBox()->GetEntryPosByName( sDisplayName, NULL, &aFilter );
                DBG_ASSERT( pDataSource, "SbaTableQueryBrowser::getObjectEntry: hmm - did not find it again!" );
            }
        }
        if (_ppDataSourceEntry)
            // (caller wants to have it ...)
            *_ppDataSourceEntry = pDataSource;

        if (pDataSource)
        {
            // expand if required so
            if (_bExpandAncestors)
                m_pTreeView->getListBox()->Expand(pDataSource);

            // look for the object container
            SvLBoxEntry* pCommandType = NULL;
            switch (_nCommandType)
            {
                case CommandType::TABLE:
                    pCommandType = m_pTreeView->getListBox()->GetModel()->GetEntry(pDataSource, CONTAINER_TABLES);
                    break;

                case CommandType::QUERY:
                    pCommandType = m_pTreeView->getListBox()->GetModel()->GetEntry(pDataSource, CONTAINER_QUERIES);
                    break;
            }

            if (_ppContainerEntry)
                *_ppContainerEntry = pCommandType;

            if (pCommandType)
            {
                // expand if required so
                if (_bExpandAncestors)
                    m_pTreeView->getListBox()->Expand(pCommandType);

                // look for the object
                pObject = m_pTreeView->getListBox()->GetEntryPosByName(_rCommand, pCommandType);
            }
        }
    }
    return pObject;
}

// -------------------------------------------------------------------------
SvLBoxEntry* SbaTableQueryBrowser::getObjectEntry(const ::svx::ODataAccessDescriptor& _rDescriptor,
        SvLBoxEntry** _ppDataSourceEntry, SvLBoxEntry** _ppContainerEntry,
        sal_Bool _bExpandAncestors)
{
    // extract the props from the descriptor
    ::rtl::OUString sDataSource;
    ::rtl::OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    sal_Bool bEscapeProcessing = sal_True;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    return getObjectEntry( sDataSource, sCommand, nCommandType, _ppDataSourceEntry, _ppContainerEntry, _bExpandAncestors, SharedConnection() );
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::connectExternalDispatches()
{
    Reference< XDispatchProvider >  xProvider( getFrame(), UNO_QUERY );
    DBG_ASSERT(xProvider.is(), "SbaTableQueryBrowser::connectExternalDispatches: no DispatchProvider !");
    if (xProvider.is())
    {
        if ( m_aExternalFeatures.empty() )
        {
            const sal_Char* pURLs[] = {
                ".uno:DataSourceBrowser/DocumentDataSource",
                ".uno:DataSourceBrowser/FormLetter",
                ".uno:DataSourceBrowser/InsertColumns",
                ".uno:DataSourceBrowser/InsertContent",
            };
            const sal_uInt16 nIds[] = {
                ID_BROWSER_DOCUMENT_DATASOURCE,
                ID_BROWSER_FORMLETTER,
                ID_BROWSER_INSERTCOLUMNS,
                ID_BROWSER_INSERTCONTENT
            };

            for ( size_t i=0; i < sizeof( pURLs ) / sizeof( pURLs[0] ); ++i )
            {
                URL aURL;
                aURL.Complete = ::rtl::OUString::createFromAscii( pURLs[i] );
                if ( m_xUrlTransformer.is() )
                    m_xUrlTransformer->parseStrict( aURL );
                m_aExternalFeatures[ nIds[ i ] ] = ExternalFeature( aURL );
            }
        }

        for ( ExternalFeaturesMap::iterator feature = m_aExternalFeatures.begin();
              feature != m_aExternalFeatures.end();
              ++feature
            )
        {
            feature->second.xDispatcher = xProvider->queryDispatch(
                feature->second.aURL, ::rtl::OUString::createFromAscii("_parent"), FrameSearchFlag::PARENT
            );

            if ( feature->second.xDispatcher.get() == static_cast< XDispatch* >( this ) )
            {
                OSL_ENSURE( sal_False, "SbaTableQueryBrowser::connectExternalDispatches: this should not happen anymore!" );
                    // (nowadays, the URLs aren't in our SupportedFeatures list anymore, so we should
                    // not supply a dispatcher for this)
                feature->second.xDispatcher.clear();
            }

            if ( feature->second.xDispatcher.is() )
            {
                try
                {
                    feature->second.xDispatcher->addStatusListener( this, feature->second.aURL );
                }
                catch(Exception&)
                {
                    OSL_ENSURE( 0, "SbaTableQueryBrowser::connectExternalDispatches: caught an exception while attaching a status listener!!" );
                }
            }

            implCheckExternalSlot( feature->first );
        }
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implCheckExternalSlot( sal_uInt16 _nId )
{
    if ( !m_xMainToolbar.is() )
        return;

    Window* pToolboxWindow = VCLUnoHelper::GetWindow( m_xMainToolbar );
    ToolBox* pToolbox = dynamic_cast< ToolBox* >( pToolboxWindow );
    OSL_ENSURE( pToolbox, "SbaTableQueryBrowser::implCheckExternalSlot: cannot obtain the toolbox window!" );

    // check if we have to hide this item from the toolbox
    if ( pToolbox )
    {
        sal_Bool bHaveDispatcher = m_aExternalFeatures[ _nId ].xDispatcher.is();
        if ( bHaveDispatcher != pToolbox->IsItemVisible( _nId ) )
            bHaveDispatcher ? pToolbox->ShowItem( _nId ) : pToolbox->HideItem( _nId );
    }

    // and invalidate this feature in general
    InvalidateFeature( _nId );
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    // our frame ?
    Reference< ::com::sun::star::frame::XFrame >  xSourceFrame(_rSource.Source, UNO_QUERY);
    if (m_xCurrentFrameParent.is() && (xSourceFrame == m_xCurrentFrameParent))
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
    else
    {
        // search the external dispatcher causing this call in our map
        Reference< XDispatch > xSource(_rSource.Source, UNO_QUERY);
        if(xSource.is())
        {
            for (  ExternalFeaturesMap::iterator aLoop = m_aExternalFeatures.begin();
                  aLoop != m_aExternalFeatures.end();
                  ++aLoop
                )
            {
                if ( aLoop->second.xDispatcher.get() == xSource.get() )
                {
                    ExternalFeaturesMap::iterator aPrevious = aLoop;
                    --aPrevious;

                    // remove it
                    m_aExternalFeatures.erase( aLoop );

                    // maybe update the UI
                    implCheckExternalSlot(aLoop->first);

                    // continue, the same XDispatch may be resposible for more than one URL
                    aLoop = aPrevious;
                }
            }
        }
        else
        {
            Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
            if ( xCon.is() && m_pTreeView && m_pTreeView->getListBox() )
            {   // our connection is in dispose so we have to find the entry equal with this connection
                // and close it what means to collapse the entry
                // get the top-level representing the removed data source
                SvLBoxEntry* pDSLoop = m_pTreeView->getListBox()->FirstChild(NULL);
                while (pDSLoop)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSLoop->GetUserData());
                    if ( pData && pData->xConnection == xCon )
                    {
                        // we set the conenction to null to avoid a second disposing of the connection
                        pData->xConnection.clear();
                        closeConnection(pDSLoop,sal_False);
                        break;
                    }

                    pDSLoop = m_pTreeView->getListBox()->NextSibling(pDSLoop);
                }
            }
            else
                SbaXDataBrowserController::disposing(_rSource);
        }
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implRemoveStatusListeners()
{
    // clear all old dispatches
    for ( ExternalFeaturesMap::const_iterator aLoop = m_aExternalFeatures.begin();
          aLoop != m_aExternalFeatures.end();
          ++aLoop
        )
    {
        if ( aLoop->second.xDispatcher.is() )
        {
            try
            {
                aLoop->second.xDispatcher->removeStatusListener( this, aLoop->second.aURL );
            }
            catch (Exception&)
            {
                DBG_ERROR("SbaTableQueryBrowser::implRemoveStatusListeners: could not remove a status listener!");
            }
        }
    }
    m_aExternalFeatures.clear();
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL SbaTableQueryBrowser::select( const Any& _rSelection ) throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    Sequence< PropertyValue > aDescriptorSequence;
    if (!(_rSelection >>= aDescriptorSequence))
        throw IllegalArgumentException(::rtl::OUString(), *this, 1);
        // TODO: error message

    ODataAccessDescriptor aDescriptor;
    try
    {
        aDescriptor = ODataAccessDescriptor(aDescriptorSequence);
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "SbaTableQueryBrowser::select: could not extract the descriptor!");
    }

    // check the precense of the props we need
    if ( !(aDescriptor.has(daDataSource) || aDescriptor.has(daDatabaseLocation)) || !aDescriptor.has(daCommand) || !aDescriptor.has(daCommandType))
        throw IllegalArgumentException(::rtl::OUString(), *this, 1);
        // TODO: error message

    return implSelect(aDescriptor,sal_True);
}

// -------------------------------------------------------------------------
Any SAL_CALL SbaTableQueryBrowser::getSelection(  ) throw (RuntimeException)
{
    Any aReturn;

    try
    {
        Reference< XLoadable > xLoadable(getRowSet(), UNO_QUERY);
        if (xLoadable.is() && xLoadable->isLoaded())
        {
            Reference< XPropertySet > aFormProps(getRowSet(), UNO_QUERY);
            ODataAccessDescriptor aDescriptor(aFormProps);
            // remove properties which are not part of our "selection"
            aDescriptor.erase(daConnection);
            aDescriptor.erase(daCursor);

            aReturn <<= aDescriptor.createPropertyValueSequence();
        }
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "SbaTableQueryBrowser::getSelection: caught an exception while retrieving the selection!");
    }

    return aReturn;
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::addSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener ) throw (RuntimeException)
{
    m_aSelectionListeners.addInterface(_rxListener);
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener ) throw (RuntimeException)
{
    m_aSelectionListeners.removeInterface(_rxListener);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & _xFrame) throw( RuntimeException )
{
    implRemoveStatusListeners();

    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

    SbaXDataBrowserController::attachFrame(_xFrame);

    Reference< XFrame > xCurrentFrame( getFrame() );
    if ( xCurrentFrame.is() )
    {
        m_xCurrentFrameParent = xCurrentFrame->findFrame(::rtl::OUString::createFromAscii("_parent"),FrameSearchFlag::PARENT);
        if ( m_xCurrentFrameParent.is() )
            m_xCurrentFrameParent->addFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

        // obtain our toolbox
        try
        {
            Reference< XPropertySet > xFrameProps( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            Reference< XLayoutManager > xLayouter(
                xFrameProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) ),
                UNO_QUERY );

            if ( xLayouter.is() )
            {
                Reference< XUIElement > xUI(
                    xLayouter->getElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/toolbar" ) ) ),
                    UNO_SET_THROW );
                m_xMainToolbar = m_xMainToolbar.query( xUI->getRealInterface() );
                OSL_ENSURE( m_xMainToolbar.is(), "SbaTableQueryBrowser::attachFrame: where's my toolbox?" );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // get the dispatchers for the external slots
    connectExternalDispatches();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::addModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xSourceSet(_xGridControlModel, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->addPropertyChangeListener(PROPERTY_ROW_HEIGHT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_FONT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTCOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTLINECOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTEMPHASIS, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTRELIEF, static_cast<XPropertyChangeListener*>(this));
    }

}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::removeModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::removeModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xSourceSet(_xGridControlModel, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->removePropertyChangeListener(PROPERTY_ROW_HEIGHT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_FONT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTCOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTLINECOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTEMPHASIS, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTRELIEF, static_cast<XPropertyChangeListener*>(this));
    }
}
// -------------------------------------------------------------------------
void SbaTableQueryBrowser::RowChanged()
{
    if(getBrowserView())
    {
        SbaGridControl* pControl = getBrowserView()->getVclControl();
        if (!pControl->IsEditing())
            InvalidateFeature(ID_BROWSER_COPY);
    }
    SbaXDataBrowserController::RowChanged();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::ColumnChanged()
{
    if(getBrowserView())
    {
        SbaGridControl* pControl = getBrowserView()->getVclControl();
        if (!pControl->IsEditing())
            InvalidateFeature(ID_BROWSER_COPY);
    }
    SbaXDataBrowserController::ColumnChanged();
}
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::AddColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::AddColumnListener(xCol);
    SafeAddPropertyListener(xCol, PROPERTY_WIDTH, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_HIDDEN, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_ALIGN, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_FORMATKEY, static_cast<XPropertyChangeListener*>(this));
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::RemoveColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::RemoveColumnListener(xCol);
    SafeRemovePropertyListener(xCol, PROPERTY_WIDTH, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_HIDDEN, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_ALIGN, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_FORMATKEY, static_cast<XPropertyChangeListener*>(this));
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::criticalFail()
{
    SbaXDataBrowserController::criticalFail();
    unloadAndCleanup( sal_False );
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::LoadFinished(sal_Bool _bWasSynch)
{
    SbaXDataBrowserController::LoadFinished(_bWasSynch);

    m_sQueryCommand = ::rtl::OUString();
    m_bQueryEscapeProcessing = sal_False;

    if (isValid() && !loadingCancelled())
    {
        // did we load a query?
        sal_Bool bTemporary;    // needed because we m_bQueryEscapeProcessing is only one bit wide (and we want to pass it by reference)
        if ( implGetQuerySignature( m_sQueryCommand, bTemporary ) )
            m_bQueryEscapeProcessing = bTemporary;
    }

    // if the form has been loaded, this means that our "selection" has changed
    EventObject aEvent( *this );
    m_aSelectionListeners.notifyEach( &XSelectionChangeListener::selectionChanged, aEvent );

    // update our database document
    Reference< XModel > xDocument;
    try
    {
        Reference< XPropertySet > xCursorProps( getRowSet(), UNO_QUERY_THROW );
        Reference< XConnection > xConnection( xCursorProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ), UNO_QUERY );
        if ( xConnection.is() )
        {
            Reference< XChild > xChild( xConnection, UNO_QUERY_THROW );
            Reference< XDocumentDataSource > xDataSource( xChild->getParent(), UNO_QUERY_THROW );
            xDocument.set( xDataSource->getDatabaseDocument(), UNO_QUERY_THROW );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_xCurrentDatabaseDocument = xDocument;
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::getExternalSlotState( sal_uInt16 _nId ) const
{
    sal_Bool bEnabled = sal_False;
    ExternalFeaturesMap::const_iterator aPos = m_aExternalFeatures.find( _nId );
    if ( ( m_aExternalFeatures.end() != aPos ) && aPos->second.xDispatcher.is() )
        bEnabled = aPos->second.bEnabled;
    return bEnabled;
}

//------------------------------------------------------------------------------
FeatureState SbaTableQueryBrowser::GetState(sal_uInt16 nId) const
{
    FeatureState aReturn;
        // (disabled automatically)

    // no chance without a view
    if (!getBrowserView() || !getBrowserView()->getVclControl())
        return aReturn;

    switch (nId)
    {
        case ID_BROWSER_CLOSE:
            // the close button should always be enabled
            aReturn.bEnabled = !m_bEnableBrowser;
            return aReturn;
    // "toggle explorer" is always enabled (if we have a explorer)
        case ID_BROWSER_EXPLORER:
        {       // this slot is available even if no form is loaded
            aReturn.bEnabled = m_bEnableBrowser;
            aReturn.bChecked = haveExplorer();
            return aReturn;
        }
        case ID_BROWSER_REMOVEFILTER:
            aReturn = SbaXDataBrowserController::GetState(nId);
            return aReturn;
    }

    if (!isLoaded())
        return aReturn;

    try
    {
        sal_Bool bHandled = sal_False;
        switch (nId)
        {
            case ID_BROWSER_DOCUMENT_DATASOURCE:
                // the slot is enabled if we have an external dispatcher able to handle it,
                // and the dispatcher must have enabled the slot in general
                aReturn.bEnabled = getExternalSlotState( ID_BROWSER_DOCUMENT_DATASOURCE );
                bHandled = sal_True;
                break;
            case ID_BROWSER_REFRESH:
                aReturn.bEnabled = sal_True;
                bHandled = sal_True;
                break;
        }

        if (bHandled)
            return aReturn;

        // no chance without valid models
        if (isValid() && !isValidCursor() && nId != ID_BROWSER_CLOSE)
            return aReturn;

        // no chance while loading the form
        if (PendingLoad())
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_INSERTCOLUMNS:
            case ID_BROWSER_INSERTCONTENT:
            case ID_BROWSER_FORMLETTER:
            {
                // the slot is enabled if we have an external dispatcher able to handle it,
                // and the dispatcher must have enabled the slot in general
                aReturn.bEnabled = getExternalSlotState( nId );

                // for the Insert* slots, we need at least one selected row
                if (ID_BROWSER_FORMLETTER != nId)
                    aReturn.bEnabled = aReturn.bEnabled && getBrowserView()->getVclControl()->GetSelectRowCount();

                // disabled for native queries which are not saved within the database
                // 67706 - 23.08.99 - FS
                Reference< XPropertySet >  xDataSource(getRowSet(), UNO_QUERY);
                try
                {
                    aReturn.bEnabled = aReturn.bEnabled && xDataSource.is();

                    if (xDataSource.is())
                    {
                        sal_Int32 nType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMAND_TYPE));
                        aReturn.bEnabled = aReturn.bEnabled && ((::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) || (nType == ::com::sun::star::sdb::CommandType::QUERY)));
                    }
                }
                catch(DisposedException&)
                {
                    OSL_ENSURE(sal_False, "SbaTableQueryBrowser::GetState: object already disposed!");
                }
                catch(Exception&)
                {
                    OSL_ENSURE(sal_False, "SbaTableQueryBrowser::GetState: caught a strange exception!!");
                }

            }
            break;
            case ID_BROWSER_TITLE:
                {
                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                    sal_Int32 nCommandType = CommandType::TABLE;
                    xProp->getPropertyValue(PROPERTY_COMMAND_TYPE) >>= nCommandType;
                    String sTitle;
                    switch (nCommandType)
                    {
                        case CommandType::TABLE:
                            sTitle = String(ModuleRes(STR_TBL_TITLE)); break;
                        case CommandType::QUERY:
                        case CommandType::COMMAND:
                            sTitle = String(ModuleRes(STR_QRY_TITLE)); break;
                        default:
                            DBG_ASSERT(sal_False, "SbaTableQueryBrowser::GetState: unknown command type!");
                    }
                    ::rtl::OUString aName;
                    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aName;
                    String sObject(aName.getStr());

                    sTitle.SearchAndReplace('#',sObject);
                    aReturn.sTitle = sTitle;
                    aReturn.bEnabled = sal_True;
                }
                break;
            case ID_BROWSER_TABLEATTR:
            case ID_BROWSER_ROWHEIGHT:
            case ID_BROWSER_COLATTRSET:
            case ID_BROWSER_COLWIDTH:
                aReturn.bEnabled = getBrowserView() && getBrowserView()->getVclControl() && isValid() && isValidCursor();
                //  aReturn.bEnabled &= getDefinition() && !getDefinition()->GetDatabase()->IsReadOnly();
                break;

            case ID_BROWSER_COPY:
                if(m_pTreeView->HasChildPathFocus())
                    aReturn.bEnabled = isEntryCopyAllowed(m_pTreeView->getListBox()->GetCurEntry());
                else if (getBrowserView() && getBrowserView()->getVclControl() && !getBrowserView()->getVclControl()->IsEditing())
                {
                    SbaGridControl* pControl = getBrowserView()->getVclControl();
                    aReturn.bEnabled = pControl->canCopyCellText(pControl->GetCurRow(), pControl->GetCurColumnId());
                }
                else
                    return SbaXDataBrowserController::GetState(nId);
                break;

            default:
                return SbaXDataBrowserController::GetState(nId);
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aReturn;

}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::Execute(sal_uInt16 nId, const Sequence< PropertyValue >& aArgs)
{
    switch (nId)
    {
        case ID_BROWSER_REFRESH:
        {
            if ( !SaveModified( ) )
                // nothing to do
                break;

            sal_Bool bFullReinit = sal_False;
            // check if the query signature (if the form is based on a query) has changed
            if ( m_sQueryCommand.getLength() )
            {
                ::rtl::OUString sNewQueryCommand;
                sal_Bool bNewQueryEP;

#if OSL_DEBUG_LEVEL > 0
                sal_Bool bIsQuery =
#endif
                implGetQuerySignature( sNewQueryCommand, bNewQueryEP );
                OSL_ENSURE( bIsQuery, "SbaTableQueryBrowser::Execute: was a query before, but is not anymore?" );

                bFullReinit = ( sNewQueryCommand != m_sQueryCommand ) || ( m_bQueryEscapeProcessing != bNewQueryEP );
            }
            if ( !bFullReinit )
            {
                // let the base class do a simple reload
                SbaXDataBrowserController::Execute(nId,aArgs);
                break;
            }
            // NO break here!
        }

        case ID_BROWSER_REFRESH_REBUILD:
        {
            if ( !SaveModified() )
                // nothing to do
                break;

            SvLBoxEntry* pSelected = m_pCurrentlyDisplayed;
            // unload
            unloadAndCleanup( sal_False );

            // reselect the entry
            if(pSelected)
                OnSelectEntry( pSelected );
            else
            {
                Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                implSelect(::svx::ODataAccessDescriptor(xProp));
            }
        }
        break;

        case ID_BROWSER_EXPLORER:
            toggleExplorer();
            break;

        case ID_BROWSER_DOCUMENT_DATASOURCE:
            implSelect(m_aDocumentDataSource);
            break;

        case ID_BROWSER_INSERTCOLUMNS:
        case ID_BROWSER_INSERTCONTENT:
        case ID_BROWSER_FORMLETTER:
            if (getBrowserView() && isValidCursor())
            {
                // the URL the slot id is assigned to
                OSL_ENSURE( m_aExternalFeatures.find( nId ) != m_aExternalFeatures.end(),
                    "SbaTableQueryBrowser::Execute( ID_BROWSER_?): how could this ever be enabled?" );
                URL aParentUrl = m_aExternalFeatures[ nId ].aURL;

                // let the dispatcher execute the slot
                Reference< XDispatch > xDispatch( m_aExternalFeatures[ nId ].xDispatcher );
                if (xDispatch.is())
                {
                    // set the properties for the dispatch

                    // first fill the selection
                    SbaGridControl* pGrid = getBrowserView()->getVclControl();
                    MultiSelection* pSelection = (MultiSelection*)pGrid->GetSelection();
                    Sequence< Any > aSelection;
                    if ( !pGrid->IsAllSelected() )
                    {   // transfer the selected rows only if not all rows are selected
                        // (all rows means the whole table)
                        // i3832 - 03.04.2002 - fs@openoffice.org
                        if (pSelection != NULL)
                        {
                            aSelection.realloc(pSelection->GetSelectCount());
                            long nIdx = pSelection->FirstSelected();
                            Any* pSelectionNos = aSelection.getArray();
                            while (nIdx >= 0)
                            {
                                *pSelectionNos++ <<= (sal_Int32)(nIdx + 1);
                                nIdx = pSelection->NextSelected();
                            }
                        }
                    }

                    Reference< XResultSet > xCursorClone;
                    try
                    {
                        Reference< XResultSetAccess > xResultSetAccess(getRowSet(),UNO_QUERY);
                        if (xResultSetAccess.is())
                            xCursorClone = xResultSetAccess->createResultSet();
                    }
                    catch(DisposedException&)
                    {
                        OSL_ENSURE(0,"Object already disposed!");
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("SbaTableQueryBrowser::Execute(ID_BROWSER_?): could not clone the cursor!");
                    }

                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);

                    try
                    {
                        ODataAccessDescriptor aDescriptor;
                        ::rtl::OUString sDataSourceName;
                        xProp->getPropertyValue(PROPERTY_DATASOURCENAME) >>= sDataSourceName;

                        aDescriptor.setDataSource(sDataSourceName);
                        aDescriptor[daCommand]      =   xProp->getPropertyValue(PROPERTY_COMMAND);
                        aDescriptor[daCommandType]  =   xProp->getPropertyValue(PROPERTY_COMMAND_TYPE);
                        aDescriptor[daConnection]   =   xProp->getPropertyValue(PROPERTY_ACTIVE_CONNECTION);
                        aDescriptor[daCursor]       <<= xCursorClone;
                        if ( aSelection.getLength() )
                        {
                            aDescriptor[daSelection]            <<= aSelection;
                            aDescriptor[daBookmarkSelection]    <<= sal_False;
                                // these are selection indicies
                                // before we change this, all clients have to be adjusted
                                // so that they recognize the new BookmarkSelection property!
                        }

                        xDispatch->dispatch(aParentUrl, aDescriptor.createPropertyValueSequence());
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("SbaTableQueryBrowser::Execute(ID_BROWSER_?): could not dispatch the slot (caught an exception)!");
                    }
                }
            }
            break;

        case ID_BROWSER_CLOSE:
            closeTask();
            // if it's not 0, such a async close is already pending
            break;

        case ID_BROWSER_COPY:
            if(m_pTreeView->HasChildPathFocus())
            {
                copyEntry(m_pTreeView->getListBox()->GetCurEntry());
            }
            else if (getBrowserView() && getBrowserView()->getVclControl() && !getBrowserView()->getVclControl()->IsEditing())
            {
                SbaGridControl* pControl = getBrowserView()->getVclControl();
                pControl->copyCellText(pControl->GetCurRow(), pControl->GetCurColumnId());
            }
            else
                SbaXDataBrowserController::Execute(nId,aArgs);
            break;
        default:
            SbaXDataBrowserController::Execute(nId,aArgs);
            break;
    }
}
// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implAddDatasource(const String& _rDbName, Image& _rDbImage,
        String& _rQueryName, Image& _rQueryImage, String& _rTableName, Image& _rTableImage,
        const SharedConnection& _rxConnection)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    // initialize the names/images if necessary
    if (!_rQueryName.Len())
        _rQueryName = String(ModuleRes(RID_STR_QUERIES_CONTAINER));
    if (!_rTableName.Len())
        _rTableName = String(ModuleRes(RID_STR_TABLES_CONTAINER));

    ImageProvider aImageProvider;
    if (!_rQueryImage)
        _rQueryImage = aImageProvider.getFolderImage( DatabaseObject::QUERY, isHiContrast() );
    if (!_rTableImage)
        _rTableImage = aImageProvider.getFolderImage( DatabaseObject::TABLE, isHiContrast() );

    if (!_rDbImage)
        _rDbImage = aImageProvider.getDatabaseImage( isHiContrast() );

    // add the entry for the data source
    // special handling for data sources denoted by URLs - we do not want to display this ugly URL, do we?
    // #i33699# - 2004-09-24 - fs@openoffice.org
    String sDSDisplayName, sDataSourceId;
    getDataSourceDisplayName_isURL( _rDbName, sDSDisplayName, sDataSourceId );

    SvLBoxEntry* pDatasourceEntry = m_pTreeView->getListBox()->InsertEntry( sDSDisplayName, _rDbImage, _rDbImage, NULL, sal_False );
    DBTreeListModel::DBTreeListUserData* pDSData = new DBTreeListModel::DBTreeListUserData;
    pDSData->eType = etDatasource;
    pDSData->sAccessor = sDataSourceId;
    pDSData->xConnection = _rxConnection;
    pDatasourceEntry->SetUserData(pDSData);

    // the child for the queries container
    {
        SvLBoxEntry* pQueries = m_pTreeView->getListBox()->InsertEntry(_rQueryName, _rQueryImage, _rQueryImage, pDatasourceEntry, sal_True);
        DBTreeListModel::DBTreeListUserData* pQueriesData = new DBTreeListModel::DBTreeListUserData;
        pQueriesData->eType = etQueryContainer;
        pQueries->SetUserData(pQueriesData);
    }

    // the child for the tables container
    {
        SvLBoxEntry* pTables = m_pTreeView->getListBox()->InsertEntry(_rTableName, _rTableImage, _rTableImage, pDatasourceEntry, sal_True);
        DBTreeListModel::DBTreeListUserData* pTablesData = new DBTreeListModel::DBTreeListUserData;
        pTablesData->eType = etTableContainer;
        pTables->SetUserData(pTablesData);
    }

}
// -------------------------------------------------------------------------
void SbaTableQueryBrowser::initializeTreeModel()
{
    if (m_xDatabaseContext.is())
    {
        Image aDBImage, aQueriesImage, aTablesImage;
        String sQueriesName, sTablesName;

        // fill the model with the names of the registered datasources
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pIter    = aDatasources.getConstArray();
        const ::rtl::OUString* pEnd     = pIter + aDatasources.getLength();
        for (; pIter != pEnd; ++pIter)
            implAddDatasource( *pIter, aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage, SharedConnection() );
    }
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::populateTree(const Reference<XNameAccess>& _xNameAccess,
                                            SvLBoxEntry* _pParent,
                                            EntryType _eEntryType)
{
    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pParent->GetUserData());
    if(pData) // don't ask if the nameaccess is already set see OnExpandEntry views and tables
        pData->xContainer = _xNameAccess;

    try
    {
        Sequence< ::rtl::OUString > aNames = _xNameAccess->getElementNames();
        const ::rtl::OUString* pIter    = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pIter + aNames.getLength();
        for (; pIter != pEnd; ++pIter)
        {
            if(!m_pTreeView->getListBox()->GetEntryPosByName(*pIter,_pParent))
            {
                DBTreeListModel::DBTreeListUserData* pEntryData = new DBTreeListModel::DBTreeListUserData;
                pEntryData->eType = _eEntryType;
                implAppendEntry( _pParent, *pIter, pEntryData, _eEntryType );
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::populateTree: could not fill the tree");
        return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::implAppendEntry( SvLBoxEntry* _pParent, const String& _rName, void* _pUserData, EntryType _eEntryType )
{
    ::std::auto_ptr< ImageProvider > pImageProvider( getImageProviderFor( _pParent ) );

    Image aImage, aImageHC;
    pImageProvider->getImages( _rName, getDatabaseObjectType( _eEntryType ), aImage, aImageHC );

    SvLBoxEntry* pNewEntry = m_pTreeView->getListBox()->InsertEntry( _rName, _pParent, sal_False, LIST_APPEND, _pUserData );

    m_pTreeView->getListBox()->SetExpandedEntryBmp( pNewEntry, aImage, BMP_COLOR_NORMAL );
    m_pTreeView->getListBox()->SetCollapsedEntryBmp( pNewEntry, aImage, BMP_COLOR_NORMAL );
    m_pTreeView->getListBox()->SetExpandedEntryBmp( pNewEntry, aImageHC, BMP_COLOR_HIGHCONTRAST );
    m_pTreeView->getListBox()->SetCollapsedEntryBmp( pNewEntry, aImageHC, BMP_COLOR_HIGHCONTRAST );
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, SvLBoxEntry*, _pParent)
{
    if (_pParent->HasChilds())
        // nothing to to ...
        return 1L;

    ::osl::MutexGuard aGuard(m_aEntryMutex);

    SvLBoxEntry* pFirstParent = m_pTreeView->getListBox()->GetRootLevelParent(_pParent);
    OSL_ENSURE(pFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListModel::DBTreeListUserData* pData = static_cast< DBTreeListModel::DBTreeListUserData* >(_pParent->GetUserData());
    OSL_ENSURE(pData,"SbaTableQueryBrowser::OnExpandEntry: No user data!");
#if OSL_DEBUG_LEVEL > 0
    SvLBoxString* pString = static_cast<SvLBoxString*>(pFirstParent->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
    OSL_ENSURE(pString,"SbaTableQueryBrowser::OnExpandEntry: No string item!");
#endif

    if (etTableContainer == pData->eType)
    {
        WaitObject aWaitCursor(getBrowserView());

        // it could be that we already have a connection
        SharedConnection xConnection;
        ensureConnection( pFirstParent, xConnection );

        if ( xConnection.is() )
        {
            SQLExceptionInfo aInfo;
            try
            {
                Reference< XWarningsSupplier > xWarnings(xConnection, UNO_QUERY);
                if (xWarnings.is())
                    xWarnings->clearWarnings();

                // first insert the views because the tables can also include
                // views but that time the bitmap is the wrong one
                // the nameaccess will be overwriten in populateTree
                Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                if(xViewSup.is())
                    populateTree( xViewSup->getViews(), _pParent, etTableOrView );

                Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                if(xTabSup.is())
                {
                    populateTree( xTabSup->getTables(), _pParent, etTableOrView );
                    Reference<XContainer> xCont(xTabSup->getTables(),UNO_QUERY);
                    if(xCont.is())
                        // add as listener to know when elements are inserted or removed
                        xCont->addContainerListener(this);
                }

                if (xWarnings.is())
                {
                    SQLExceptionInfo aWarnings(xWarnings->getWarnings());
                    if (aWarnings.isValid() && sal_False)
                    {
                        SQLContext aContext;
                        aContext.Message = String(ModuleRes(STR_OPENTABLES_WARNINGS));
                        aContext.Details = String(ModuleRes(STR_OPENTABLES_WARNINGS_DETAILS));
                        aContext.NextException = aWarnings.get();
                        aWarnings = aContext;
                        showError(aWarnings);
                    }
                    // TODO: we need a better concept for these warnings:
                    // something like "don't show any warnings for this datasource, again" would be nice
                    // But this requires an extension of the InteractionHandler and an additional property on the data source
                }
            }
            catch(const SQLContext& e) { aInfo = e; }
            catch(const SQLWarning& e) { aInfo = e; }
            catch(const SQLException& e) { aInfo = e; }
            catch(const WrappedTargetException& e)
            {
                SQLException aSql;
                if(e.TargetException >>= aSql)
                    aInfo = aSql;
                else
                    OSL_ENSURE(sal_False, "SbaTableQueryBrowser::OnExpandEntry: something strange happended!");
            }
            catch(const Exception&)
            {
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser, OnExpandEntry: caught an unknown exception while populating the tables!");
            }
            if (aInfo.isValid())
                showError(aInfo);
        }
        else
            return 0L;
                // 0 indicates that an error occured
    }
    else
    {   // we have to expand the queries or bookmarks
        if (ensureEntryObject(_pParent))
        {
            DBTreeListModel::DBTreeListUserData* pParentData = static_cast< DBTreeListModel::DBTreeListUserData* >( _pParent->GetUserData() );
            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
            populateTree( xCollection, _pParent, etQuery );
        }
    }
    return 1L;
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::ensureEntryObject( SvLBoxEntry* _pEntry )
{
    DBG_ASSERT(_pEntry, "SbaTableQueryBrowser::ensureEntryObject: invalid argument!");
    if (!_pEntry)
        return sal_False;

    EntryType eType = getEntryType( _pEntry );

    // the user data of the entry
    DBTreeListModel::DBTreeListUserData* pEntryData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pEntry->GetUserData());
    OSL_ENSURE(pEntryData,"ensureEntryObject: user data should already be set!");

    SvLBoxEntry* pDataSourceEntry = m_pTreeView->getListBox()->GetRootLevelParent(_pEntry);

    sal_Bool bSuccess = sal_False;
    switch (eType)
    {
        case etQueryContainer:
            if ( pEntryData->xContainer.is() )
            {
                // nothing to do
                bSuccess = sal_True;
                break;
            }

            try
            {
                Reference< XQueryDefinitionsSupplier > xQuerySup;
                m_xDatabaseContext->getByName( getDataSourceAcessor( pDataSourceEntry ) ) >>= xQuerySup;
                if (xQuerySup.is())
                {
                    Reference< XNameAccess > xQueryDefs = xQuerySup->getQueryDefinitions();
                    Reference< XContainer > xCont(xQueryDefs, UNO_QUERY);
                    if (xCont.is())
                        // add as listener to get notified if elements are inserted or removed
                        xCont->addContainerListener(this);

                    pEntryData->xContainer = xQueryDefs;
                    bSuccess = pEntryData->xContainer.is();
                }
                else
                    DBG_ERROR("SbaTableQueryBrowser::ensureEntryObject: no XQueryDefinitionsSupplier interface!");
            }
            catch(Exception&)
            {
                DBG_ERROR("SbaTableQueryBrowser::ensureEntryObject: caught an exception while retrieving the queries container!");
            }
            break;

        default:
            DBG_ERROR("SbaTableQueryBrowser::ensureEntryObject: ooops ... missing some implementation here!");
            // TODO ...
            break;
    }

    return bSuccess;
}
//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::implSelect(const ::svx::ODataAccessDescriptor& _rDescriptor,sal_Bool _bSelectDirect)
{
    // extract the props
    ::rtl::OUString sDataSource;
    ::rtl::OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    sal_Bool bEscapeProcessing = sal_True;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    // select it
    return implSelect( sDataSource, sCommand, nCommandType, bEscapeProcessing, SharedConnection(), _bSelectDirect );
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::implLoadAnything(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rCommand,
    const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing, const SharedConnection& _rxConnection)
{
    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
    if(xProp.is())
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(xProp,UNO_QUERY);
        try
        {
            // the values allowing the RowSet to re-execute
            xProp->setPropertyValue(PROPERTY_DATASOURCENAME, makeAny(_rDataSourceName));
            if(_rxConnection.is())
                xProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( _rxConnection.getTyped() ) );

                // set this _before_ setting the connection, else the rowset would rebuild it ...
            xProp->setPropertyValue(PROPERTY_COMMAND_TYPE, makeAny(_nCommandType));
            xProp->setPropertyValue(PROPERTY_COMMAND, makeAny(_rCommand));
            xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, ::cppu::bool2any(_bEscapeProcessing));
            if ( m_bPreview )
            {
                // this be undone by the grid control in DbGridControl::RecalcRows
                // xProp->setPropertyValue(PROPERTY_FETCHSIZE, makeAny(sal_Int32(20)));
                xProp->setPropertyValue(PROPERTY_FETCHDIRECTION, makeAny(FetchDirection::FORWARD));
            }

            // the formatter depends on the data source we're working on, so rebuild it here ...
            initFormatter();

            // switch the grid to design mode while loading
            getBrowserView()->getGridControl()->setDesignMode(sal_True);
            InitializeForm(getRowSet());

            sal_Bool bSuccess = sal_True;

            {
                {
                    Reference< XNameContainer >  xColContainer(getFormComponent(), UNO_QUERY);
                    // first we have to clear the grid
                    clearGridColumns(xColContainer);
                }
                FormErrorHelper aHelper(this);
                // load the form
                bSuccess = reloadForm(xLoadable);

                // initialize the model
                InitializeGridModel(getFormComponent());

                if ( m_bPreview )
                    initializePreviewMode();

                LoadFinished(sal_True);
            }

            InvalidateAll();
            return bSuccess;
        }
        catch(SQLException& e)
        {
            showError(SQLExceptionInfo(e));
        }
        catch(WrappedTargetException& e)
        {
            SQLException aSql;
            if(e.TargetException >>= aSql)
                showError(SQLExceptionInfo(aSql));
            else
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser::implLoadAnything: something strange happended!");
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "SbaTableQueryBrowser::implLoadAnything: something strange happended!");
        }
    }

    InvalidateAll();
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::implSelect(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rCommand,
                                      const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing,
                                      const SharedConnection& _rxConnection
                                      ,sal_Bool _bSelectDirect)
{
    if (_rDataSourceName.getLength() && _rCommand.getLength() && (-1 != _nCommandType))
    {
        SvLBoxEntry* pDataSource = NULL;
        SvLBoxEntry* pCommandType = NULL;
        SvLBoxEntry* pCommand = getObjectEntry( _rDataSourceName, _rCommand, _nCommandType, &pDataSource, &pCommandType, sal_True, _rxConnection );

        //  if (pDataSource) // OJ change for the new app
        {
            if (pCommand)
            {
                if ( _bSelectDirect )
                {
                    OnSelectEntry(pCommand);
                }
                else
                    m_pTreeView->getListBox()->Select(pCommand);
            }
            else if (!pCommandType)
            {
                if ( m_pCurrentlyDisplayed )
                {   // tell the old entry (if any) it has been deselected
                    selectPath(m_pCurrentlyDisplayed, sal_False);
                    m_pCurrentlyDisplayed = NULL;
                }

                // we have a command and need to display this in the rowset
                return implLoadAnything(_rDataSourceName, _rCommand, _nCommandType, _bEscapeProcessing, _rxConnection);
            }
        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnSelectEntry, SvLBoxEntry*, _pEntry)
{
    ::osl::MutexGuard aGuard(m_aEntryMutex);

    DBTreeListModel::DBTreeListUserData* pEntryData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pEntry->GetUserData());
    switch (pEntryData->eType)
    {
        case etTableOrView:
        case etQuery:
            break;
        default:
            // nothing to do
            return 0L;
    }

    OSL_ENSURE(m_pTreeModel->HasParent(_pEntry), "SbaTableQueryBrowser::OnSelectEntry: invalid entry (1)!");
    OSL_ENSURE(m_pTreeModel->HasParent(m_pTreeModel->GetParent(_pEntry)), "SbaTableQueryBrowser::OnSelectEntry: invalid entry (2)!");

    // get the entry for the tables or queries
    SvLBoxEntry* pContainer = m_pTreeModel->GetParent(_pEntry);
    DBTreeListModel::DBTreeListUserData* pContainerData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());

    // get the entry for the datasource
    SvLBoxEntry* pConnection = m_pTreeModel->GetParent(pContainer);
    DBTreeListModel::DBTreeListUserData* pConData = static_cast<DBTreeListModel::DBTreeListUserData*>(pConnection->GetUserData());

    // reinitialize the rowset
    // but first check if it is necessary
    // get all old properties
    Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
    ::rtl::OUString aOldName;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND) >>= aOldName;
    sal_Int32 nOldType = 0;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND_TYPE) >>= nOldType;
    Reference<XConnection> xOldConnection;
    ::cppu::extractInterface(xOldConnection,xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION));
    // the name of the table or query
    SvLBoxString* pString = (SvLBoxString*)_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
    OSL_ENSURE(pString,"There must be a string item!");
    ::rtl::OUString aName(pString->GetText().GetBuffer());

    sal_Int32 nCommandType =    ( etTableContainer == pContainerData->eType)
                            ?   CommandType::TABLE
                            :   CommandType::QUERY;

    // check if need to rebuild the rowset
    sal_Bool bRebuild = ( xOldConnection != pConData->xConnection )
                     || ( nOldType != nCommandType )
                     || ( aName != aOldName );

    Reference< ::com::sun::star::form::XLoadable >  xLoadable = getLoadable();
    bRebuild |= !xLoadable->isLoaded();
    if(bRebuild)
    {
        try
        {
            WaitObject aWaitCursor(getBrowserView());

            // tell the old entry it has been deselected
            selectPath(m_pCurrentlyDisplayed, sal_False);
            m_pCurrentlyDisplayed = NULL;

            // not really loaded
            m_pCurrentlyDisplayed = _pEntry;
            // tell the new entry it has been selected
            selectPath(m_pCurrentlyDisplayed, sal_True);

            // get the name of the data source currently selected
            ensureConnection( m_pCurrentlyDisplayed, pConData->xConnection );

            if ( !pConData->xConnection.is() )
            {
                unloadAndCleanup( sal_False );
                return 0L;
            }

            Reference<XNameAccess> xNameAccess;
            switch(nCommandType)
            {
                case CommandType::TABLE:
                    {
                        // only for tables
                        if ( !pContainerData->xContainer.is() )
                        {
                            Reference<XTablesSupplier> xSup( pConData->xConnection, UNO_QUERY );
                            if(xSup.is())
                                xNameAccess = xSup->getTables();

                            pContainerData->xContainer = xNameAccess;
                        }
                        else
                            xNameAccess.set( pContainerData->xContainer, UNO_QUERY );
                    }
                    break;
                case CommandType::QUERY:
                    {
                        Reference<XQueriesSupplier> xSup( pConData->xConnection, UNO_QUERY );
                        if(xSup.is())
                            xNameAccess = xSup->getQueries();
                    }
                    break;
            }
            String sStatus(ModuleRes( CommandType::TABLE == nCommandType ? STR_LOADING_TABLE : STR_LOADING_QUERY ));
            sStatus.SearchAndReplaceAscii("$name$", aName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sStatus);

            if(xNameAccess.is() && xNameAccess->hasByName(aName))
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pEntry->GetUserData());
                if ( !pData->xObjectProperties.is() )
                {
                    Reference<XInterface> xObject;
                    if(xNameAccess->getByName(aName) >>= xObject) // remember the table or query object
                    {
                        pData->xObjectProperties = pData->xObjectProperties.query( xObject );
                        // if the query contains a parameterized statement and preview is enabled we won't get any data.
                        if ( m_bPreview && nCommandType == CommandType::QUERY && xObject.is() )
                        {
                            ::rtl::OUString sSql;
                            Reference<XPropertySet> xObjectProps(xObject,UNO_QUERY);
                            xObjectProps->getPropertyValue(PROPERTY_COMMAND) >>= sSql;
                            Reference< XMultiServiceFactory >  xFactory( pConData->xConnection, UNO_QUERY );
                            if (xFactory.is())
                            {
                                try
                                {
                                    Reference<XSingleSelectQueryAnalyzer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
                                    if ( xAnalyzer.is() )
                                    {
                                        xAnalyzer->setQuery(sSql);
                                        Reference<XParametersSupplier> xParSup(xAnalyzer,UNO_QUERY);
                                        if ( xParSup->getParameters()->getCount() > 0 )
                                        {
                                            String sFilter = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE "));
                                            sFilter = sFilter + xAnalyzer->getFilter();
                                            String sReplace(sSql);
                                            sReplace.SearchAndReplace(sFilter,String());
                                            xAnalyzer->setQuery(sReplace);
                                            Reference<XSingleSelectQueryComposer> xComposer(xAnalyzer,UNO_QUERY);
                                            xComposer->setFilter(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0=1")));
                                            aName = xAnalyzer->getQuery();
                                            nCommandType = CommandType::COMMAND;
                                        }
                                    }
                                }
                                catch (Exception&)
                                {
                                }
                            }
                        }
                    }
                }
            }

            String sDataSourceName( getDataSourceAcessor( pConnection ) );
            if ( implLoadAnything( sDataSourceName, aName, nCommandType, sal_True, pConData->xConnection ) )
                // set the title of the beamer
                ;/*updateTitle();*/
            else
            {   // clean up
                criticalFail();
            }
        }
        catch(const SQLException& e)
        {
            showError(SQLExceptionInfo(e));
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
        catch(WrappedTargetException& e)
        {
            SQLException aSql;
            if(e.TargetException >>= aSql)
                showError(SQLExceptionInfo(aSql));
            else
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser::OnSelectEntry: something strange happended!");
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
        catch(Exception&)
        {
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
    }
    return 0L;
}

// -----------------------------------------------------------------------------
SvLBoxEntry* SbaTableQueryBrowser::getEntryFromContainer(const Reference<XNameAccess>& _rxNameAccess)
{
    DBTreeListBox* pListBox = m_pTreeView->getListBox();
    SvLBoxEntry* pContainer = NULL;
    if ( pListBox )
    {
        SvLBoxEntry* pDSLoop = pListBox->FirstChild(NULL);
        while (pDSLoop)
        {
            pContainer  = pListBox->GetEntry(pDSLoop, CONTAINER_QUERIES);
            DBTreeListModel::DBTreeListUserData* pQueriesData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());
            if ( pQueriesData && pQueriesData->xContainer == _rxNameAccess )
                break;

            pContainer  = pListBox->GetEntry(pDSLoop, CONTAINER_TABLES);
            DBTreeListModel::DBTreeListUserData* pTablesData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());
            if ( pTablesData && pTablesData->xContainer == _rxNameAccess )
                break;

            pDSLoop     = pListBox->NextSibling(pDSLoop);
            pContainer  = NULL;
        }
    }
    return pContainer;
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aEntryMutex);

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // first search for a definition container where we can insert this element

    SvLBoxEntry* pEntry = getEntryFromContainer(xNames);
    if(pEntry)  // found one
    {
        // insert the new entry into the tree
        DBTreeListModel::DBTreeListUserData* pContainerData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntry->GetUserData());
        OSL_ENSURE(pContainerData, "elementInserted: There must be user data for this type!");

        DBTreeListModel::DBTreeListUserData* pNewData = new DBTreeListModel::DBTreeListUserData;
        sal_Bool bIsTable = etTableContainer == pContainerData->eType;
        if ( bIsTable )
        {
            _rEvent.Element >>= pNewData->xObjectProperties;// remember the new element
            pNewData->eType = etTableOrView;
        }
        else
        {
            if ((sal_Int32)m_pTreeView->getListBox()->GetChildCount(pEntry) < ( xNames->getElementNames().getLength() - 1 ) )
            {
                // the item inserts its children on demand, but it has not been expanded yet. So ensure here and
                // now that it has all items
                populateTree(xNames, pEntry, etQuery );
            }
            pNewData->eType = etQuery;
        }
        implAppendEntry( pEntry, ::comphelper::getString( _rEvent.Accessor ), pNewData, pNewData->eType );
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a new datasource has been added to the context
        // the name of the new ds
        ::rtl::OUString sNewDS;
        _rEvent.Accessor >>= sNewDS;

        // add new entries to the list box model
        Image a, b, c;  // not interested in  reusing them
        String e, f;
        implAddDatasource( sNewDS, a, e, b, f, c, SharedConnection() );
    }
    else
        SbaXDataBrowserController::elementInserted(_rEvent);
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::isCurrentlyDisplayedChanged(const String& _sName,SvLBoxEntry* _pContainer)
{
    return m_pCurrentlyDisplayed
            &&  getEntryType(m_pCurrentlyDisplayed) == getChildType(_pContainer)
            &&  m_pTreeView->getListBox()->GetParent(m_pCurrentlyDisplayed) == _pContainer
            &&  m_pTreeView->getListBox()->GetEntryText(m_pCurrentlyDisplayed) == _sName;
}
// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aEntryMutex);


    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // get the top-level representing the removed data source
    // and search for the queries and tables
    SvLBoxEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    { // a query or table has been removed
        String aName = ::comphelper::getString(_rEvent.Accessor).getStr();

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   // the element displayed currently has been replaced

            // we need to remember the old value
            SvLBoxEntry* pTemp = m_pCurrentlyDisplayed;

            // unload
            unloadAndCleanup( sal_False ); // don't dispose the connection

            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pTemp->GetUserData());
            pTemp->SetUserData(NULL);
            delete pData;
                // the data could be null because we have a table which isn't correct
            m_pTreeModel->Remove(pTemp);
        }
        else
        {
            // remove the entry from the model
            SvLBoxEntry* pChild = m_pTreeModel->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox()->GetEntryText(pChild) == aName)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pChild->GetUserData());
                    pChild->SetUserData(NULL);
                    delete pData;
                    m_pTreeModel->Remove(pChild);
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }

        // maybe the object which is part of the document data source has been removed
        checkDocumentDataSource();
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a datasource has been removed from the context

        // the name
        ::rtl::OUString sNewDS;
        _rEvent.Accessor >>= sNewDS;
        String sNewDatasource = sNewDS;

        // get the top-level representing the removed data source
        SvLBoxEntry* pDSLoop = m_pTreeView->getListBox()->FirstChild(NULL);
        while (pDSLoop)
        {
            if (m_pTreeView->getListBox()->GetEntryText(pDSLoop) == sNewDatasource)
                break;

            pDSLoop = m_pTreeView->getListBox()->NextSibling(pDSLoop);
        }

        if (pDSLoop)
        {
            if (isSelected(pDSLoop))
            {   // a table or query belonging to the deleted data source is currently beeing displayed.
                OSL_ENSURE(m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) == pDSLoop, "SbaTableQueryBrowser::elementRemoved: inconsistence (1)!");
                unloadAndCleanup( sal_True );
            }
            else
                OSL_ENSURE(
                        (NULL == m_pCurrentlyDisplayed)
                    ||  (m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) != pDSLoop), "SbaTableQueryBrowser::elementRemoved: inconsistence (2)!");

            // look for user data to delete
            SvTreeEntryList* pList = m_pTreeModel->GetChildList(pDSLoop);
            if(pList)
            {
                SvLBoxEntry* pEntryLoop = static_cast<SvLBoxEntry*>(pList->First());
                while (pEntryLoop)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
                    pEntryLoop->SetUserData(NULL);
                    delete pData;
                    pEntryLoop = static_cast<SvLBoxEntry*>(pList->Next());
                }
            }
            // remove the entry. This should remove all children, too.
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSLoop->GetUserData());
            pDSLoop->SetUserData(NULL);
            delete pData;
            m_pTreeModel->Remove(pDSLoop);
        }
        else
            DBG_ERROR("SbaTableQueryBrowser::elementRemoved: unknown datasource name!");

        // maybe the object which is part of the document data source has been removed
        checkDocumentDataSource();
    }
    else
        SbaXDataBrowserController::elementRemoved(_rEvent);
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aEntryMutex);

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    SvLBoxEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    {    // a table or query as been replaced
        String aName = ::comphelper::getString(_rEvent.Accessor).getStr();

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   // the element displayed currently has been replaced

            // we need to remember the old value
            SvLBoxEntry* pTemp = m_pCurrentlyDisplayed;
            unloadAndCleanup( sal_False ); // don't dispose the connection

            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pTemp->GetUserData());
            if (pData)
            {
                if ( etTableOrView == pData->eType )
                { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                     _rEvent.Element >>= pData->xObjectProperties;  // remember the new element
                }
                else
                {
                    pTemp->SetUserData(NULL);
                    delete pData;
                }
            }
        }
        else
        {
            // find the entry for this name
            SvLBoxEntry* pChild = m_pTreeModel->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox()->GetEntryText(pChild) == aName)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pChild->GetUserData());
                    if (pData)
                    {
                        if ( etTableOrView == pData->eType )
                        { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                            _rEvent.Element >>= pData->xObjectProperties;   // remember the new element
                        }
                        else
                        {
                            pChild->SetUserData(NULL);
                            delete pData;
                        }
                    }
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }

        // maybe the object which is part of the document data source has been removed
        checkDocumentDataSource();
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a datasource has been replaced in the context
        DBG_ERROR("SbaTableQueryBrowser::elementReplaced: no support for replaced data sources!");
            // very suspicious: the database context should not allow to replace data source, only to register
            // and revoke them
    }
    else
        SbaXDataBrowserController::elementReplaced(_rEvent);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::impl_releaseConnection( SharedConnection& _rxConnection )
{
    // remove as event listener
    Reference< XComponent > xComponent( _rxConnection, UNO_QUERY );
    if ( xComponent.is() )
    {
        Reference< XEventListener > xListener( static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY );
        xComponent->removeEventListener( xListener );
    }

    try
    {
        // temporary (hopefully!) hack for #i55274#
        Reference< XFlushable > xFlush( _rxConnection, UNO_QUERY );
        if ( xFlush.is() )
            xFlush->flush();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // clear
    _rxConnection.clear();
        // will implicitly dispose if we have the ownership, since xConnection is a SharedConnection
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::disposeConnection( SvLBoxEntry* _pDSEntry )
{
    DBG_ASSERT( _pDSEntry, "SbaTableQueryBrowser::disposeConnection: invalid entry (NULL)!" );
    DBG_ASSERT( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::disposeConnection: invalid entry (not top-level)!" );

    if ( _pDSEntry )
    {
        DBTreeListModel::DBTreeListUserData* pTreeListData = static_cast< DBTreeListModel::DBTreeListUserData* >( _pDSEntry->GetUserData() );
        if ( pTreeListData )
            impl_releaseConnection( pTreeListData->xConnection );
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::closeConnection(SvLBoxEntry* _pDSEntry,sal_Bool _bDisposeConnection)
{
    DBG_ASSERT(_pDSEntry, "SbaTableQueryBrowser::closeConnection: invalid entry (NULL)!");
    DBG_ASSERT( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::closeConnection: invalid entry (not top-level)!");

    // if one of the entries of the given DS is displayed currently, unload the form
    if (m_pCurrentlyDisplayed && (m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) == _pDSEntry))
        unloadAndCleanup(_bDisposeConnection);

    // collapse the query/table container
    for (SvLBoxEntry* pContainers = m_pTreeModel->FirstChild(_pDSEntry); pContainers; pContainers= m_pTreeModel->NextSibling(pContainers))
    {
        SvLBoxEntry* pElements = m_pTreeModel->FirstChild(pContainers);
        if ( pElements )
            m_pTreeView->getListBox()->Collapse(pContainers);
        m_pTreeView->getListBox()->EnableExpandHandler(pContainers);
        // and delete their children (they are connection-relative)
        for (; pElements; )
        {
            SvLBoxEntry* pRemove = pElements;
            pElements= m_pTreeModel->NextSibling(pElements);
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pRemove->GetUserData());
            pRemove->SetUserData(NULL);
            delete pData;
            m_pTreeModel->Remove(pRemove);
        }
    }
    // collapse the entry itself
    m_pTreeView->getListBox()->Collapse(_pDSEntry);

    // dispose/reset the connection
    if ( _bDisposeConnection )
        disposeConnection( _pDSEntry );
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::unloadAndCleanup( sal_Bool _bDisposeConnection )
{
    if (!m_pCurrentlyDisplayed)
        // nothing to do
        return;

    SvLBoxEntry* pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed);

    // de-select the path for the currently displayed table/query
    if (m_pCurrentlyDisplayed)
    {
        selectPath(m_pCurrentlyDisplayed, sal_False);
    }
    m_pCurrentlyDisplayed = NULL;

    try
    {
        // get the active connection. We need to dispose it.
        Reference< XPropertySet > xRowSetProps(getRowSet(),UNO_QUERY);
        Reference< XConnection > xConn;
        xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION) >>= xConn;
#if OSL_DEBUG_LEVEL > 1
        {
            Reference< XComponent > xComp;
            ::cppu::extractInterface(xComp, xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION));
        }
#endif

        // unload the form
        Reference< XLoadable > xLoadable = getLoadable();
        if (xLoadable->isLoaded())
            xLoadable->unload();

        // clear the grid control
        Reference< XNameContainer > xConta(getControlModel(),UNO_QUERY);
        clearGridColumns(xConta);

        // dispose the connection
        if(_bDisposeConnection)
            disposeConnection( pDSEntry );
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(WrappedTargetException& e)
    {
        SQLException aSql;
        if(e.TargetException >>= aSql)
            showError(SQLExceptionInfo(aSql));
        else
            OSL_ENSURE(sal_False, "SbaTableQueryBrowser::unloadAndCleanup: something strange happended!");
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "SbaTableQueryBrowser::unloadAndCleanup: could not reset the form");
    }
}

// -------------------------------------------------------------------------
namespace
{
    Reference< XInterface > lcl_getDataSource( const Reference< XNameAccess >& _rxDatabaseContext,
        const ::rtl::OUString& _rDataSourceName, const Reference< XConnection >& _rxConnection )
    {
        Reference< XDataSource > xDataSource;
        try
        {
            if ( _rDataSourceName.getLength() && _rxDatabaseContext->hasByName( _rDataSourceName ) )
                xDataSource.set( _rxDatabaseContext->getByName( _rDataSourceName ), UNO_QUERY_THROW );

            if ( !xDataSource.is() )
            {
                Reference< XChild > xConnAsChild( _rxConnection, UNO_QUERY );
                if ( xConnAsChild.is() )
                    xDataSource.set( xConnAsChild->getParent(), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xDataSource;
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::impl_initialize()
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // first initialize the parent
    SbaXDataBrowserController::impl_initialize();

    Reference<XConnection> xForeignConnection;
    Reference< XFrame > xFrame;

    ::rtl::OUString aTableName, aCatalogName, aSchemaName;

    sal_Bool bEsacpeProcessing = sal_True;
    sal_Int32 nInitialDisplayCommandType = CommandType::COMMAND;
    ::rtl::OUString sInitialDataSourceName;
    ::rtl::OUString sInitialCommand;

    const NamedValueCollection& rArguments( getInitParams() );

    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_DATASOURCENAME, sInitialDataSourceName );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_COMMAND_TYPE, nInitialDisplayCommandType );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_COMMAND, sInitialCommand );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_ACTIVE_CONNECTION, xForeignConnection );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_UPDATE_CATALOGNAME, aCatalogName );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_UPDATE_SCHEMANAME, aSchemaName );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_UPDATE_TABLENAME, aTableName );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_ESCAPE_PROCESSING, bEsacpeProcessing );
    rArguments.get_ensureType( "Frame", xFrame );
    rArguments.get_ensureType( (::rtl::OUString)PROPERTY_SHOWMENU, m_bShowMenu );

    // disable the browser if either of ShowTreeViewButton (compatibility name) or EnableBrowser
    // is present and set to FALSE
    sal_Bool bDisableBrowser =  ( sal_False == rArguments.getOrDefault( "ShowTreeViewButton", sal_True ) )   // compatibility name
                            ||  ( sal_False == rArguments.getOrDefault( (::rtl::OUString)PROPERTY_ENABLE_BROWSER, sal_True ) );
    OSL_ENSURE( !rArguments.has( "ShowTreeViewButton" ),
        "SbaTableQueryBrowser::impl_initialize: ShowTreeViewButton is superseded by EnableBrowser!" );
    m_bEnableBrowser = !bDisableBrowser;

    // hide the tree view it is disabled in general, or if the settings tell to hide it initially
    sal_Bool bHideTreeView =    ( !m_bEnableBrowser )
                            ||  ( sal_False == rArguments.getOrDefault( "ShowTreeView", sal_True ) )  // compatibility name
                            ||  ( sal_False == rArguments.getOrDefault( (::rtl::OUString)PROPERTY_SHOW_BROWSER, sal_True ) );
    OSL_ENSURE( !rArguments.has( "ShowTreeView" ),
        "SbaTableQueryBrowser::impl_initialize: ShowTreeView is superseded by ShowBrowser!" );

    if ( bHideTreeView )
        hideExplorer();
    else
        showExplorer();

    if ( m_bPreview )
    {
        try
        {
            Sequence< ::rtl::OUString> aProperties(5);
            Sequence< Any> aValues(5);

            ::rtl::OUString* pStringIter = aProperties.getArray();
            Any* pValueIter = aValues.getArray();
            *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AlwaysShowCursor"));
            *pValueIter++   <<= sal_False;
            *pStringIter++  = PROPERTY_BORDER;
            *pValueIter++   <<= sal_Int16(0);

            *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasNavigationBar"));
            *pValueIter++       <<= sal_False;
            *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasRecordMarker"));
            *pValueIter++       <<= sal_False;

            *pStringIter++  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tabstop"));
            *pValueIter++       <<= sal_False;

            Reference< XMultiPropertySet >  xFormMultiSet(getFormComponent(), UNO_QUERY);
            if ( xFormMultiSet.is() )
                xFormMultiSet->setPropertyValues(aProperties, aValues);
        }
        catch(Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // are we loaded into a (sub)frame of an embedded document (i.e. a form belonging to a database
    // document)?
    sal_Bool bSubFrameOfEmbeddedDocument = sal_False;
    if ( xFrame.is() )
    {
        Reference<XFramesSupplier> xSup = xFrame->getCreator();
        Reference<XController> xCont = xSup.is() ? xSup->getController() : Reference<XController>();

        bSubFrameOfEmbeddedDocument = xCont.is() && ::dbtools::isEmbeddedInDatabase( xCont->getModel(), xForeignConnection );
    }

    // if we have a connection at this point, it was either passed from outside, our
    // determined from a outer DB document. In both cases, do not dispose it later on.
    SharedConnection xConnection( xForeignConnection, SharedConnection::NoTakeOwnership );

    // should we display all registered databases in the left hand side tree?
    // or only *one* special?
    sal_Bool bLimitedTreeEntries = sal_False;
    // if we're part of a frame which is a secondary frame of a database document, then only
    // display the database for this document, not all registered ones
    bLimitedTreeEntries |= bSubFrameOfEmbeddedDocument;
    // if the tree view is not to be displayed at all, then only display the data source
    // which was given as initial selection
    bLimitedTreeEntries |= ( m_bEnableBrowser != sal_True );

    if ( bLimitedTreeEntries )
    {
        if ( xConnection.is() )
        {
            startConnectionListening( xConnection );

            // if no initial name was given, try to obtain one from the data source
            if ( !sInitialDataSourceName.getLength() )
            {
                Reference< XChild > xChild( xConnection, UNO_QUERY );
                Reference< XPropertySet > xDataSourceProperties;
                if ( xChild.is() )
                    xDataSourceProperties = xDataSourceProperties.query( xChild->getParent() );
                if ( xDataSourceProperties.is() )
                {
                    try
                    {
                        OSL_VERIFY( xDataSourceProperties->getPropertyValue( PROPERTY_NAME ) >>= sInitialDataSourceName );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "SbaTableQueryBrowser::impl_initialize: a connection parent which does not have a 'Name'!??" );
                    }
                }
            }
        }

        Image aDBImage, aQueriesImage, aTablesImage;
        String sQueriesName, sTablesName;

        implAddDatasource( sInitialDataSourceName,
            aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage, xConnection
        );
        m_pTreeView->getListBox()->Expand( m_pTreeView->getListBox()->First() );
    }
    else
        initializeTreeModel();

    if ( m_bEnableBrowser )
    {
        m_aDocScriptSupport = ::boost::optional< bool >( false );
    }
    else
    {
        // we are not used as "browser", but as mere view for a single table/query/command. In particular,
        // there is a specific database document which we belong to.
        Reference< XOfficeDatabaseDocument > xDocument( getDataSourceOrModel(
            lcl_getDataSource( m_xDatabaseContext, sInitialDataSourceName, xConnection ) ), UNO_QUERY );
        m_aDocScriptSupport = ::boost::optional< bool >( Reference< XEmbeddedScripts >( xDocument, UNO_QUERY ).is() );
    }

    // TODO: remove the following line. The current version is just to be able
    // to integrate an intermediate version of the CWS, which should behave as
    // if no macros in DB docs are allowed
    m_aDocScriptSupport = ::boost::optional< bool> ( false );

    if ( implSelect( sInitialDataSourceName, sInitialCommand, nInitialDisplayCommandType, bEsacpeProcessing, xConnection, sal_True ) )
    {
        try
        {
            Reference< XPropertySet > xRowSetProps(getRowSet(), UNO_QUERY);
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(aCatalogName));
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(aSchemaName));
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(aTableName));

        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "SbaTableQueryBrowser::impl_initialize: could not set the update related names!");
        }
    }

    InvalidateAll();
}

// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::haveExplorer() const
{
    return m_pTreeView && m_pTreeView->IsVisible();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::hideExplorer()
{
    if (!haveExplorer())
        return;
    if (!getBrowserView())
        return;

    m_pTreeView->Hide();
    m_pSplitter->Hide();
    getBrowserView()->Resize();

    InvalidateFeature(ID_BROWSER_EXPLORER);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::showExplorer()
{
    if (haveExplorer())
        return;

    if (!getBrowserView())
        return;

    m_pTreeView->Show();
    m_pSplitter->Show();
    getBrowserView()->Resize();

    InvalidateFeature(ID_BROWSER_EXPLORER);
}

// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::ensureConnection(SvLBoxEntry* _pAnyEntry, SharedConnection& _rConnection)
{
    SvLBoxEntry* pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(_pAnyEntry);
    DBTreeListModel::DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast<DBTreeListModel::DBTreeListUserData*>(pDSEntry->GetUserData())
            :   NULL;

    return ensureConnection( pDSEntry, pDSData, _rConnection );
}

// -----------------------------------------------------------------------------
::std::auto_ptr< ImageProvider > SbaTableQueryBrowser::getImageProviderFor( SvLBoxEntry* _pAnyEntry )
{
    ::std::auto_ptr< ImageProvider > pImageProvider( new ImageProvider );
    SharedConnection xConnection;
    if ( getExistentConnectionFor( _pAnyEntry, xConnection ) )
        pImageProvider.reset( new ImageProvider( xConnection ) );
    return pImageProvider;
}

// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::getExistentConnectionFor( SvLBoxEntry* _pAnyEntry, SharedConnection& _rConnection )
{
    SvLBoxEntry* pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent( _pAnyEntry );
    DBTreeListModel::DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast< DBTreeListModel::DBTreeListUserData* >( pDSEntry->GetUserData() )
            :   NULL;
    if ( pDSData )
        _rConnection = pDSData->xConnection;
    return _rConnection.is();
}

#ifdef DBG_UTIL
// -----------------------------------------------------------------------------
bool SbaTableQueryBrowser::impl_isDataSourceEntry( SvLBoxEntry* _pEntry ) const
{
    return m_pTreeModel->GetRootLevelParent( _pEntry ) == _pEntry;
}
#endif

// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::ensureConnection( SvLBoxEntry* _pDSEntry, void* pDSData, SharedConnection& _rConnection )
{
    DBG_ASSERT( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::ensureConnection: this entry does not denote a data source!" );
    if(_pDSEntry)
    {
        DBTreeListModel::DBTreeListUserData* pTreeListData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSData);
        ::rtl::OUString aDSName = GetEntryText(_pDSEntry);

        if ( pTreeListData )
            _rConnection = pTreeListData->xConnection;

        if ( !_rConnection.is() && pTreeListData )
        {
            // show the "connecting to ..." status
            String sConnecting(ModuleRes(STR_CONNECTING_DATASOURCE));
            sConnecting.SearchAndReplaceAscii("$name$", aDSName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sConnecting);

            // build a string showing context information in case of error
            String sConnectingContext( ModuleRes( STR_COULDNOTCONNECT_DATASOURCE ) );
            sConnectingContext.SearchAndReplaceAscii("$name$", aDSName);

            // connect
            _rConnection.reset(
                connect( getDataSourceAcessor( _pDSEntry ), sConnectingContext, sal_True ),
                SharedConnection::TakeOwnership
            );

            // remember the connection
            pTreeListData->xConnection = _rConnection;
        }
    }

    return _rConnection.is();
}

// -----------------------------------------------------------------------------
IMPL_LINK( SbaTableQueryBrowser, OnTreeEntryCompare, const SvSortData*, _pSortData )
{
    SvLBoxEntry* pLHS = static_cast<SvLBoxEntry*>(_pSortData->pLeft);
    SvLBoxEntry* pRHS = static_cast<SvLBoxEntry*>(_pSortData->pRight);
    DBG_ASSERT(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");
    // we want the table entry and the end so we have to do a check

    if (isContainer(pRHS))
    {
        // don't use getEntryType (directly or indirecly) for the LHS:
        // LHS is currently beeing inserted, so it is not "completely valid" at the moment

        const EntryType eRight = getEntryType(pRHS);
        if (etTableContainer == eRight)
            // every other container should be placed _before_ the bookmark container
            return -1;

        const String sLeft = m_pTreeView->getListBox()->GetEntryText(pLHS);

        EntryType eLeft = etTableContainer;
        if (String(ModuleRes(RID_STR_TABLES_CONTAINER)) == sLeft)
            eLeft = etTableContainer;
        else if (String(ModuleRes(RID_STR_QUERIES_CONTAINER)) == sLeft)
            eLeft = etQueryContainer;

        return  eLeft < eRight
            ?   COMPARE_LESS
            :       eLeft == eRight
                ?   COMPARE_EQUAL
                :   COMPARE_GREATER;
    }

    SvLBoxString* pLeftTextItem = static_cast<SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    SvLBoxString* pRightTextItem = static_cast<SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

    String sLeftText = pLeftTextItem->GetText();
    String sRightText = pRightTextItem->GetText();

    sal_Int32 nCompareResult = 0;   // equal by default

    if (m_xCollator.is())
    {
        try
        {
            nCompareResult = m_xCollator->compareString(sLeftText, sRightText);
        }
        catch(Exception&)
        {
        }
    }
    else
        // default behaviour if we do not have a collator -> do the simple string compare
        nCompareResult = sLeftText.CompareTo(sRightText);

    return nCompareResult;
}

// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::implAdministrate( SvLBoxEntry* _pApplyTo )
{
    try
    {
        // get the desktop object
        sal_Int32 nFrameSearchFlag = FrameSearchFlag::ALL | FrameSearchFlag::GLOBAL ;
        Reference< XComponentLoader > xFrameLoader(getORB()->createInstance(SERVICE_FRAME_DESKTOP),UNO_QUERY);

        if ( xFrameLoader.is() )
        {
            // the initial selection
            SvLBoxEntry* pTopLevelSelected = _pApplyTo;
            while (pTopLevelSelected && m_pTreeView->getListBox()->GetParent(pTopLevelSelected))
                pTopLevelSelected = m_pTreeView->getListBox()->GetParent(pTopLevelSelected);
            ::rtl::OUString sInitialSelection;
            if (pTopLevelSelected)
                sInitialSelection = getDataSourceAcessor( pTopLevelSelected );

            Reference< XModel > xDocumentModel(
                getDataSourceOrModel(getDataSourceByName_displayError( sInitialSelection, getView(), getORB(), true )),UNO_QUERY);

            if ( xDocumentModel.is() )
            {
                Reference< XInteractionHandler > xInteractionHandler(
                    getORB()->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.InteractionHandler" ) ) ),
                        UNO_QUERY );
                OSL_ENSURE( xInteractionHandler.is(), "SbaTableQueryBrowser::implAdministrate: no interaction handler available!" );

                ::comphelper::NamedValueCollection aLoadArgs;
                aLoadArgs.put( "Model", xDocumentModel );
                aLoadArgs.put( "InteractionHandler", xInteractionHandler );
                aLoadArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

                Sequence< PropertyValue > aLoadArgPV;
                aLoadArgs >>= aLoadArgPV;

                xFrameLoader->loadComponentFromURL(
                    xDocumentModel->getURL(),
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default")),
                    nFrameSearchFlag,
                    aLoadArgPV
                );
            }
        }
    }
    catch(::com::sun::star::uno::Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::implAdministrate: caught an exception while creating/executing the dialog!");
    }
}

// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::requestQuickHelp( const SvLBoxEntry* _pEntry, String& _rText ) const
{
    const DBTreeListModel::DBTreeListUserData* pData = static_cast< const DBTreeListModel::DBTreeListUserData* >( _pEntry->GetUserData() );
    if ( ( pData->eType == etDatasource ) && pData->sAccessor.Len() )
    {
        _rText = ::svt::OFileNotation( pData->sAccessor ).get( ::svt::OFileNotation::N_SYSTEM );
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::requestContextMenu( const CommandEvent& _rEvent )
{
    ::Point aPosition;
    SvLBoxEntry* pEntry = NULL;
    SvLBoxEntry* pOldSelection = NULL;
    if (_rEvent.IsMouseEvent())
    {
        aPosition = _rEvent.GetMousePosPixel();
        // ensure that the entry which the user clicked at is selected
        pEntry = m_pTreeView->getListBox()->GetEntry(aPosition);
        if (pEntry && !m_pTreeView->getListBox()->IsSelected(pEntry))
        {
            pOldSelection = m_pTreeView->getListBox()->FirstSelected();
            m_pTreeView->getListBox()->lockAutoSelect();
            m_pTreeView->getListBox()->Select(pEntry);
            m_pTreeView->getListBox()->unlockAutoSelect();
        }
    }
    else
    {
        // use the center of the current entry
        pEntry = m_pTreeView->getListBox()->GetCurEntry();
        OSL_ENSURE(pEntry,"No current entry!");
        aPosition = m_pTreeView->getListBox()->GetEntryPosition(pEntry);
        aPosition.X() += m_pTreeView->getListBox()->GetOutputSizePixel().Width() / 2;
        aPosition.Y() += m_pTreeView->getListBox()->GetEntryHeight() / 2;
    }

    // disable entries according to the currently selected entry

    // does the datasource which the selected entry belongs to has an open connection ?
    SvLBoxEntry* pDSEntry = NULL;
    DBTreeListModel::DBTreeListUserData* pDSData = NULL;
    DBTreeListModel::DBTreeListUserData* pEntryData = NULL;
    if(pEntry)
    {
        pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(pEntry);
        pDSData =   pDSEntry
                ?   static_cast<DBTreeListModel::DBTreeListUserData*>(pDSEntry->GetUserData())
                :   NULL;
        pEntryData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntry->GetUserData());
    }


    EntryType eType = pEntryData ? pEntryData->eType : etUnknown;

    ModuleRes nMenuRes( MENU_BROWSER_DEFAULTCONTEXT );
    PopupMenu aContextMenu( nMenuRes );
    PopupMenu* pDynamicSubMenu = NULL;

    // enable menu entries
    if (!pDSData || !pDSData->xConnection.is())
    {
        aContextMenu.EnableItem(ID_TREE_CLOSE_CONN, sal_False);
    }
    aContextMenu.EnableItem(SID_COPY,   sal_False);

    ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithServiceFactory( getORB(),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.DataAccess/ApplicationIntegration/InstalledFeatures/Common" ) ) ) );
    sal_Bool bHaveEditDatabase( sal_True );
    OSL_VERIFY( aConfig.getNodeValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EditDatabaseFromDataSourceView" ) ) ) >>= bHaveEditDatabase );
    aContextMenu.EnableItem( ID_TREE_EDIT_DATABASE, bHaveEditDatabase );

    if ( pEntry )
    {

        switch (eType)
        {
            // 1. for tables/views
            case etTableContainer:
            case etTableOrView:
            {
                aContextMenu.EnableItem( SID_COPY, etTableOrView == eType );
            }
            break;

            // 3. for queries
            case etQueryContainer:
            case etQuery:
            {
                // 3.2 actions on existing queries
                aContextMenu.EnableItem( SID_COPY, etQuery == eType );
            }
            break;
            case etDatasource:
            case etUnknown:
                break;
        }
    }

    if (!getORB().is())
        // no ORB -> no administration dialog
        aContextMenu.EnableItem(ID_TREE_EDIT_DATABASE, sal_False);

    // no disabled entries
    aContextMenu.RemoveDisabledEntries();

    USHORT nPos = aContextMenu.Execute(m_pTreeView->getListBox(), aPosition);

    delete pDynamicSubMenu;
    pDynamicSubMenu = NULL;

    // restore the old selection
    if (pOldSelection)
    {
        m_pTreeView->getListBox()->lockAutoSelect();
        m_pTreeView->getListBox()->Select(pOldSelection);
        m_pTreeView->getListBox()->unlockAutoSelect();
    }

    switch (nPos)
    {
        case ID_TREE_EDIT_DATABASE:
            implAdministrate(pEntry);
            break;

        case ID_TREE_CLOSE_CONN:
            openHelpAgent(HID_DSBROWSER_DISCONNECTING);
            closeConnection(pDSEntry);
            break;

        case SID_COPY:
        {
            TransferableHelper* pTransfer = implCopyObject( pEntry, (etQuery == eType) ? CommandType::QUERY : CommandType::TABLE );
            Reference< XTransferable> aEnsureDelete = pTransfer;

            if (pTransfer)
                pTransfer->CopyToClipboard(getView());
        }
        break;

        case ID_TREE_ADMINISTRATE:
            ::svx::administrateDatabaseRegistration( getView() );
            break;
    }

    return sal_True;    // handled
}
// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::implGetQuerySignature( ::rtl::OUString& _rCommand, sal_Bool& _bEscapeProcessing )
{
    _rCommand = ::rtl::OUString();
    _bEscapeProcessing = sal_False;

    try
    {
        // ontain the dss (data source signature) of the form
        ::rtl::OUString sDataSourceName;
        ::rtl::OUString sCommand;
        sal_Int32       nCommandType = CommandType::COMMAND;
        Reference< XPropertySet > xRowsetProps( getRowSet(), UNO_QUERY );
        ODataAccessDescriptor aDesc( xRowsetProps );
        sDataSourceName = aDesc.getDataSource();
        aDesc[ daCommand ]      >>= sCommand;
        aDesc[ daCommandType ]  >>= nCommandType;

        // do we need to do anything?
        if ( CommandType::QUERY != nCommandType )
            return sal_False;

        // get the query object
        Reference< XQueryDefinitionsSupplier > xSuppQueries;
        Reference< XNameAccess > xQueries;
        Reference< XPropertySet > xQuery;
        m_xDatabaseContext->getByName( sDataSourceName ) >>= xSuppQueries;
        if ( xSuppQueries.is() )
            xQueries = xSuppQueries->getQueryDefinitions();
        if ( xQueries.is() )
            xQueries->getByName( sCommand ) >>= xQuery;
        OSL_ENSURE( xQuery.is(), "SbaTableQueryBrowser::implGetQuerySignature: could not retrieve the query object!" );

        // get the two properties we need
        if ( xQuery.is() )
        {
            xQuery->getPropertyValue( PROPERTY_COMMAND ) >>= _rCommand;
            _bEscapeProcessing = ::cppu::any2bool( xQuery->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) );
            return sal_True;
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "SbaTableQueryBrowser::implGetQuerySignature: caught an exception!" );
    }

    return sal_False;
}
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    if (aEvent.Frame == m_xCurrentFrameParent)
    {
        if(aEvent.Action == FrameAction_COMPONENT_DETACHING)
            implRemoveStatusListeners();
        else if (aEvent.Action == FrameAction_COMPONENT_REATTACHED)
            connectExternalDispatches();
    }
    else
        SbaXDataBrowserController::frameAction(aEvent);

}
// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::clearGridColumns(const Reference< XNameContainer >& _xColContainer)
{
    // first we have to clear the grid
    Sequence< ::rtl::OUString > aNames = _xColContainer->getElementNames();
    const ::rtl::OUString* pIter    = aNames.getConstArray();
    const ::rtl::OUString* pEnd     = pIter + aNames.getLength();
    Reference< XInterface > xColumn;
    for (; pIter != pEnd;++pIter)
    {
        _xColContainer->getByName(*pIter) >>= xColumn;
        _xColContainer->removeByName(*pIter);
        ::comphelper::disposeComponent(xColumn);
    }
}
// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::isHiContrast() const
{
    sal_Bool bRet = sal_False;
    if ( m_pTreeView )
        bRet = m_pTreeView->getListBox()->GetBackground().GetColor().IsDark();
    return bRet;
}
// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::loadMenu(const Reference< XFrame >& _xFrame)
{
    if ( m_bShowMenu )
    {
        OGenericUnoController::loadMenu(_xFrame);
    }
    else if ( !m_bPreview )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);

        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            xLayoutManager->createElement( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/toolbar" )));
            xLayoutManager->unlock();
            xLayoutManager->doLayout();
        }
        onLoadedMenu( xLayoutManager );
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString SbaTableQueryBrowser::getPrivateTitle() const
{
    ::rtl::OUString sTitle;
    if ( m_pCurrentlyDisplayed )
    {
        SvLBoxEntry* pContainer = m_pTreeModel->GetParent(m_pCurrentlyDisplayed);
        // get the entry for the datasource
        SvLBoxEntry* pConnection = m_pTreeModel->GetParent(pContainer);
        ::rtl::OUString sName = m_pTreeView->getListBox()->GetEntryText(m_pCurrentlyDisplayed);
        sTitle = GetEntryText( pConnection );
        INetURLObject aURL(sTitle);
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            sTitle = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_WITH_CHARSET);
        if ( sName.getLength() )
        {
            sName += ::rtl::OUString::createFromAscii(" - ");
            sName += sTitle;
            sTitle = sName;
        }
    }

    return sTitle;
}
// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::preReloadForm()
{
    sal_Bool bIni = sal_False;
    if ( !m_pCurrentlyDisplayed )
    {
        // switch the grid to design mode while loading
        getBrowserView()->getGridControl()->setDesignMode(sal_True);
        // we had an invalid statement so we need to connect the column models
        Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
        ::svx::ODataAccessDescriptor aDesc(xRowSetProps);
        // extract the props
        ::rtl::OUString sDataSource;
        ::rtl::OUString sCommand;
        sal_Int32 nCommandType = CommandType::COMMAND;
        sal_Bool bEscapeProcessing = sal_True;
        extractDescriptorProps(aDesc, sDataSource, sCommand, nCommandType, bEscapeProcessing);
        if ( sDataSource.getLength() && sCommand.getLength() && (-1 != nCommandType) )
        {
            SvLBoxEntry* pDataSource = NULL;
            SvLBoxEntry* pCommandType = NULL;
            m_pCurrentlyDisplayed = getObjectEntry( sDataSource, sCommand, nCommandType, &pDataSource, &pCommandType, sal_True, SharedConnection() );
            bIni = sal_True;
        }
    }
    return bIni;
}

// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::postReloadForm()
{
    InitializeGridModel(getFormComponent());
    LoadFinished(sal_True);
    //updateTitle();
}

//------------------------------------------------------------------------------
Reference< XEmbeddedScripts > SAL_CALL SbaTableQueryBrowser::getScriptContainer() throw (RuntimeException)
{
    Reference< XEmbeddedScripts > xScripts( m_xCurrentDatabaseDocument, UNO_QUERY );
    OSL_ENSURE( xScripts.is() || !m_xCurrentDatabaseDocument.is(),
        "SbaTableQueryBrowser::getScriptContainer: invalid database document!" );
    return xScripts;
}

// .........................................................................
}   // namespace dbaui
// .........................................................................


