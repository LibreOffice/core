/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selector.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 15:00:30 $
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
#include "precompiled_svx.hxx"

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#include "selector.hxx"
#include "dialmgr.hxx"

#include "selector.hrc"
#include "fmresids.hrc"
#include "dialogs.hrc"

#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/macrconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/XBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>

#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/DispatchInformation.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>

#define PRTSTR(x) rtl::OUStringToOString(x, RTL_TEXTENCODING_ASCII_US).pData->buffer

SV_IMPL_PTRARR(SvxGroupInfoArr_Impl, SvxGroupInfoPtr);

class TestOUString : public OUString
{
public:

    TestOUString() : OUString()
    { OSL_TRACE("Creating empty TestOUString"); }

    TestOUString( OUString value ) : OUString( value )
    { OSL_TRACE("Creating nontempty OUString: %s", PRTSTR(value)); }

    ~TestOUString()
    { OSL_TRACE("Destroying TestOUString: %s", PRTSTR(*this)); }
};

class ScriptInfo
{
private:

    OUString m_sURI;
    OUString m_sDescription;

public:

    ScriptInfo( const OUString& uri )
        : m_sURI( uri ) {}
    ~ScriptInfo() {}

    void SetHelpText( const OUString& description )
    {
        if ( description.getLength() != 0 )
        {
            m_sDescription = description.copy(0);
        }
    };
    const OUString& GetHelpText() const { return m_sDescription; }
    const OUString& GetURI() const { return m_sURI; }
};

/*
 * The implementations of SvxConfigFunctionListBox_Impl and
 * SvxConfigGroupListBox_Impl are copied from sfx2/source/dialog/cfg.cxx
 */
SvxConfigFunctionListBox_Impl::SvxConfigFunctionListBox_Impl( Window* pParent, const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
    , pCurEntry( 0 )
    , m_pDraggingEntry( 0 )
{
    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    GetModel()->SetSortMode( SortAscending );

    // Timer f"ur die BallonHelp
    aTimer.SetTimeout( 200 );
    aTimer.SetTimeoutHdl(
        LINK( this, SvxConfigFunctionListBox_Impl, TimerHdl ) );
}

SvxConfigFunctionListBox_Impl::~SvxConfigFunctionListBox_Impl()
{
    ClearAll();
}

SvLBoxEntry* SvxConfigFunctionListBox_Impl::GetLastSelectedEntry()
{
    if ( m_pDraggingEntry != NULL )
    {
        return m_pDraggingEntry;
    }
    else
    {
        return FirstSelected();
    }
}

void SvxConfigFunctionListBox_Impl::MouseMove( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    pCurEntry = GetCurEntry();

    if ( pCurEntry && GetEntry( aMousePos ) == pCurEntry )
        aTimer.Start();
    else
    {
        Help::ShowBalloon( this, aMousePos, String() );
        aTimer.Stop();
    }
}


IMPL_LINK( SvxConfigFunctionListBox_Impl, TimerHdl, Timer*, EMPTYARG)
{
    aTimer.Stop();
    Point aMousePos = GetPointerPosPixel();
    SvLBoxEntry *pEntry = GetCurEntry();
    if ( pEntry && GetEntry( aMousePos ) == pEntry && pCurEntry == pEntry )
        Help::ShowBalloon( this, OutputToScreenPixel( aMousePos ), GetHelpText( pEntry ) );
    return 0L;
}

void SvxConfigFunctionListBox_Impl::ClearAll()
{
    USHORT nCount = aArr.Count();
    for ( USHORT i=0; i<nCount; i++ )
    {
        SvxGroupInfo_Impl *pData = aArr[i];

        if ( pData->nKind == SVX_CFGFUNCTION_SLOT && pData->pObject != NULL )
        {
            delete (OUString*)pData->pObject;
        }

        if ( pData->nKind == SVX_CFGFUNCTION_SCRIPT )
        {
            ScriptInfo *pInfo = (ScriptInfo*) pData->pObject;
            if ( pInfo != NULL )
            {
                delete pInfo;
            }
        }

        if ( pData->nKind == SVX_CFGGROUP_SCRIPTCONTAINER )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }

        delete pData;
    }

    aArr.Remove( 0, nCount );
    Clear();
}

SvLBoxEntry* SvxConfigFunctionListBox_Impl::GetEntry_Impl( const String& rName )
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        if ( GetEntryText( pEntry ) == rName )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

SvLBoxEntry* SvxConfigFunctionListBox_Impl::GetEntry_Impl( USHORT nId )
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        SvxGroupInfo_Impl *pData = (SvxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && pData->nOrd == nId )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

USHORT SvxConfigFunctionListBox_Impl::GetId( SvLBoxEntry *pEntry )
{
    SvxGroupInfo_Impl *pData = pEntry ?
        (SvxGroupInfo_Impl*) pEntry->GetUserData() : 0;
    if ( pData )
        return pData->nOrd;
    return 0;
}

