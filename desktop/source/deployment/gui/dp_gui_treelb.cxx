/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_treelb.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:17:36 $
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
#include "precompiled_desktop.hxx"

#include "dp_misc.h"
#include "dp_gui.h"
#include "dp_gui_shared.hxx"
#include "dp_ucb.h"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/anytostring.hxx"
#include "ucbhelper/content.hxx"
#include "vos/mutex.hxx"
#include "vcl/help.hxx"
#include "vcl/waitobj.hxx"
#include "tools/urlobj.hxx"
#include "svtools/imagemgr.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackageManagerFactory.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "com/sun/star/frame/XModuleManager.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include <hash_set>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;


namespace dp_gui
{

//------------------------------------------------------------------------------
PackageState getPackageState(
    Reference<deployment::XPackage> const & xPackage,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    try {
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            xPackage->isRegistered( Reference<task::XAbortChannel>(),
                                    xCmdEnv ) );
        if (option.IsPresent)
        {
            beans::Ambiguous<sal_Bool> const & reg = option.Value;
            if (reg.IsAmbiguous)
                return AMBIGUOUS;
            else
                return reg.Value ? REGISTERED : NOT_REGISTERED;
        }
        else
            return NOT_AVAILABLE;
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception & exc) {
        (void) exc;
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return NOT_AVAILABLE;
    }
}


//==============================================================================
struct NodeImpl : public ::cppu::WeakImplHelper1<util::XModifyListener>
{
    //Keeps the parameters of the XModifyListener::modified call so they can be
    //used later in the event handler asyncModified. It also keeps the NodeImpl
    //instance alive so that the posted events (to ourself) can always be processed
    struct ModifiedParams
    {   ModifiedParams(Reference<XCommandEnvironment> const & _xCmdEnv, Reference<XInterface> const & _xNodeImpl);
        //The argument supplied to XModifyListener::modified
        Reference<XCommandEnvironment> xCmdEnv;
        //This reference keeps the NodeImpl alive until the event has been processed
        Reference<XInterface> xNodeImpl;
    };

    DialogImpl::TreeListBoxImpl * m_treelb;
    SvLBoxEntry * m_lbEntry;
    DialogImpl::TreeListBoxImpl::t_nodeList::iterator m_it;

    const OUString m_factoryURL;
    Reference<deployment::XPackageManager> m_xPackageManager;
    Reference<deployment::XPackage> m_xPackage;
    Reference<css::uno::XComponentContext> m_xComponentContext;
    bool m_bDisposing;

    virtual ~NodeImpl();

    inline NodeImpl( Reference<css::uno::XComponentContext> xComponentContext,
                     DialogImpl::TreeListBoxImpl * treelb,
                     OUString const & factoryURL,
                     Reference<deployment::XPackageManager> const &
                     xPackageManager,
                     Reference<deployment::XPackage> const & xPackage )
        : m_treelb( treelb ),
          m_lbEntry( 0 ),
          m_it( treelb->m_nodes.end() ),
          m_factoryURL( factoryURL ),
          m_xPackageManager( xPackageManager ),
          m_xPackage( xPackage ),
          m_xComponentContext(xComponentContext),
          m_bDisposing(false)
        {}

    //Perfom asynchronous Modified call in main thread
    DECL_LINK(asyncModified, ModifiedParams*);

    Image getIcon() const;

    static inline NodeImpl * get( SvLBoxEntry * entry );

    void modified( Reference<XCommandEnvironment> const & xCmdEnv );

    // XEventListener
    virtual void SAL_CALL disposing( lang::EventObject const & evt )
        throw (RuntimeException);
    // XModifyListener
    virtual void SAL_CALL modified( lang::EventObject const & )
        throw (RuntimeException);

};

NodeImpl::ModifiedParams::ModifiedParams(
    Reference<XCommandEnvironment> const & _xCmdEnv, Reference<XInterface> const & _xNodeImpl):
    xCmdEnv(_xCmdEnv),
    xNodeImpl(_xNodeImpl)
{
}
//______________________________________________________________________________
inline NodeImpl * NodeImpl::get( SvLBoxEntry * entry )
{
    OSL_ASSERT( entry != 0 );
    if (entry != 0) {
        NodeImpl * node = static_cast<NodeImpl *>(entry->GetUserData());
        OSL_ASSERT( node != 0 );
        return node;
    }
    return 0;
}

//______________________________________________________________________________
NodeImpl::~NodeImpl()
{
}

