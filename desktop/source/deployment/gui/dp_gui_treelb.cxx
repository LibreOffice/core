/*************************************************************************
 *
 *  $RCSfile: dp_gui_treelb.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:05:04 $
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

#include "dp_misc.h"
#include "dp_gui.h"
#include "dp_ucb.h"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "vos/mutex.hxx"
#include "vcl/help.hxx"
#include "tools/urlobj.hxx"
#include "svtools/imagemgr.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "drafts/com/sun/star/frame/XModuleManager.hpp"
#include <hash_set>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_gui
{

//------------------------------------------------------------------------------
PackageState getPackageState( Reference<deployment::XPackage> const & xPackage )
{
    try {
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            xPackage->isRegistered( Reference<task::XAbortChannel>(),
                                    Reference<XCommandEnvironment>() ) );
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
    catch (Exception &) {
        return NOT_AVAILABLE;
    }
}


//==============================================================================
struct NodeImpl : public ::cppu::WeakImplHelper1<util::XModifyListener>
{
    DialogImpl::TreeListBoxImpl * m_treelb;
    SvLBoxEntry * m_lbEntry;
    DialogImpl::TreeListBoxImpl::t_nodeList::iterator m_it;

    OUString m_factoryURL;
    Reference<deployment::XPackageManager> m_xPackageManager;
    Reference<deployment::XPackage> m_xPackage;

    virtual ~NodeImpl();

    inline NodeImpl( DialogImpl::TreeListBoxImpl * treelb,
                     OUString const & factoryURL,
                     Reference<deployment::XPackageManager> const &
                     xPackageManager,
                     Reference<deployment::XPackage> const & xPackage )
        : m_treelb( treelb ),
          m_lbEntry( 0 ),
          m_it( treelb->m_nodes.end() ),
          m_factoryURL( factoryURL ),
          m_xPackageManager( xPackageManager ),
          m_xPackage( xPackage )
        {}

    Image getIcon() const;

    static inline NodeImpl * get( SvLBoxEntry * entry );

    // XEventListener
    virtual void SAL_CALL disposing( lang::EventObject const & evt )
        throw (RuntimeException);
    // XModifyListener
    virtual void SAL_CALL modified( lang::EventObject const & evt )
        throw (RuntimeException);
};

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
        sal_uInt16 id;
        if (m_xPackage->getIcon(
                m_treelb->m_hiContrastMode, true /* small */ ) >>= id)
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
//______________________________________________________________________________
void NodeImpl::disposing( lang::EventObject const & evt )
    throw (RuntimeException)
{
    ::vos::OGuard guard( Application::GetSolarMutex() );

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

struct iface_hash
{
    ::std::size_t operator ()( Reference<deployment::XPackage> const x ) const {
        return reinterpret_cast< ::std::size_t >(
            Reference<XInterface>(x, UNO_QUERY_THROW).get() );
    }
};

// XModifyListener
//______________________________________________________________________________
void NodeImpl::modified( lang::EventObject const & evt )
    throw (RuntimeException)
{
    ::vos::OGuard guard( Application::GetSolarMutex() );

    try {
        if (m_xPackage.is())
        {
            switch (getPackageState( m_xPackage )) {
            case REGISTERED:
                m_treelb->SetEntryText( m_treelb->m_strEnabled, m_lbEntry, 1 );
                break;
            case NOT_REGISTERED:
                m_treelb->SetEntryText( m_treelb->m_strDisabled, m_lbEntry, 1 );
                break;
            case AMBIGUOUS:
                m_treelb->SetEntryText( m_treelb->m_strUnknown, m_lbEntry, 1 );
                break;
            case NOT_AVAILABLE:
                m_treelb->SetEntryText( String(), m_lbEntry, 1 );
                break;
            }

            if (m_treelb->m_dialog != 0)
                m_treelb->m_dialog->updateButtonStates();
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

            ::rtl::Reference<ProgressCommandEnv> cmdEnv(
                new ProgressCommandEnv( m_treelb->m_dialog, String() ) );
            Sequence< Reference<deployment::XPackage> > packages(
                m_xPackageManager->getDeployedPackages(
                    Reference<task::XAbortChannel>(), cmdEnv.get() ) );

            Reference<deployment::XPackage> const * ppackages =
                packages.getConstArray();
            t_set::const_iterator const iEnd( tlboxPackages.end() );
            for ( pos = packages.getLength(); pos--; )
            {
                t_set::iterator iFind( tlboxPackages.find( ppackages[ pos ] ) );
                if (iFind == iEnd)
                    m_treelb->addPackageNode( m_lbEntry, ppackages[ pos ] );
#if OSL_DEBUG_LEVEL > 0
                else
                    tlboxPackages.erase( iFind );
#endif
            }
        }
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        // already handled by UUI handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        m_treelb->m_dialog->errbox( exc );
    }
}

//______________________________________________________________________________
DialogImpl::TreeListBoxImpl::~TreeListBoxImpl()
{
    ::vos::OGuard guard( Application::GetSolarMutex() );
    m_dialog = 0; // in deinit

    lang::EventObject evt;
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
DialogImpl::TreeListBoxImpl::TreeListBoxImpl(
    Window * pParent, DialogImpl * dialog )
    : SvHeaderTabListBox( pParent,
                          WB_CLIPCHILDREN | WB_TABSTOP |
                          WB_HASBUTTONS | WB_HASLINES |
                          /* WB_BORDER | */ WB_HASLINESATROOT |
                          WB_HASBUTTONSATROOT | WB_HIDESELECTION |
                          WB_HSCROLL ),
      m_dialog( dialog ),
      m_currentEntry( 0 ),
      m_hiContrastMode( GetDisplayBackground().GetColor().IsDark() ),
      m_strEnabled( getResourceString(RID_STR_ENABLED) ),
      m_strDisabled( getResourceString(RID_STR_DISABLED) ),
      m_strUnknown( getResourceString(RID_STR_UNKNOWN) ),
      m_strCtxAdd( getResourceString(RID_CTX_ITEM_ADD) ),
      m_strCtxRemove( getResourceString(RID_CTX_ITEM_REMOVE) ),
      m_strCtxEnable( getResourceString(RID_CTX_ITEM_ENABLE) ),
      m_strCtxDisable( getResourceString(RID_CTX_ITEM_DISABLE) ),
      m_strCtxExport( getResourceString(RID_CTX_ITEM_EXPORT) ),
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
    bool sortIn )
{
    NodeImpl * node = new NodeImpl(
        this, factoryURL, xPackageManager, xPackage );
    Reference<util::XModifyListener> xListener( node );
    m_nodes.push_front( xListener );
    node->m_it = m_nodes.begin();

    ULONG pos = LIST_APPEND;
    if (sortIn)
    {
        // sort in after top-level nodes after user, shared:
        pos = parentNode == 0 ? 2 : pos = 0;
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
        node->m_lbEntry = InsertEntry( displayName,
                                       imgIcon, imgIcon,
                                       0, pos, 0xffff, node );
        node->m_xPackageManager->addModifyListener( xListener );
    }
    else
    {
        String name( displayName );
        name.AppendAscii( RTL_CONSTASCII_STRINGPARAM("\t") );
        node->m_lbEntry = InsertEntry( name,
                                       imgIcon, imgIcon,
                                       parentNode, pos, 0xffff, node );
        // update status:
        node->modified( lang::EventObject() );
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
    Reference<deployment::XPackage> const & xPackage )
{
    return addNode( parentNode, xPackage->getDisplayName(),
                    OUString() /* no factory URL */,
                    NodeImpl::get(parentNode)->m_xPackageManager,
                    xPackage );
}

//______________________________________________________________________________
bool DialogImpl::TreeListBoxImpl::isFirstLevelChild( SvLBoxEntry * entry )
{
    entry = GetParent( entry );
    if (entry != 0)
        return GetParent( entry ) == 0;
    else
        return false;
}

//______________________________________________________________________________
OUString DialogImpl::TreeListBoxImpl::getContext( SvLBoxEntry * entry )
{
    return NodeImpl::get(entry)->m_xPackageManager->getContext();
}

//______________________________________________________________________________
Reference<deployment::XPackage>
DialogImpl::TreeListBoxImpl::getPackage( SvLBoxEntry * entry )
{
    return NodeImpl::get(entry)->m_xPackage;
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
    if ((m_dialog->m_addButton->IsEnabled() ||
         m_dialog->m_removeButton->IsEnabled()) &&
        (m_dialog->m_enableButton->IsEnabled()||
         m_dialog->m_disableButton->IsEnabled()))
        menu->InsertSeparator();

    if (m_dialog->m_enableButton->IsEnabled())
        menu->InsertItem( RID_BTN_ENABLE, m_strCtxEnable );
    if (m_dialog->m_disableButton->IsEnabled())
        menu->InsertItem( RID_BTN_DISABLE, m_strCtxDisable );
    if (m_dialog->m_exportButton->IsEnabled() &&
        (m_dialog->m_addButton->IsEnabled() ||
         m_dialog->m_removeButton->IsEnabled()))
        menu->InsertSeparator();

    if (m_dialog->m_exportButton->IsEnabled())
        menu->InsertItem( RID_BTN_EXPORT, m_strCtxExport );

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
    default:
        OSL_ENSURE( 0, "### forgot button entry?!" );
        break;
    }
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::MouseMove( MouseEvent const & evt )
{
    Point pos = evt.GetPosPixel();
    m_currentEntry = GetCurEntry();

    if (m_currentEntry != 0 && GetEntry( pos ) == m_currentEntry) {
        m_timer.Start();
    }
    else {
        Help::ShowBalloon( this, pos, String() );
        m_timer.Stop();
    }
}

//______________________________________________________________________________
IMPL_LINK( DialogImpl::TreeListBoxImpl, TimerHandler, Timer *, timer )
{
    m_timer.Stop();
    Point pos = GetPointerPosPixel();
    SvLBoxEntry * currentEntry = GetCurEntry();
    if (currentEntry != 0 &&
        GetEntry( pos ) == currentEntry &&
        m_currentEntry == currentEntry ) // still the same?
    {
        NodeImpl * node = NodeImpl::get(currentEntry);
        String balloon;
        if (node->m_xPackage.is())
        {
            ::rtl::OUStringBuffer buf;
            buf.append( node->m_xPackage->getDescription() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\n(") );
            buf.append( node->m_xPackage->getMediaType() );
            buf.append( static_cast<sal_Unicode>(')') );
            balloon = buf.makeStringAndClear();
        }
        else if (node->m_factoryURL.getLength() > 0)
        {
            balloon = SvFileInformationManager::GetDescription(
                INetURLObject(node->m_factoryURL) );
        }

        if (balloon.Len() > 0)
            Help::ShowBalloon(
                this, OutputToScreenPixel( pos ), balloon );
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
    SvLBoxEntry * currentEntry = GetCurEntry();
    if (currentEntry != 0 && isFirstLevelChild( currentEntry ))
    {
        KeyCode key = evt.GetKeyCode();
        if (key == KEY_DELETE) {
            // remove selected:
            if (m_dialog != 0)
                m_dialog->m_removeButton->Click();
            return;
        }
    }
    SvTreeListBox::KeyInput( evt );
}

//______________________________________________________________________________
void DialogImpl::TreeListBoxImpl::RequestingChilds( SvLBoxEntry * pParent )
{
    try {
        if (GetChildCount( pParent ) > 0)
            return;

        Sequence< Reference<deployment::XPackage> > packages;
        NodeImpl * parentNode = NodeImpl::get(pParent);

        ::rtl::Reference<ProgressCommandEnv> cmdEnv(
            new ProgressCommandEnv( m_dialog, String() ) );
        if (parentNode->m_xPackage.is()) {
            packages = parentNode->m_xPackage->getBundle(
                Reference<task::XAbortChannel>(), cmdEnv.get() );
        }
        else { // is context
            packages = parentNode->m_xPackageManager->getDeployedPackages(
                Reference<task::XAbortChannel>(), cmdEnv.get() );
        }
        if (packages.getLength() > 0) {
            Reference<deployment::XPackage> const * ppackages =
                packages.getConstArray();
            SetUpdateMode(FALSE);
            for ( sal_Int32 pos = packages.getLength(); pos--; )
                addPackageNode( parentNode->m_lbEntry, ppackages[ pos ] );
            SetUpdateMode(TRUE);
        }
    }
    catch (CommandFailedException &) {
        // already handled by UUI handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        m_dialog->errbox( exc );
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

//##############################################################################

//______________________________________________________________________________
IMPL_STATIC_LINK( DialogImpl, destroyDialog, void *, p )
{
    if (s_dialog.get() != 0)
    {
        ::rtl::Reference<DialogImpl> dialog( s_dialog );
        s_dialog.clear();
        try {
            dialog->disposing( lang::EventObject( dialog->m_xDesktop ) );
        }
        catch (RuntimeException &) {
            OSL_ASSERT( 0 );
        }
    }

    if (! office_is_running())
        Application::Quit();

    return 0;
}

//______________________________________________________________________________
BOOL DialogImpl::Close()
{
    Application::PostUserEvent(
        STATIC_LINK( 0, DialogImpl, destroyDialog ), 0 );
    return ModelessDialog::Close();
}

// XEventListener
//______________________________________________________________________________
void DialogImpl::disposing( lang::EventObject const & evt )
    throw (RuntimeException)
{
    lang::EventObject evt_( static_cast<OWeakObject *>(this) );
    {
        bool shutDown = (evt.Source == m_xDesktop);
        {
            ::vos::OGuard guard( Application::GetSolarMutex() );
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
                s_dialog.clear();
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
}

// XTerminateListener
//______________________________________________________________________________
void DialogImpl::queryTermination( lang::EventObject const & evt )
    throw (frame::TerminationVetoException, RuntimeException)
{
}

//______________________________________________________________________________
void DialogImpl::notifyTermination( lang::EventObject const & evt )
    throw (RuntimeException)
{
    disposing( evt );
}

// XContentEventListener
//______________________________________________________________________________
void DialogImpl::contentEvent( ContentEvent const & evt )
    throw (RuntimeException)
{
    ::vos::OGuard guard( Application::GetSolarMutex() );

    OSL_ASSERT( evt.Id == m_xTdocRoot->getIdentifier() );
    try {
        switch (evt.Action) {
        case ContentAction::INSERTED:
        {
            ::ucb::Content ucbContent( evt.Content, 0 );
            OUString factoryURL;
            Reference<XInterface> xDocumentModel;
            if (ucbContent.getPropertyValue(
                    OUSTR("DocumentModel") ) >>= xDocumentModel)
            {
                Reference< ::drafts::com::sun::star::frame::XModuleManager >
                    xModuleManager(
                        m_xComponentContext->getServiceManager()
                        ->createInstanceWithContext(
                            OUSTR("drafts." // xxx todo
                                  "com.sun.star.frame.ModuleManager"),
                            m_xComponentContext ),
                        UNO_QUERY_THROW );
                Reference<container::XNameAccess> xModuleConfig(
                    xModuleManager, UNO_QUERY_THROW );
                // get the long name of the document:
                OUString appModule( xModuleManager->identify(
                                        xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                extract_throw(
                    &moduleDescr, xModuleConfig->getByName(appModule) );
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if (pmoduleDescr[ pos ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "ooSetupFactoryEmptyDocumentURL") )) {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        break;
                    }
                }
            }

            m_treelb->addNode(
                0 /* no parent */,
                extract_throw<OUString>( ucbContent.getPropertyValue(
                                             OUSTR("Title") ) ),
                factoryURL,
                m_xPkgMgrFac->getPackageManager(
                    make_url( ucbContent.getURL(), OUSTR("uno_packages") ) ),
                Reference<deployment::XPackage>() );
            break;
        }
        case ContentAction::REMOVED: {
            ::ucb::Content ucb_content( evt.Content, 0 );
            OUString context(
                make_url( ucb_content.getURL(), OUSTR("uno_packages") ) );

            // find tdoc context and remove:
            SvLBoxEntry * entry = m_treelb->First();
            while (entry != 0)
            {
                ::rtl::Reference<NodeImpl> node( NodeImpl::get(entry) );
                entry = m_treelb->NextSibling(entry);
                if (node->m_xPackageManager->getContext().equals( context ))
                {
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