String SvxConfigFunctionListBox_Impl::GetHelpText( SvLBoxEntry *pEntry )
{
    // Information zum selektierten Entry aus den Userdaten holen
    SvxGroupInfo_Impl *pInfo =
        pEntry ? (SvxGroupInfo_Impl*) pEntry->GetUserData(): 0;

    if ( pInfo )
    {
        if ( pInfo->nKind == SVX_CFGFUNCTION_SLOT )
        {
            OUString* aCmdURL = (OUString*) pInfo->pObject;

            OUString aHelpText =
                Application::GetHelp()->GetHelpText( *aCmdURL, this );

            return aHelpText;
        }
        else if ( pInfo->nKind == SVX_CFGFUNCTION_SCRIPT )
        {
            ScriptInfo* pScriptInfo = (ScriptInfo*) pInfo->pObject;
            return pScriptInfo->GetHelpText();
        }
    }

    return String();
}

void SvxConfigFunctionListBox_Impl::FunctionSelected()
{
    Help::ShowBalloon( this, Point(), String() );
}

// drag and drop support
DragDropMode SvxConfigFunctionListBox_Impl::NotifyStartDrag(
    TransferDataContainer& /*aTransferDataContainer*/, SvLBoxEntry* pEntry )
{
    m_pDraggingEntry = pEntry;
    return GetDragDropMode();
}

void SvxConfigFunctionListBox_Impl::DragFinished( sal_Int8 /*nDropAction*/ )
{
    m_pDraggingEntry = NULL;
}

sal_Int8
SvxConfigFunctionListBox_Impl::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

SvxConfigGroupListBox_Impl::SvxConfigGroupListBox_Impl(
    Window* pParent, const ResId& rResId,
    ULONG nConfigMode, const Reference< frame::XFrame >& xFrame )
        : SvTreeListBox( pParent, rResId )
        , nMode( nConfigMode ), bShowSF( TRUE ),
    m_hdImage(ResId(IMG_HARDDISK)),
    m_hdImage_hc(ResId(IMG_HARDDISK_HC)),
    m_libImage(ResId(IMG_LIB)),
    m_libImage_hc(ResId(IMG_LIB_HC)),
    m_macImage(ResId(IMG_MACRO)),
    m_macImage_hc(ResId(IMG_MACRO_HC)),
    m_docImage(ResId(IMG_DOC)),
    m_docImage_hc(ResId(IMG_DOC_HC)),
    m_sMyMacros(String(ResId(STR_MYMACROS))),
    m_sProdMacros(String(ResId(STR_PRODMACROS)))
{
    FreeResource();

    if ( xFrame != NULL )
    {
        m_xFrame.set( xFrame );
    }

    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );

    ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );

    SetNodeBitmaps(
        aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
        aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE ),
        BMP_COLOR_NORMAL );

    SetNodeBitmaps(
        aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
        aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE ),
        BMP_COLOR_HIGHCONTRAST );
}


SvxConfigGroupListBox_Impl::~SvxConfigGroupListBox_Impl()
{
    ClearAll();
}

void SvxConfigGroupListBox_Impl::ClearAll()
{
    USHORT nCount = aArr.Count();
    for ( USHORT i=0; i<nCount; i++ )
    {
        SvxGroupInfo_Impl *pData = aArr[i];
        if ( pData->nKind == SVX_CFGGROUP_SCRIPTCONTAINER )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }
        delete pData;
    }

    aArr.Remove( 0, nCount );
    Clear();
}