//______________________________________________________________________________
Image NodeImpl::getIcon() const
{
    Image ret;
    if (m_xPackage.is())
    {
        const Reference<deployment::XPackageTypeInfo> xPackageType(
            m_xPackage->getPackageType() );
        sal_uInt16 id = 0;
        if (xPackageType.is() &&
            (xPackageType->getIcon(
                // works here, because getCppuType(unsigned short) is defined
                // for IDL UNSIGNED SHORT (not IDL CHAR),
                // try to avoid UNSIGNED SHORT if possible:
                m_treelb->m_hiContrastMode, true /* small */ ) >>= id))
        {
            // opt most common package bundle icon:
            if (id == RID_IMG_DEF_PACKAGE_BUNDLE)
                ret = m_treelb->m_defaultPackageBundle;
            else if (id == RID_IMG_DEF_PACKAGE_BUNDLE_HC)
                ret = m_treelb->m_defaultPackageBundle_hc;
            else
                ret = DialogImpl::getResId(id);
        }
        else
        {
            if (m_xPackage->isBundle())
                ret = m_treelb->m_hiContrastMode
                    ? m_treelb->m_defaultPackageBundle_hc
                    : m_treelb->m_defaultPackageBundle;
            else
                ret = m_treelb->m_hiContrastMode
                    ? m_treelb->m_defaultPackage_hc
                    : m_treelb->m_defaultPackage;
        }
    }
    else if (m_factoryURL.getLength() > 0)
    {
        ret = SvFileInformationManager::GetFileImage(
            INetURLObject(m_factoryURL), false /* big */,
            m_treelb->m_hiContrastMode );
    }
    else
    {
        OUString context( m_xPackageManager->getContext() );
        if (context.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:") ))
        {
            // unknown document:
            if (m_treelb->m_hiContrastMode)
                ret = DialogImpl::getResId(RID_IMG_DEF_DOCUMENT);
            else
                ret = DialogImpl::getResId(RID_IMG_DEF_DOCUMENT_HC);
        }
        else
        {
            OSL_ASSERT(
                context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ) ||
                context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ) );

            if (m_treelb->m_hiContrastMode)
                ret = DialogImpl::getResId(RID_IMG_DEF_INSTALLATION_HC);
            else
                ret = DialogImpl::getResId(RID_IMG_DEF_INSTALLATION);
        }
    }

    return ret;
}

// XEventListener
//Can be called from a separate thread, for example the thread which installes the
//extension updates. Therefore we make sure to modifiy the gui only in the main thread.
//We keep the NodeImpl alive by incresing the ref count (ModifiedParams).
void NodeImpl::disposing( lang::EventObject const & evt )
    throw (RuntimeException)
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    m_bDisposing = true;

    OSL_ASSERT( m_lbEntry != 0 );
    if (m_lbEntry != 0)
    {
        // remove all children:
        SvLBoxEntry * entry = m_treelb->FirstChild(m_lbEntry);
        while (entry != 0) {
            ::rtl::Reference<NodeImpl> node( NodeImpl::get(entry) );
            entry = m_treelb->NextSibling(entry);
            node->disposing( evt );
        }

        m_treelb->GetModel()->Remove( m_lbEntry );
        m_lbEntry = 0;
    }

    OSL_ASSERT( m_it != m_treelb->m_nodes.end() );
    if (m_it != m_treelb->m_nodes.end()) {
        m_treelb->m_nodes.erase( m_it );
        m_it = m_treelb->m_nodes.end();
    }

    if (m_xPackage.is())
        m_xPackage->removeModifyListener( this );
    else
        m_xPackageManager->removeModifyListener( this );
    m_xPackageManager.clear();
    m_xPackage.clear();

    if (m_treelb->m_dialog != 0)
        m_treelb->m_dialog->updateButtonStates();
}

//void NodeImpl::asyncDisposing( Reference<XCommandEnvironment> const & xCmdEnv )
//{
//    Application::PostUserEvent(
//        LINK( this, NodeImpl, asyncDisposing), 0)
//}
//
//IMPL_LINK(NodeImpl, asyncDisposing, void*, EMPTYARG)
//{
//}

struct iface_hash {
    ::std::size_t operator () (Reference<deployment::XPackage> const &x) const {
        return reinterpret_cast< ::std::size_t >(
            Reference<XInterface>(x, UNO_QUERY_THROW).get() );
    }
};

//______________________________________________________________________________
//Can be called from a separate thread, for example the thread which installes the
//extension updates. Therefore we make sure to modifiy the gui only in the main thread.
//We keep the NodeImpl alive by incresing the ref count (ModifiedParams).
void NodeImpl::modified( Reference<XCommandEnvironment> const & xCmdEnv )
{
    Application::PostUserEvent(
        LINK( this, NodeImpl, asyncModified ),
        new ModifiedParams(
            xCmdEnv,  Reference<XInterface>(static_cast<OWeakObject*>(this), UNO_QUERY_THROW)));
}