void SvxConfigGroupListBox_Impl::Init( SvStringsDtor * )
{
    SetUpdateMode(FALSE);
    ClearAll();

    Reference< XComponentContext > xContext;
    Reference < beans::XPropertySet > xProps(
        ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );

    xContext.set( xProps->getPropertyValue(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))),
        UNO_QUERY );

    // are we showing builtin commands?
    if ( nMode && xContext.is() && m_xFrame.is() )
    {
        Reference< lang::XMultiComponentFactory > xMCF =
            xContext->getServiceManager();

        Reference< frame::XDispatchInformationProvider > xDIP(
            m_xFrame, UNO_QUERY );

        Reference< ::com::sun::star::frame::XModuleManager >
            xModuleManager( xMCF->createInstanceWithContext(
                OUString::createFromAscii(
                    "com.sun.star.frame.ModuleManager" ),
                xContext ),
            UNO_QUERY );

        OUString aModuleId;
        try{
            aModuleId = xModuleManager->identify( m_xFrame );
        }catch(const uno::Exception&)
            { aModuleId = ::rtl::OUString(); }

        Reference< container::XNameAccess > xNameAccess(
            xMCF->createInstanceWithContext(
                OUString::createFromAscii(
                    "com.sun.star.frame.UICommandDescription" ),
                xContext ),
            UNO_QUERY );

        if ( xNameAccess.is() )
        {
            xNameAccess->getByName( aModuleId ) >>= m_xModuleCommands;
        }

        Reference< container::XNameAccess > xAllCategories(
            xMCF->createInstanceWithContext(
                OUString::createFromAscii(
                    "com.sun.star.ui.UICategoryDescription" ),
                xContext ),
            UNO_QUERY );

        Reference< container::XNameAccess > xModuleCategories;
        if ( xAllCategories.is() )
        {
            if ( aModuleId.getLength() != 0 )
            {
                try
                {
                    xModuleCategories = Reference< container::XNameAccess >(
                           xAllCategories->getByName( aModuleId ), UNO_QUERY );
                }
                catch ( container::NoSuchElementException& )
                {
                }
            }

            if ( !xModuleCategories.is() )
            {
                xModuleCategories = xAllCategories;
            }
        }

        if ( xModuleCategories.is() )
        {
            Sequence< sal_Int16 > gids =
                xDIP->getSupportedCommandGroups();

            for ( sal_Int32 i = 0; i < gids.getLength(); i++ )
            {
                Sequence< frame::DispatchInformation > commands;
                try
                {
                    commands =
                        xDIP->getConfigurableDispatchInformation( gids[i] );
                }
                catch ( container::NoSuchElementException& )
                {
                    continue;
                }

                if ( commands.getLength() == 0 )
                {
                    continue;
                }

                sal_Int32 gid = gids[i];
                OUString idx = OUString::valueOf( gid );
                OUString group = idx;
                try
                {
                    xModuleCategories->getByName( idx ) >>= group;
                }
                catch ( container::NoSuchElementException& )
                {
                }

                SvLBoxEntry *pEntry = InsertEntry( group, NULL );

                SvxGroupInfo_Impl *pInfo =
                    new SvxGroupInfo_Impl( SVX_CFGGROUP_FUNCTION, gids[i] );
                aArr.Insert( pInfo, aArr.Count() );

                pEntry->SetUserData( pInfo );
            }
        }
    }

    if ( bShowSF && xContext.is() )
    {
        // Add Scripting Framework entries
        Reference< browse::XBrowseNode > rootNode;
        Reference< XComponentContext> xCtx;

        try
        {
            Reference < beans::XPropertySet > _xProps(
                ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
            xCtx.set( _xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), UNO_QUERY_THROW );
            Reference< browse::XBrowseNodeFactory > xFac( xCtx->getValueByName(
                OUString::createFromAscii( "/singletons/com.sun.star.script.browse.theBrowseNodeFactory") ), UNO_QUERY_THROW );
            rootNode.set( xFac->createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
        }
        catch( Exception& )
        {
            // OSL_TRACE("Exception getting root node: %s", PRTSTR(e.Message));
            // TODO exception handling
        }

        if ( rootNode.is() )
        {
            if ( nMode )
            {
            /*
                We call acquire on the XBrowseNode so that it does not
                get autodestructed and become invalid when accessed later.
            */
            rootNode->acquire();

            SvxGroupInfo_Impl *pInfo =
                new SvxGroupInfo_Impl( SVX_CFGGROUP_SCRIPTCONTAINER, 0,
                    static_cast<void *>(rootNode.get()));

            String aTitle =
                String( ResId( STR_SELECTOR_MACROS, DIALOG_MGR() ) );

            SvLBoxEntry *pNewEntry = InsertEntry( aTitle, NULL );
            pNewEntry->SetUserData( pInfo );
            pNewEntry->EnableChildsOnDemand( TRUE );
            aArr.Insert( pInfo, aArr.Count() );
            }
            else
            {
                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();
                        BOOL bIsRootNode = FALSE;

                        OUString user = OUString::createFromAscii("user");
                        OUString share = OUString::createFromAscii("share");
                        if ( rootNode->getName().equals(OUString::createFromAscii("Root") ))
                        {
                            bIsRootNode = TRUE;
                        }

                        OUString sDisplayTitle;
                        OUString sModelTitle;
                        SfxObjectShell* pCurrentDoc = SfxObjectShell::GetWorkingDocument();
                        if ( pCurrentDoc )
                        {
                            sDisplayTitle = pCurrentDoc->GetTitle();
                            //sModelTitle = xModelToDocTitle( pCurrentDoc->GetModel() );
                            SvxScriptSelectorDialog::GetDocTitle( pCurrentDoc->GetModel(), sModelTitle );
                        }

                        if ( sDisplayTitle.getLength() == 0 && sModelTitle.getLength() != 0 )
                        {
                            sDisplayTitle = sModelTitle;
                        }

                        for ( long n = 0; n < children.getLength(); n++ )
                        {
                            /* To mimic current starbasic behaviour we
                            need to make sure that only the current document
                            is displayed in the config tree. Tests below
                            set the bDisplay flag to FALSE if the current
                            node is a first level child of the Root and is NOT
                            either the current document, user or share */
                            Reference< browse::XBrowseNode >& theChild = children[n];
                            ::rtl::OUString uiName = theChild->getName();
                            BOOL bDisplay = TRUE;

                            if ( bIsRootNode )
                            {
                                if ( uiName.equals( sModelTitle ) )
                                {
                                    uiName = sDisplayTitle;
                                }
                                else if ( uiName.equals( user ) )
                                {
                                    uiName = m_sMyMacros;
                                }
                                else if ( uiName.equals( share ) )
                                {
                                    uiName = m_sProdMacros;
                                }
                                else
                                {
                                    bDisplay = FALSE;
                                }
                            }
                            if (children[n]->getType() != browse::BrowseNodeTypes::SCRIPT  && bDisplay )
                            {

                                /*
                                    We call acquire on the XBrowseNode so that it does not
                                    get autodestructed and become invalid when accessed later.
                                */
                                theChild->acquire();

                                SvxGroupInfo_Impl* pInfo =
                                    new SvxGroupInfo_Impl(SVX_CFGGROUP_SCRIPTCONTAINER,
                                        0, static_cast<void *>( theChild.get()));

                                Image aImage = GetImage( theChild, xContext, bIsRootNode,BMP_COLOR_NORMAL );
                                SvLBoxEntry* pNewEntry =
                                    InsertEntry( uiName, NULL);
                                SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                                SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                                aImage = GetImage( theChild, xContext, bIsRootNode,BMP_COLOR_HIGHCONTRAST );
                                SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);
                                SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);

                                pNewEntry->SetUserData( pInfo );
                                aArr.Insert( pInfo, aArr.Count() );

                                if ( children[n]->hasChildNodes() )
                                {
                                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                        children[n]->getChildNodes();

                                    for ( long m = 0; m < grandchildren.getLength(); m++ )
                                    {
                                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                        {
                                            pNewEntry->EnableChildsOnDemand( TRUE );
                                            m = grandchildren.getLength();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                catch (RuntimeException &) {
                    // do nothing, the entry will not be displayed in the UI
                }
            }
        }
    }
    MakeVisible( GetEntry( 0,0 ) );
    SetUpdateMode( TRUE );
}

Image SvxConfigGroupListBox_Impl::GetImage( Reference< browse::XBrowseNode > node, Reference< XComponentContext > xCtx, bool bIsRootNode, bool bHighContrast )
{
    Image aImage;
    if ( bIsRootNode )
    {
        OUString user = OUString::createFromAscii("user");
        OUString share = OUString::createFromAscii("share");
        if (node->getName().equals( user ) || node->getName().equals(share ) )
        {
            if( bHighContrast == BMP_COLOR_NORMAL )
                aImage = m_hdImage;
            else
                aImage = m_hdImage_hc;
        }
        else
        {
            OUString factoryURL;
            OUString nodeName = node->getName();
            Reference<XInterface> xDocumentModel = getDocumentModel(xCtx, nodeName );
            if ( xDocumentModel.is() )
            {
                Reference< ::com::sun::star::frame::XModuleManager >
                    xModuleManager(
                        xCtx->getServiceManager()
                            ->createInstanceWithContext(
                                OUString::createFromAscii("com.sun.star.frame.ModuleManager"),
                                xCtx ),
                            UNO_QUERY_THROW );
                Reference<container::XNameAccess> xModuleConfig(
                    xModuleManager, UNO_QUERY_THROW );
                // get the long name of the document:
                OUString appModule( xModuleManager->identify(
                                    xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                Any aAny = xModuleConfig->getByName(appModule);
                if( sal_True != ( aAny >>= moduleDescr ) )
                {
                    throw RuntimeException(OUString::createFromAscii("SFTreeListBox::Init: failed to get PropertyValue"), Reference< XInterface >());
                }
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if (pmoduleDescr[ pos ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "ooSetupFactoryEmptyDocumentURL") ))
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        break;
                    }
                }
            }
            if( factoryURL.getLength() > 0 )
            {
                if( bHighContrast == BMP_COLOR_NORMAL )
                    aImage = SvFileInformationManager::GetFileImage(
                        INetURLObject(factoryURL), false,
                        BMP_COLOR_NORMAL );
                else
                    aImage = SvFileInformationManager::GetFileImage(
                        INetURLObject(factoryURL), false,
                        BMP_COLOR_HIGHCONTRAST );
            }
            else
            {
                if( bHighContrast == BMP_COLOR_NORMAL )
                    aImage = m_docImage;
                else
                    aImage = m_docImage_hc;
            }
        }
    }
    else
    {
        if( node->getType() == browse::BrowseNodeTypes::SCRIPT )
        {
            if( bHighContrast == BMP_COLOR_NORMAL )
                aImage = m_macImage;
            else
                aImage = m_macImage_hc;
        }
        else
        {
            if( bHighContrast == BMP_COLOR_NORMAL )
                aImage = m_libImage;
            else
                aImage = m_libImage_hc;
        }
    }
    return aImage;
}

Reference< XInterface  >
SvxConfigGroupListBox_Impl::getDocumentModel(
    Reference< XComponentContext >& xCtx, OUString& docName )
{
    Reference< XInterface > xModel;
    Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();
    Reference< frame::XDesktop > desktop (
        mcf->createInstanceWithContext(
            OUString::createFromAscii("com.sun.star.frame.Desktop"),                 xCtx ),
            UNO_QUERY );

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    while (components->hasMoreElements())
    {
        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            OUString sTdocUrl;
            SvxScriptSelectorDialog::GetDocTitle( model, sTdocUrl );

            if( sTdocUrl.equals( docName ) )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

void SvxConfigGroupListBox_Impl::GroupSelected()
{
    SvLBoxEntry *pEntry = FirstSelected();
    SvxGroupInfo_Impl *pInfo = (SvxGroupInfo_Impl*) pEntry->GetUserData();
    pFunctionListBox->SetUpdateMode(FALSE);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SVX_CFGGROUP_FUNCTION &&
             pInfo->nKind != SVX_CFGGROUP_SCRIPTCONTAINER )
    {
        pFunctionListBox->SetUpdateMode(TRUE);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SVX_CFGGROUP_FUNCTION :
        {
            SvLBoxEntry *_pEntry = FirstSelected();
            if ( _pEntry != NULL )
            {
                SvxGroupInfo_Impl *_pInfo =
                    (SvxGroupInfo_Impl*) _pEntry->GetUserData();

                Reference< frame::XDispatchInformationProvider > xDIP(
                    m_xFrame, UNO_QUERY );

                Sequence< frame::DispatchInformation > commands;
                try
                {
                    commands = xDIP->getConfigurableDispatchInformation(
                        _pInfo->nOrd );
                }
                catch ( container::NoSuchElementException& )
                {
                }

                for ( sal_Int32 i = 0; i < commands.getLength(); i++ )
                {
                    if ( commands[i].Command.getLength() == 0 )
                    {
                        continue;
                    }

                    Image aImage;

                    OUString* aCmdURL = new OUString( commands[i].Command );

                    if ( m_pImageProvider )
                    {
                        aImage = m_pImageProvider->GetImage( *aCmdURL );
                    }

                    OUString aLabel;
                    try
                    {
                        Any a = m_xModuleCommands->getByName( *aCmdURL );
                        Sequence< beans::PropertyValue > aPropSeq;

                        if ( a >>= aPropSeq )
                        {
                            for ( sal_Int32 k = 0; k < aPropSeq.getLength(); k++ )
                            {
                                if ( aPropSeq[k].Name.equalsAscii( "Name" ) )
                                {
                                    aPropSeq[k].Value >>= aLabel;
                                    break;
                                }
                            }
                        }
                    }
                    catch ( container::NoSuchElementException& )
                    {
                    }

                    if ( aLabel.getLength() == 0 )
                    {
                        aLabel = commands[i].Command;
                    }

                    SvLBoxEntry* pFuncEntry = NULL;
                    if ( !!aImage )
                    {
                        pFuncEntry = pFunctionListBox->InsertEntry(
                            aLabel, aImage, aImage );
                    }
                    else
                    {
                        pFuncEntry = pFunctionListBox->InsertEntry(
                            aLabel, NULL );
                    }

                    SvxGroupInfo_Impl *_pGroupInfo = new SvxGroupInfo_Impl(
                        SVX_CFGFUNCTION_SLOT, 123, aCmdURL );

                    pFunctionListBox->aArr.Insert(
                        _pGroupInfo, pFunctionListBox->aArr.Count() );

                    pFuncEntry->SetUserData( _pGroupInfo );
                }
            }
            break;
        }

        case SVX_CFGGROUP_SCRIPTCONTAINER:
        {
            Reference< browse::XBrowseNode > rootNode(
                reinterpret_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

            try {
                if ( rootNode->hasChildNodes() )
                {
                    Sequence< Reference< browse::XBrowseNode > > children =
                        rootNode->getChildNodes();

                    for ( long n = 0; n < children.getLength(); n++ )
                    {
                        if (children[n]->getType() == browse::BrowseNodeTypes::SCRIPT)
                        {
                            OUString uri;
                            OUString description;

                            Reference < beans::XPropertySet >xPropSet( children[n], UNO_QUERY );
                            if (!xPropSet.is())
                            {
                                continue;
                            }

                            Any value =
                                xPropSet->getPropertyValue( String::CreateFromAscii( "URI" ) );
                            value >>= uri;

                            try
                            {
                                value = xPropSet->getPropertyValue(
                                    String::CreateFromAscii( "Description" ) );
                                value >>= description;
                            }
                            catch (Exception &) {
                                // do nothing, the description will be empty
                            }

                            ScriptInfo* aInfo = new ScriptInfo( uri );
                            aInfo->SetHelpText( description );

                            SvxGroupInfo_Impl* _pGroupInfo =
                                new SvxGroupInfo_Impl(
                                    SVX_CFGFUNCTION_SCRIPT, 123, aInfo );

                            Image aImage = GetImage( children[n], Reference< XComponentContext >(), sal_False, BMP_COLOR_NORMAL );
                            SvLBoxEntry* pNewEntry =
                                pFunctionListBox->InsertEntry( children[n]->getName(), NULL );
                            pFunctionListBox->SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                            pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                            aImage = GetImage( children[n], Reference< XComponentContext >(), sal_False, BMP_COLOR_HIGHCONTRAST );
                            pFunctionListBox->SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);
                            pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);

                            pNewEntry->SetUserData( _pGroupInfo );

                            pFunctionListBox->aArr.Insert(
                                _pGroupInfo, pFunctionListBox->aArr.Count() );

                        }
                    }
                }
            }
            catch (RuntimeException &) {
                // do nothing, the entry will not be displayed in the UI
            }
            break;
        }

        default:
        {
            return;
        }
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( 0, 0 ) );

    pFunctionListBox->SetUpdateMode(TRUE);
}

BOOL SvxConfigGroupListBox_Impl::Expand( SvLBoxEntry* pParent )
{
    BOOL bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        // Wieviele Entries k"onnen angezeigt werden ?
        ULONG nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        // Wieviele Kinder sollen angezeigt werden ?
        ULONG nChildCount = GetVisibleChildCount( pParent );

        // Passen alle Kinder und der parent gleichzeitig in die View ?
        if ( nChildCount+1 > nEntries )
        {
            // Wenn nicht, wenigstens parent ganz nach oben schieben
            MakeVisible( pParent, TRUE );
        }
        else
        {
            // An welcher relativen ViewPosition steht der aufzuklappende parent
            SvLBoxEntry *pEntry = GetFirstEntryInView();
            ULONG nParentPos = 0;
            while ( pEntry && pEntry != pParent )
            {
                nParentPos++;
                pEntry = GetNextEntryInView( pEntry );
            }

            // Ist unter dem parent noch genug Platz f"ur alle Kinder ?
            if ( nParentPos + nChildCount + 1 > nEntries )
                ScrollOutputArea( (short)( nEntries - ( nParentPos + nChildCount + 1 ) ) );
        }
    }

    return bRet;
}

void SvxConfigGroupListBox_Impl::RequestingChilds( SvLBoxEntry *pEntry )
{
    SvxGroupInfo_Impl *pInfo = (SvxGroupInfo_Impl*) pEntry->GetUserData();
    pInfo->bWasOpened = TRUE;
    switch ( pInfo->nKind )
    {
        case SVX_CFGGROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode(
                    reinterpret_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;
                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();
                        BOOL bIsRootNode = FALSE;

                        OUString user = OUString::createFromAscii("user");
                        OUString share = OUString::createFromAscii("share");
                        if ( rootNode->getName().equals(OUString::createFromAscii("Root") ))
                        {
                            bIsRootNode = TRUE;
                        }

                        OUString sDisplayTitle;
                        OUString sModelTitle;
                        SfxObjectShell* pCurrentDoc = SfxObjectShell::GetWorkingDocument();
                        if ( pCurrentDoc )
                        {
                            sDisplayTitle = pCurrentDoc->GetTitle();
                            //sModelTitle = xModelToDocTitle( pCurrentDoc->GetModel() );
                            SvxScriptSelectorDialog::GetDocTitle( pCurrentDoc->GetModel(), sModelTitle );
                        }

                        if ( sDisplayTitle.getLength() == 0 && sModelTitle.getLength() != 0 )
                        {
                            sDisplayTitle = sModelTitle;
                        }

                        for ( long n = 0; n < children.getLength(); n++ )
                        {
                            /* To mimic current starbasic behaviour we
                            need to make sure that only the current document
                            is displayed in the config tree. Tests below
                            set the bDisplay flag to FALSE if the current
                            node is a first level child of the Root and is NOT
                            either the current document, user or share */
                            Reference< browse::XBrowseNode >& theChild = children[n];
                            //#139111# some crash reports show that it might be unset
                            if(!theChild.is())
                                continue;
                            ::rtl::OUString uiName = theChild->getName();
                            BOOL bDisplay = TRUE;

                            if ( bIsRootNode )
                            {
                                if ( uiName.equals( sModelTitle ) )
                                {
                                    uiName = sDisplayTitle;
                                }
                                else if ( uiName.equals( user ) )
                                {
                                    uiName = m_sMyMacros;
                                }
                                else if ( uiName.equals( share ) )
                                {
                                    uiName = m_sProdMacros;
                                }
                                else
                                {
                                    bDisplay = FALSE;
                                }
                            }
                            if (children[n]->getType() != browse::BrowseNodeTypes::SCRIPT  && bDisplay )
                            {

                                /*
                                    We call acquire on the XBrowseNode so that it does not
                                    get autodestructed and become invalid when accessed later.
                                */
                                theChild->acquire();

                                SvxGroupInfo_Impl* _pGroupInfo =
                                    new SvxGroupInfo_Impl(SVX_CFGGROUP_SCRIPTCONTAINER,
                                        0, static_cast<void *>( theChild.get()));

                                OUString _uiName = theChild->getName();
                                sal_Bool isRootNode = sal_False;
                                //if we show slots (as in the customize dialog)
                                //then the user & share are added at depth=1
                                if(nMode && GetModel()->GetDepth(pEntry) == 0)
                                {
                                    if ( _uiName.equals( user ) )
                                    {
                                        _uiName = m_sMyMacros;
                                        isRootNode = sal_True;
                                    }
                                    else if ( _uiName.equals( share ) )
                                    {
                                        _uiName = m_sProdMacros;
                                        isRootNode = sal_True;
                                    }
                                }
                                Image aImage = GetImage( theChild, Reference< XComponentContext >(), isRootNode, BMP_COLOR_NORMAL );
                                SvLBoxEntry* pNewEntry =
                                    InsertEntry( _uiName, pEntry );
                                SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                                SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_NORMAL);
                                aImage = GetImage( theChild, Reference< XComponentContext >(), isRootNode, BMP_COLOR_HIGHCONTRAST );
                                SetExpandedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);
                                SetCollapsedEntryBmp(pNewEntry, aImage, BMP_COLOR_HIGHCONTRAST);

                                pNewEntry->SetUserData( _pGroupInfo );
                                aArr.Insert( _pGroupInfo, aArr.Count() );

                                /* i30923 - Would be nice if there was a better
                                 * way to determine if a basic lib had children
                                 * without having to ask for them (which forces
                                 * the library to be loaded */
                                pNewEntry->EnableChildsOnDemand( TRUE );
                            }
                        }
                    }
                }
                catch (RuntimeException &) {
                    // do nothing, the entry will not be displayed in the UI
                }
            }
            break;
        }

        default:
            DBG_ERROR( "Falscher Gruppentyp!" );
            break;
    }
}

void
SvxScriptSelectorDialog::GetDocTitle(
    const Reference< frame::XModel >& xModel,
    OUString& rTitle )
{
    OUString aResult = OUString::createFromAscii("Unknown");

    if ( !xModel.is() )
    {
        rTitle = aResult;
        return;
    }

    try
    {
        Reference< frame::XController > xCurrentController = xModel->getCurrentController();
        if ( xCurrentController.is() )
        {
            Reference< beans::XPropertySet > xProps( xCurrentController->getFrame(), UNO_QUERY_THROW );

            OUString prop = OUString::createFromAscii( "Title" );
            OUString tmp;

            if ( sal_True == ( xProps->getPropertyValue( prop ) >>= tmp ) )
            {
                if ( xModel->getURL().getLength() == 0 )
                {
                    // process "UntitledX - YYYYYYYY" // to get UntitledX
                    sal_Int32 pos = 0;
                    aResult = tmp.getToken(0, ' ', pos);
                }
                else
                {
                    Reference< document::XDocumentInfoSupplier > xDIS(
                        xModel, UNO_QUERY );

                    if ( xDIS.is() )
                    {
                        Reference< beans::XPropertySet > xProp (
                            xDIS->getDocumentInfo(), UNO_QUERY );
                        xProp->getPropertyValue( prop ) >>= tmp;
                    }

                    if ( tmp.getLength() == 0 )
                    {
                        // strip out the last leaf of location name
                        // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
                        tmp = xModel->getURL();
                        INetURLObject aURLObj( tmp );
                        if ( !aURLObj.HasError() )
                            tmp = aURLObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                    }
                    aResult = tmp;
                }
            }
        }
    }
    catch ( Exception& )
    {
        // OSL_TRACE("GetDocTitle() exception: %s", PRTSTR(e.Message));
    }
    rTitle = aResult;
}