IMPL_LINK(NodeImpl, asyncModified, ModifiedParams*, pModifiedParams)
{
    //m_bDisposing is guarded by the solar mutex
    if (! m_bDisposing)
    {
        const Reference<XCommandEnvironment> & xCmdEnv = pModifiedParams->xCmdEnv;
        try {
            if (m_xPackage.is())
            {
                switch (getPackageState( m_xPackage, xCmdEnv ))
                {
                case REGISTERED:
                    m_treelb->SetEntryText( m_treelb->m_strEnabled, m_lbEntry, 2 );
                    break;
                case NOT_REGISTERED:
                    m_treelb->SetEntryText( m_treelb->m_strDisabled, m_lbEntry, 2 );
                    break;
                case AMBIGUOUS:
                    m_treelb->SetEntryText( m_treelb->m_strUnknown, m_lbEntry, 2 );
                    break;
                case NOT_AVAILABLE:
                    m_treelb->SetEntryText( String(), m_lbEntry, 2 );
                    break;
                }

                if (m_treelb->m_dialog != 0)
                    m_treelb->m_dialog->updateButtonStates( xCmdEnv );
            }
            else
            {
                typedef ::std::hash_set<
                    Reference<deployment::XPackage>, iface_hash > t_set;
                t_set tlboxPackages;
                sal_Int32 count = m_treelb->GetLevelChildCount(m_lbEntry);
                sal_Int32 pos = 0;
                for ( ; pos < count; ++pos ) {
                    tlboxPackages.insert(
                        NodeImpl::get(
                            m_treelb->GetEntry(m_lbEntry, pos) )->m_xPackage );
                }

                const Sequence< Reference<deployment::XPackage> > packages(
                    m_xPackageManager->getDeployedPackages(
                        Reference<task::XAbortChannel>(), xCmdEnv ) );
                t_set::const_iterator const iEnd( tlboxPackages.end() );
                for ( pos = packages.getLength(); pos--; )
                {
                    t_set::iterator iFind( tlboxPackages.find( packages[ pos ] ) );
                    if (iFind == iEnd) {
                        m_treelb->addPackageNode(
                            m_lbEntry, packages[ pos ], xCmdEnv );
                    }
                }
            }
        }
        catch (...)
        {
            //Make sure we release the reference to NodeImpl
            pModifiedParams->xNodeImpl.clear();
            pModifiedParams->xCmdEnv.clear();
            delete pModifiedParams;
            throw;
        }
    } // if (!m_bDisposing)

    //Make sure we release the reference to NodeImpl
    pModifiedParams->xNodeImpl.clear();
    pModifiedParams->xCmdEnv.clear();
    delete pModifiedParams;

    return 0;
}

// XModifyListener
//______________________________________________________________________________
void NodeImpl::modified( lang::EventObject const & )
    throw (RuntimeException)
{
    try {
        const Reference<XCommandEnvironment> xCmdEnv(
           new ProgressCommandEnv( m_xComponentContext, m_treelb->m_dialog, String() ) );
        modified( xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        // already handled by UUI handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        m_treelb->m_dialog->errbox( ::comphelper::anyToString(exc) );
    }
}

//______________________________________________________________________________
DialogImpl::TreeListBoxImpl::~TreeListBoxImpl()
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    m_dialog = 0; // in deinit

    const lang::EventObject evt;
    SvLBoxEntry * entry = First();
    while (entry != 0) {
        // remove all contexts:
        ::rtl::Reference<NodeImpl> node( NodeImpl::get(entry) );
        OSL_ASSERT( ! node->m_xPackage.is() );
        entry = NextSibling(entry);
        node->disposing( evt );
    }
    OSL_ASSERT( m_nodes.empty() );
}

//______________________________________________________________________________
DialogImpl::SelectionBoxControl::SelectionBoxControl( DialogImpl * dialog )
    : Control( dialog, WB_BORDER | WB_TABSTOP ),
    m_bShutDown(false),
    m_dialog(dialog)
{
}

long DialogImpl::SelectionBoxControl::Notify( NotifyEvent & rEvt )
{
    const long nRet = Control::Notify( rEvt );
    if (IsReallyVisible() &&
        rEvt.GetType() == EVENT_GETFOCUS &&
        rEvt.GetWindow() != static_cast<Window *>(m_dialog->m_treelb.get())
        && ! m_bShutDown)
    {
        m_dialog->m_treelb->GrabFocus();
    }
    return nRet;
}