/*
 * Implementation of SvxScriptSelectorDialog
 *
 * This dialog is used for selecting Slot API commands
 * and Scripting Framework Scripts.
 */

SvxScriptSelectorDialog::SvxScriptSelectorDialog(
  Window* pParent, BOOL bShowSlots, const Reference< frame::XFrame >& xFrame )
    :
    ModelessDialog( pParent, SVX_RES( RID_DLG_SCRIPTSELECTOR ) ),
    aDialogDescription( this, ResId( TXT_SELECTOR_DIALOG_DESCRIPTION ) ),
    aGroupText( this, ResId( TXT_SELECTOR_CATEGORIES ) ),
    aCategories( this, ResId( BOX_SELECTOR_CATEGORIES ),
        bShowSlots ? SFX_SLOT_MENUCONFIG : 0, xFrame ),
    aFunctionText( this, ResId( TXT_SELECTOR_COMMANDS ) ),
    aCommands( this, ResId( BOX_SELECTOR_COMMANDS ) ),
    aOKButton( this, ResId( BTN_SELECTOR_OK ) ),
    aCancelButton( this, ResId( BTN_SELECTOR_CANCEL ) ),
    aHelpButton( this, ResId( BTN_SELECTOR_HELP ) ),
    aDescription( this, ResId( GRP_SELECTOR_DESCRIPTION ) ),
    aDescriptionText( this, ResId( TXT_SELECTOR_DESCRIPTION ) ),
    m_bShowSlots( bShowSlots )
{

    ResMgr* pMgr = DIALOG_MGR();

    // If we are showing Slot API commands update labels in the UI, and
    // enable drag'n'drop
    if ( m_bShowSlots )
    {
        aGroupText.SetText( String( ResId( STR_SELECTOR_CATEGORIES, pMgr ) ) );
        aOKButton.SetText( String( ResId( STR_SELECTOR_ADD, pMgr ) ) );
        aCancelButton.SetText( String( ResId( STR_SELECTOR_CLOSE, pMgr ) ) );
        aFunctionText.SetText( String( ResId( STR_SELECTOR_COMMANDS, pMgr ) ) );
        SetDialogDescription(
            String( ResId( STR_SELECTOR_ADD_COMMANDS_DESCRIPTION, pMgr ) ) );
        SetText( String( ResId( STR_SELECTOR_ADD_COMMANDS, pMgr ) ) );

        aCommands.SetDragDropMode( SV_DRAGDROP_APP_COPY );
    }

    ResizeControls();

    aCategories.SetFunctionListBox( &aCommands );
    aCategories.Init();
    // aCategories.Select( aCategories.GetEntry( 0, 0 ) );

    aCategories.SetSelectHdl(
            LINK( this, SvxScriptSelectorDialog, SelectHdl ) );
    aCommands.SetSelectHdl( LINK( this, SvxScriptSelectorDialog, SelectHdl ) );

    aOKButton.SetClickHdl( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );
    aCancelButton.SetClickHdl( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );

    UpdateUI();
    FreeResource();
}