//______________________________________________________________________________
DialogImpl::TreeListBoxImpl::TreeListBoxImpl(
    Reference<XComponentContext> const & context, Window * pParent,
    DialogImpl * dialog )
    : SvHeaderTabListBox( pParent,
                          WB_CLIPCHILDREN | WB_TABSTOP |
                          WB_HASBUTTONS | WB_HASLINES |
                          /* WB_BORDER | */ WB_HASLINESATROOT |
                          WB_HASBUTTONSATROOT | WB_HIDESELECTION |
                          WB_HSCROLL ),
      m_context( context ),
      m_dialog( dialog ),
      m_currentSelectedEntry( 0 ),
      m_hiContrastMode( GetDisplayBackground().GetColor().IsDark() ),
      m_strEnabled( getResourceString(RID_STR_ENABLED) ),
      m_strDisabled( getResourceString(RID_STR_DISABLED) ),
      m_strUnknown( getResourceString(RID_STR_UNKNOWN) ),
      m_strCtxAdd( getResourceString(RID_CTX_ITEM_ADD) ),
      m_strCtxRemove( getResourceString(RID_CTX_ITEM_REMOVE) ),
      m_strCtxEnable( getResourceString(RID_CTX_ITEM_ENABLE) ),
      m_strCtxDisable( getResourceString(RID_CTX_ITEM_DISABLE) ),
      m_strCtxExport( getResourceString(RID_CTX_ITEM_EXPORT) ),
      m_strCtxCheckUpdate( getResourceString(RID_CTX_ITEM_CHECK_UPDATE) ),
      m_strCtxOptions( getResourceString(RID_CTX_ITEM_OPTIONS) ),
      m_defaultPackage( getResId(RID_IMG_DEF_PACKAGE) ),
      m_defaultPackage_hc( getResId(RID_IMG_DEF_PACKAGE_HC) ),
      m_defaultPackageBundle( getResId(RID_IMG_DEF_PACKAGE_BUNDLE) ),
      m_defaultPackageBundle_hc( getResId(RID_IMG_DEF_PACKAGE_BUNDLE_HC) )
{
    SetHelpId( HID_PACKAGE_MANAGER_TREELISTBOX );
    SetNodeBitmaps( Image( getResId(RID_IMG_PLUS) ),
                    Image( getResId(RID_IMG_MINUS) ),
                    BMP_COLOR_NORMAL );
    SetNodeBitmaps( Image( getResId(RID_IMG_PLUS_HC) ),
                    Image( getResId(RID_IMG_MINUS_HC) ),
                    BMP_COLOR_HIGHCONTRAST );
    SetSelectionMode( MULTIPLE_SELECTION );
    SetSpaceBetweenEntries( 4 );
    EnableContextMenuHandling();

    // timer for balloon help:
    m_timer.SetTimeout( 200 );
    m_timer.SetTimeoutHdl(
        LINK( this, DialogImpl::TreeListBoxImpl, TimerHandler ) );
}

//______________________________________________________________________________
SvLBoxEntry * DialogImpl::TreeListBoxImpl::addNode(
    SvLBoxEntry * parentNode,
    String const & displayName,
    OUString const & factoryURL,
    Reference<deployment::XPackageManager> const & xPackageManager,
    Reference<deployment::XPackage> const & xPackage,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool sortIn )
{
    OSL_ASSERT(xPackageManager.is());
    NodeImpl * node = new NodeImpl(
        m_context, this, factoryURL, xPackageManager, xPackage );
    Reference<util::XModifyListener> xListener( node );
    m_nodes.push_front( xListener );
    node->m_it = m_nodes.begin();

    ULONG pos = LIST_APPEND;
    if (sortIn)
    {
        // sort in after top-level nodes after user, shared:
        pos = parentNode == 0 ? 2 : 0;
        ULONG count = GetLevelChildCount(parentNode);
        for ( ; pos < count; ++pos )
        {
            SvLBoxEntry * entry = GetEntry( parentNode, pos );
            OSL_ASSERT( entry != 0 );
            if (GetEntryText(entry).CompareIgnoreCaseToAscii(
                    displayName ) == COMPARE_GREATER)
                break;
        }
        if (pos >= count)
            pos = LIST_APPEND;
    }

    Image imgIcon( node->getIcon() );
    if (parentNode == 0)
    {
        // top-level node:
        node->m_lbEntry = InsertEntryToColumn(
            displayName, imgIcon, imgIcon, NULL, pos, 0xffff, node );
        node->m_xPackageManager->addModifyListener( xListener );
    }
    else
    {
        String name( displayName );
        name.AppendAscii( RTL_CONSTASCII_STRINGPARAM("\t\t") );
        node->m_lbEntry = InsertEntryToColumn(
            name, imgIcon, imgIcon, parentNode, pos, 0xffff, node );
        SetEntryText( xPackage->getVersion(), node->m_lbEntry, 1 );
        // update status:
        node->modified( xCmdEnv );
        node->m_xPackage->addModifyListener( xListener );
    }

    bool hasChildren = !xPackage.is() || xPackage->isBundle();
    if (hasChildren && node->m_lbEntry != 0)
    {
        // HACK: xxx todo: missing method in svtools
        // set the flag which allows if the entry can be expanded
        // test ondemand
        node->m_lbEntry->SetFlags(
            (node->m_lbEntry->GetFlags() &
             ~(SV_ENTRYFLAG_NO_NODEBMP | SV_ENTRYFLAG_HAD_CHILDREN))
            | SV_ENTRYFLAG_CHILDS_ON_DEMAND );
    }

    return node->m_lbEntry;
}

//______________________________________________________________________________
SvLBoxEntry * DialogImpl::TreeListBoxImpl::addPackageNode(
    SvLBoxEntry * parentNode,
    Reference<deployment::XPackage> const & xPackage,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    return addNode( parentNode, xPackage->getDisplayName(),
                    OUString() /* no factory URL */,
                    NodeImpl::get(parentNode)->m_xPackageManager,
                    xPackage, xCmdEnv );
}

//______________________________________________________________________________
SvLBoxEntry * DialogImpl::TreeListBoxImpl::getCurrentSingleSelectedEntry() const
{
    SvLBoxEntry * entry = FirstSelected();
    if (entry != 0 && NextSelected(entry) == 0)
        return entry;
    else
        return 0;
}

//______________________________________________________________________________
bool DialogImpl::TreeListBoxImpl::isFirstLevelChild( SvLBoxEntry * entry ) const
{
    if (entry == 0)
        return false;
    entry = GetParent( entry );
    if (entry != 0)
        return GetParent( entry ) == 0;
    else
        return false;
}

//______________________________________________________________________________
OUString DialogImpl::TreeListBoxImpl::getContext( SvLBoxEntry * entry ) const
{
    if (entry == 0)
        return OUString();
    else
        return NodeImpl::get(entry)->m_xPackageManager->getContext();
}

//______________________________________________________________________________
Reference<deployment::XPackage>
DialogImpl::TreeListBoxImpl::getPackage( SvLBoxEntry * entry ) const
{
    if (entry == 0)
        return Reference<deployment::XPackage>();
    else
        return NodeImpl::get(entry)->m_xPackage;
}

//______________________________________________________________________________
Reference<deployment::XPackageManager>
DialogImpl::TreeListBoxImpl::getPackageManager( SvLBoxEntry * entry ) const
{
    if (entry == 0)
        return Reference<deployment::XPackageManager>();
    else
        return NodeImpl::get(entry)->m_xPackageManager;
}