void SvxScriptSelectorDialog::ResizeControls()
{
    Point p, newp;
    Size s, news;
    long gap;

    USHORT style = TEXT_DRAW_MULTILINE | TEXT_DRAW_TOP |
                   TEXT_DRAW_LEFT | TEXT_DRAW_WORDBREAK;

    // get dimensions of dialog instructions control
    p = aDialogDescription.GetPosPixel();
    s = aDialogDescription.GetSizePixel();

    // get dimensions occupied by text in the control
    Rectangle rect =
        GetTextRect( Rectangle( p, s ), aDialogDescription.GetText(), style );
    news = rect.GetSize();

    // the gap is the difference between the control height and its text height
    gap = s.Height() - news.Height();

    // resize the dialog instructions control
    news = Size( s.Width(), s.Height() - gap );
    aDialogDescription.SetSizePixel( news );

    // resize other controls to fill the gap
    p = aGroupText.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aGroupText.SetPosPixel( newp );

    p = aCategories.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCategories.SetPosPixel( newp );
    s = aCategories.GetSizePixel();
    news = Size( s.Width(), s.Height() + gap );
    aCategories.SetSizePixel( news );

    p = aFunctionText.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aFunctionText.SetPosPixel( newp );

    p = aCommands.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCommands.SetPosPixel( newp );
    s = aCommands.GetSizePixel();
    news = Size( s.Width(), s.Height() + gap );
    aCommands.SetSizePixel( news );

    p = aOKButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aOKButton.SetPosPixel( newp );

    p = aCancelButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCancelButton.SetPosPixel( newp );

    p = aHelpButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aHelpButton.SetPosPixel( newp );
}

SvxScriptSelectorDialog::~SvxScriptSelectorDialog()
{
}

IMPL_LINK( SvxScriptSelectorDialog, SelectHdl, Control*, pCtrl )
{
    if ( pCtrl == &aCategories )
    {
        aCategories.GroupSelected();
    }
    else if ( pCtrl == &aCommands )
    {
        aCommands.FunctionSelected();
    }
    UpdateUI();
    return 0;
}

// Check if command is selected and enable the OK button accordingly
// Grab the help text for this id if available and update the description field
void
SvxScriptSelectorDialog::UpdateUI()
{
    OUString url = GetScriptURL();
    if ( url != NULL && url.getLength() != 0 )
    {
        String rMessage =
            aCommands.GetHelpText( aCommands.FirstSelected() );
        aDescriptionText.SetText( rMessage );

        aOKButton.Enable( TRUE );
    }
    else
    {
        aDescriptionText.SetText( String() );
        aOKButton.Enable( FALSE );
    }
}

IMPL_LINK( SvxScriptSelectorDialog, ClickHdl, Button *, pButton )
{
    if ( pButton == &aCancelButton )
    {
        // If we are displaying Slot API commands then the dialog is being
        // run from Tools/Configure and we should not close it, just hide it
        if ( m_bShowSlots == FALSE )
        {
            EndDialog( RET_CANCEL );
        }
        else
        {
            Hide();
        }
    }
    else if ( pButton == &aOKButton )
    {
        GetAddHdl().Call( this );

        // If we are displaying Slot API commands then this the dialog is being
        // run from Tools/Configure and we should not close it
        if ( m_bShowSlots == FALSE )
        {
            EndDialog( RET_OK );
        }
        else
        {
            // Select the next entry in the list if possible
            SvLBoxEntry* current = aCommands.FirstSelected();
            SvLBoxEntry* next = aCommands.NextSibling( current );

            if ( next != NULL )
            {
                aCommands.Select( next );
            }
        }
    }

    return 0;
}