//______________________________________________________________________________
PopupMenu * DialogImpl::TreeListBoxImpl::CreateContextMenu(void)
{
    if (m_dialog == 0)
        return 0;

    ::std::auto_ptr<PopupMenu> menu( new PopupMenu );

    if (m_dialog->m_addButton->IsEnabled())
        menu->InsertItem( RID_BTN_ADD, m_strCtxAdd );
    if (m_dialog->m_removeButton->IsEnabled())
        menu->InsertItem( RID_BTN_REMOVE, m_strCtxRemove );

    if ((m_dialog->m_enableButton->IsEnabled()
         || m_dialog->m_disableButton->IsEnabled())
        && menu->GetItemCount() != 0)
        menu->InsertSeparator();
    if (m_dialog->m_enableButton->IsEnabled())
        menu->InsertItem( RID_BTN_ENABLE, m_strCtxEnable );
    if (m_dialog->m_disableButton->IsEnabled())
        menu->InsertItem( RID_BTN_DISABLE, m_strCtxDisable );

    if (m_dialog->m_exportButton->IsEnabled())
    {
        if (menu->GetItemCount() != 0)
            menu->InsertSeparator();
        menu->InsertItem( RID_BTN_EXPORT, m_strCtxExport );
    }

    bool updateUrl = false;
    OUString sExtUpdateUrl;
    if (dp_misc::getExtensionDefaultUpdateURL().getLength() > 0)
        updateUrl = true;

    for (SvLBoxEntry * e = FirstSelected(); e != NULL; e = NextSelected(e))
    {
        if (isFirstLevelChild(e))
        {
            updateUrl = updateUrl ||
                getPackage(e)->getUpdateInformationURLs().getLength() != 0;
        }
        else
        {
            updateUrl = false;
            break;
        }
    }
    if (updateUrl)
    {
        if (menu->GetItemCount() != 0)
            menu->InsertSeparator();
        menu->InsertItem( RID_BTN_CHECK_UPDATES, m_strCtxCheckUpdate );
    }

    if (m_dialog->m_optionsButton->IsEnabled())
    {
        menu->InsertSeparator();
        menu->InsertItem( RID_BTN_OPTIONS, m_strCtxOptions );
    }

    return menu.release();
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::ExcecuteContextMenuAction(
    USHORT nSelectedPopupEntry )
{
    if (m_dialog == 0)
        return;

    switch (nSelectedPopupEntry) {
    case 0:
        break;
    case RID_BTN_ADD:
        m_dialog->m_addButton->Click();
        break;
    case RID_BTN_REMOVE:
        m_dialog->m_removeButton->Click();
        break;
    case RID_BTN_ENABLE:
        m_dialog->m_enableButton->Click();
        break;
    case RID_BTN_DISABLE:
        m_dialog->m_disableButton->Click();
        break;
    case RID_BTN_EXPORT:
        m_dialog->m_exportButton->Click();
        break;
    case RID_BTN_CHECK_UPDATES:
        m_dialog->checkUpdates(true);
        break;
    case RID_BTN_OPTIONS:
        m_dialog->m_optionsButton->Click();
        break;
    default:
        OSL_ENSURE( 0, "### forgot button entry?!" );
        break;
    }
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::MouseMove( MouseEvent const & evt )
{
    Point pos = evt.GetPosPixel();
    m_currentSelectedEntry = getCurrentSingleSelectedEntry();

    if (m_currentSelectedEntry != 0 &&
        GetEntry( pos ) == m_currentSelectedEntry)
    {
        m_timer.Start();
    }
    else
    {
        Help::ShowBalloon( this, pos, String() );
        m_timer.Stop();
    }
}

//______________________________________________________________________________
IMPL_LINK( DialogImpl::TreeListBoxImpl, TimerHandler, Timer *, EMPTYARG )
{
    m_timer.Stop();
    Point pos = GetPointerPosPixel();
    SvLBoxEntry * currentEntry = getCurrentSingleSelectedEntry();
    if (currentEntry != 0 &&
        GetEntry( pos ) == currentEntry &&
        m_currentSelectedEntry == currentEntry ) // still the same?
    {
        NodeImpl * node = NodeImpl::get(currentEntry);
        String balloon;
        if (node->m_xPackage.is()) {
            ::rtl::OUStringBuffer buf;
            buf.append( node->m_xPackage->getDescription() );
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                node->m_xPackage->getPackageType() );
            if (xPackageType.is()) {
                if (buf.getLength() > 0)
                    buf.append( static_cast<sal_Unicode>('\n') );
                buf.append( static_cast<sal_Unicode>('(') );
                buf.append( xPackageType->getMediaType() );
#if OSL_DEBUG_LEVEL > 1
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(",\n") );
                buf.append( node->m_xPackage->getURL() );
#endif
                buf.append( static_cast<sal_Unicode>(')') );
                balloon = buf.makeStringAndClear();
            }
        }
        else if (node->m_factoryURL.getLength() > 0) {
            balloon = SvFileInformationManager::GetDescription(
                INetURLObject(node->m_factoryURL) );
        }

        if (balloon.Len() > 0)
            Help::ShowBalloon( this, OutputToScreenPixel( pos ), balloon );
    }
    return 0;
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::SelectHdl()
{
    if (m_dialog != 0)
        m_dialog->updateButtonStates();
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::DeselectHdl()
{
    if (m_dialog != 0)
        m_dialog->updateButtonStates();
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::KeyInput( KeyEvent const & evt )
{
    const KeyCode key = evt.GetKeyCode();
    if (key == KEY_DELETE &&
        m_dialog != 0 && m_dialog->m_removeButton->IsEnabled())
    {
        // remove selected:
        m_dialog->m_removeButton->Click();
        return;
    }
    SvTreeListBox::KeyInput( evt );
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::RequestingChilds( SvLBoxEntry * pParent )
{
    try {
        if (GetChildCount( pParent ) > 0)
            return;

        WaitObject wo(this); // clock...

        Sequence< Reference<deployment::XPackage> > packages;
        NodeImpl * parentNode = NodeImpl::get(pParent);

        const Reference<XCommandEnvironment> xCmdEnv(
            new ProgressCommandEnv(m_context, m_dialog, String()) );
        if (parentNode->m_xPackage.is()) {
            packages = parentNode->m_xPackage->getBundle(
                Reference<task::XAbortChannel>(), xCmdEnv );
        }
        else { // is context
            packages = parentNode->m_xPackageManager->getDeployedPackages(
                Reference<task::XAbortChannel>(), xCmdEnv );
        }
        if (packages.getLength() > 0) {
            Reference<deployment::XPackage> const * ppackages =
                packages.getConstArray();
            SetUpdateMode(FALSE);
            for ( sal_Int32 pos = packages.getLength(); pos--; ) {
                //disregard executable (application/vnd.sun.star.executable)
                //it will not be displayed an shall not be disabled/enabled
                OUString sType = ppackages[ pos ]->getPackageType()->getMediaType();
                if (sType.equals(OUSTR("application/vnd.sun.star.executable")))
                    continue;

                addPackageNode(
                    parentNode->m_lbEntry, ppackages[ pos ], xCmdEnv );
            }
            SetUpdateMode(TRUE);
        }
    }
    catch (CommandFailedException &) {
        // already handled by UUI handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        m_dialog->errbox( ::comphelper::anyToString(exc) );
    }
}

//______________________________________________________________________________
BOOL DialogImpl::TreeListBoxImpl::Expand( SvLBoxEntry * pParent )
{
    BOOL ret = SvTreeListBox::Expand( pParent );
    if (ret)
    {
        ULONG nVisibleEntries =
            GetOutputSizePixel().Height() / GetEntryHeight();
        ULONG nVisibleChildren = GetVisibleChildCount( pParent );

        if ((nVisibleChildren + 1) > nVisibleEntries) {
            MakeVisible( pParent, TRUE );
        }
        else {
            SvLBoxEntry * pEntry = GetFirstEntryInView();
            ULONG nParentPos = 0;
            while (pEntry != 0 && pEntry != pParent) {
                ++nParentPos;
                pEntry = GetNextEntryInView( pEntry );
            }

            if ((nParentPos + nVisibleChildren + 1) > nVisibleEntries)
                ScrollOutputArea( static_cast<short>(
                                      nVisibleEntries -
                                      (nParentPos + nVisibleChildren + 1) ) );
        }
    }
    return ret;
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::DataChanged( DataChangedEvent const & evt )
{
    SvTreeListBox::DataChanged( evt );
    if (evt.GetType() == DATACHANGED_SETTINGS &&
        (evt.GetFlags() & SETTINGS_STYLE) != 0 &&
        m_hiContrastMode != (bool)GetDisplayBackground().GetColor().IsDark())
    {
        m_hiContrastMode = ! m_hiContrastMode;

        // Update all images as we changed from/to high contrast mode:
        for ( SvLBoxEntry * entry = First(); entry != 0; entry = Next(entry) )
        {
            NodeImpl * node = NodeImpl::get(entry);
            Image img( node->getIcon() );
            SetExpandedEntryBmp( entry, img );
            SetCollapsedEntryBmp( entry, img );
        }

        // force redraw:
        Invalidate();
    }
}

//Currently only works for extensions installed in user context
void DialogImpl::TreeListBoxImpl::select(css::uno::Reference<css::deployment::XPackage> const & xPackage)
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    //GetEntryCount includes all child nodes
    if (GetEntryCount() == 0)
    {
        OSL_ASSERT(0);
        return;
    }

    SvLBoxEntry* rootEntry = FirstChild(NULL);
    if (rootEntry)
    {
        sal_Int32 count = GetLevelChildCount(rootEntry);
        for (sal_Int32 pos = 0; pos < count; pos++)
        {
            SvLBoxEntry* entry = GetEntry(rootEntry, pos);
            Reference<css::deployment::XPackage> const & pack = NodeImpl::get(entry)->m_xPackage;
            if (pack == xPackage)
            {
                SelectAll(false); //delete previous selection
                MakeVisible(entry);
                SetCursor(entry);
                break;
            }
        }
    }
}

//##############################################################################

//______________________________________________________________________________
IMPL_LINK( DialogImpl, destroyDialog, DialogImpl *, EMPTYARG )
{
    //When the install queue has not terminated then we repost the event
    //until the installation thread has finished. We must not block the
    //main thread because installing the extensions requires GUI. Therefore
    //blocking here could cause a deadlock.
    if (m_addExtensionQueue->hasTerminated())
    {
        //make sure the updatability thread (controls the update button) terminates
        //this call blocks until the thread is dead.
        m_updatability->stop();

        if (s_closingDialog.is())
        {
            //keep the dialog alive in this scope. disposing will already delete
            //the static reference
            ::rtl::Reference<DialogImpl> dialog( s_closingDialog );
            s_closingDialog.clear();
            try {
                dialog->disposing( lang::EventObject( dialog->m_xDesktop ) );
            }
            catch (RuntimeException &) {
                OSL_ASSERT( 0 );
            }
            dialog->ModelessDialog::Close();
        }
        if (! office_is_running())
            Application::Quit();
    }
    else
    {   //Repost event instead of blocking the main thread.
        Application::PostUserEvent(
            LINK( this, DialogImpl, destroyDialog ), 0 );
        return 0;
    }
    return 0;
}

//______________________________________________________________________________
//Called when clicking the close button
//The actual closing of the dialog is done in
//IMPL_STATIC_LINK( DialogImpl, destroyDialog, void *, EMPTYARG )
BOOL DialogImpl::Close()
{
    {
        //Let the user click Close again in the following rare case.
        if (sal_False == s_closingMutex.tryToAcquire()
            || s_closingDialog.get() != NULL)
            return FALSE;

        //For all new installation request, for example by double-clicking an oxt file,
        // a new DialogImpl is used. So temporarily there could be two dialogs visible.
        //The old one will process all remaining installation request in the AddExtensionQueue
        //and then die.
        //This is necessary for the case when the user clicks close and in the same moment
        //a new installation request arrives.
        //The last reference to this DialogImpl will be removed in destroyDialog where
        //s_closingDialog.clear is called.
        {
            ::osl::MutexGuard g(s_dialogMutex);
            s_closingDialog = s_dialog;
            s_dialog.clear();
        }
        s_closingMutex.release();
    }

    //This call does not block but signals that the installation thread shall terminate.
    m_addExtensionQueue->stop();


    //Do not call m_addExtensionQueue->stop() here. See event handler destroyDialog.
    Application::PostUserEvent(
        LINK( this, DialogImpl, destroyDialog ), 0 );
    return FALSE;
}

// XEventListener
//______________________________________________________________________________
void DialogImpl::disposing( lang::EventObject const & evt )
throw (RuntimeException)
{
    const lang::EventObject evt_( static_cast<OWeakObject *>(this) );

    // see destroyDialog
    bool shutDown = (evt.Source == m_xDesktop);
    {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        // remove contexts:
        SvLBoxEntry * entry = m_treelb->First();
        while (entry != 0)
        {
            ::rtl::Reference<NodeImpl> node( NodeImpl::get(entry) );
            entry = m_treelb->NextSibling(entry);
            if (shutDown ||
                node->m_xPackageManager
                ->getContext().matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:") ))
                node->disposing( evt_ );
        }

        if (shutDown)
        {
            m_selectionBox->m_bShutDown = true;
        }
    } // release Solar mutex which will be used by the extension queue.

    if (shutDown)
    {
        //Make sure we destroy all threads before destroying the dialog.
        //s_dialog may not contain this anymore but a new dialog
        m_updatability->stop();
        m_addExtensionQueue->stopAndWait();
        {
            ::osl::MutexGuard g(s_dialogMutex);
            if (static_cast<DialogImpl*>(s_dialog.get()) == static_cast<DialogImpl*>(this))
                s_dialog.clear();
        }
        {
            ::osl::MutexGuard g(s_closingMutex);
            if (static_cast<DialogImpl*>(s_closingDialog.get()) == static_cast<DialogImpl*>(this))
                s_closingDialog.clear();
        }
    }

    if (m_xTdocRoot.is()) {
        m_xTdocRoot->removeContentEventListener( this );
        m_xTdocRoot.clear();
    }
    if (shutDown && m_xDesktop.is()) {
        m_xDesktop->removeTerminateListener( this );
        m_xDesktop.clear();
    }
}


// XTerminateListener
//______________________________________________________________________________
void DialogImpl::queryTermination( lang::EventObject const & )
    throw (frame::TerminationVetoException, RuntimeException)
{
    // If we did not threw an exception anyway, we would have to do it
    //as long as there are queued installation requests for extensions.
    //See m_addExtensionsQueue.
    throw frame::TerminationVetoException(
        OUSTR("The office cannot be closed while the Extension Manager is running"),
        Reference<XInterface>(static_cast<frame::XTerminateListener*>(this), UNO_QUERY));
}

//______________________________________________________________________________
void DialogImpl::notifyTermination( lang::EventObject const & evt )
    throw (RuntimeException)
{
   if (m_updatability.get() != NULL)
        m_updatability->stop();
   if (m_addExtensionQueue.get() != NULL)
       m_addExtensionQueue->stopAndWait();

    disposing( evt );
}

// XContentEventListener
//______________________________________________________________________________
void DialogImpl::contentEvent( ContentEvent const & evt )
    throw (RuntimeException)
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );

    OSL_ASSERT( evt.Id == m_xTdocRoot->getIdentifier() );
    try {
        switch (evt.Action) {
        case ContentAction::INSERTED:
        {
            ::ucbhelper::Content ucbContent( evt.Content, 0 );
            OUString factoryURL;
            Reference<XInterface> xDocumentModel;
            if (ucbContent.getPropertyValue(
                    OUSTR("DocumentModel") ) >>= xDocumentModel)
            {
                Reference< ::com::sun::star::frame::XModuleManager >
                    xModuleManager(
                        m_xComponentContext->getServiceManager()
                        ->createInstanceWithContext(
                            OUSTR("com.sun.star.frame.ModuleManager"),
                            m_xComponentContext ),
                        UNO_QUERY_THROW );
                Reference<container::XNameAccess> xModuleConfig(
                    xModuleManager, UNO_QUERY_THROW );
                // get the long name of the document:
                const OUString appModule( xModuleManager->identify(
                                              xDocumentModel ) );
                const Any any_appModule( xModuleConfig->getByName(appModule) );
                const Sequence<beans::PropertyValue> moduleDescr(
                    any_appModule.get< Sequence<beans::PropertyValue> >() );
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if (moduleDescr[ pos ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "ooSetupFactoryEmptyDocumentURL") )) {
                        moduleDescr[ pos ].Value >>= factoryURL;
                        break;
                    }
                }
            }

            m_treelb->addNode(
                0 /* no parent */,
                ucbContent.getPropertyValue( OUSTR("Title") ).get<OUString>(),
                factoryURL,
                m_xPkgMgrFac->getPackageManager(
                    makeURL( ucbContent.getURL(), OUSTR("uno_packages") ) ),
                Reference<deployment::XPackage>(),
                Reference<XCommandEnvironment>() );
            break;
        }
        case ContentAction::REMOVED: {
            ::ucbhelper::Content ucbContent( evt.Content, 0 );
            OUString context(
                makeURL( ucbContent.getURL(), OUSTR("uno_packages") ) );

            // find tdoc context and remove:
            SvLBoxEntry * entry = m_treelb->First();
            while (entry != 0)
            {
                ::rtl::Reference<NodeImpl> node( NodeImpl::get(entry) );
                entry = m_treelb->NextSibling(entry);
                if (node->m_xPackageManager->getContext().equals( context )) {
                    node->disposing( lang::EventObject(
                                         static_cast<OWeakObject *>(this) ) );
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("unexpected exception occured!"),
            static_cast<OWeakObject *>(this), exc );
    }
}

}