void
SvxScriptSelectorDialog::SetRunLabel()
{
    aOKButton.SetText( String( ResId( STR_SELECTOR_RUN, DIALOG_MGR() ) ) );
}

void
SvxScriptSelectorDialog::SetDialogDescription( const String& rDescription )
{
    aDialogDescription.SetText( rDescription );
}

USHORT
SvxScriptSelectorDialog::GetSelectedId()
{
    return aCommands.GetId( aCommands.GetLastSelectedEntry() );
}

String
SvxScriptSelectorDialog::GetScriptURL()
{
    OUString result;

    SvLBoxEntry *pEntry = aCommands.GetLastSelectedEntry();
    if ( pEntry )
    {
        SvxGroupInfo_Impl *pData = (SvxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData->nKind == SVX_CFGFUNCTION_SLOT )
        {
            if ( pData->pObject != NULL )
            {
                OUString* tmp = (OUString*) pData->pObject;
                result = OUString( *tmp );
            }
        }
        else if ( pData->nKind == SVX_CFGFUNCTION_SCRIPT )
        {
            ScriptInfo* pInfo = (ScriptInfo*) pData->pObject;
            if ( pInfo != NULL )
            {
                result = pInfo->GetURI();
            }
        }
    }

    return result;
}

String
SvxScriptSelectorDialog::GetSelectedDisplayName()
{
    return aCommands.GetEntryText( aCommands.GetLastSelectedEntry() );
}

String
SvxScriptSelectorDialog::GetSelectedHelpText()
{
    return aCommands.GetHelpText( aCommands.GetLastSelectedEntry() );
}
