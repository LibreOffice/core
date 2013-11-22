/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include "accessibility/extended/accessiblelistboxentry.hxx"
#include <svtools/svtreebx.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <svtools/stringtransfer.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/controllayout.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <accessibility/helper/accessiblestrings.hrc>
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif
#define ACCESSIBLE_ACTION_COUNT 1

namespace
{
    void checkActionIndex_Impl( sal_Int32 _nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException)
    {
        if ( _nIndex < 0 || _nIndex >= ACCESSIBLE_ACTION_COUNT )
            // only three actions
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }
}

//........................................................................
namespace accessibility
{
    //........................................................................
    // class ALBSolarGuard ---------------------------------------------------------

    /** Aquire the solar mutex. */
    class ALBSolarGuard : public ::vos::OGuard
    {
    public:
        inline ALBSolarGuard() : ::vos::OGuard( Application::GetSolarMutex() ) {}
    };

    // class AccessibleListBoxEntry -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;
    DBG_NAME(AccessibleListBoxEntry)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleListBoxEntry::AccessibleListBoxEntry( SvTreeListBox& _rListBox,
                                                    SvLBoxEntry* _pEntry,
                                                    const Reference< XAccessible >& _xParent ) :

        AccessibleListBoxEntry_BASE ( m_aMutex ),
        ListBoxAccessibleBase( _rListBox ),

        m_pSvLBoxEntry  ( _pEntry ),
        m_nClientId     ( 0 ),
        m_aParent       ( _xParent )

    {
        DBG_CTOR( AccessibleListBoxEntry, NULL );

        _rListBox.FillEntryPath( _pEntry, m_aEntryPath );
    }
    // -----------------------------------------------------------------------------
    AccessibleListBoxEntry::~AccessibleListBoxEntry()
    {
        DBG_DTOR( AccessibleListBoxEntry, NULL );

        if ( IsAlive_Impl() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }

    // IA2 CWS
    void AccessibleListBoxEntry::NotifyAccessibleEvent( sal_Int16 _nEventId,
                                                   const ::com::sun::star::uno::Any& _aOldValue,
                                                   const ::com::sun::star::uno::Any& _aNewValue )
    {
        Reference< uno::XInterface > xSource( *this );
        AccessibleEventObject aEventObj( xSource, _nEventId, _aNewValue, _aOldValue );

        if (m_nClientId)
            comphelper::AccessibleEventNotifier::addEvent( m_nClientId, aEventObj );
    }


    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBox_Impl() const
    {
        Rectangle aRect;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = getListBox()->GetBoundingRect( pEntry );
            SvLBoxEntry* pParent = getListBox()->GetParent( pEntry );
            if ( pParent )
            {
                // position relative to parent entry
                Point aTopLeft = aRect.TopLeft();
                aTopLeft -= getListBox()->GetBoundingRect( pParent ).TopLeft();
                aRect = Rectangle( aTopLeft, aRect.GetSize() );
            }
        }

        return aRect;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen_Impl() const
    {
        Rectangle aRect;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = getListBox()->GetBoundingRect( pEntry );
            Point aTopLeft = aRect.TopLeft();
            aTopLeft += getListBox()->GetWindowExtentsRelative( NULL ).TopLeft();
            aRect = Rectangle( aTopLeft, aRect.GetSize() );
        }

        return aRect;
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleListBoxEntry::IsAlive_Impl() const
    {
        return ( !rBHelper.bDisposed && !rBHelper.bInDispose && isAlive() );
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleListBoxEntry::IsShowing_Impl() const
    {
        Reference< XAccessible > xParent = implGetParentAccessible( );

        sal_Bool bShowing = sal_False;
        Reference< XAccessibleContext > m_xParentContext =
            xParent.is() ? xParent->getAccessibleContext() : Reference< XAccessibleContext >();
        if( m_xParentContext.is() )
        {
            Reference< XAccessibleComponent > xParentComp( m_xParentContext, uno::UNO_QUERY );
            if( xParentComp.is() )
                bShowing = GetBoundingBox_Impl().IsOver( VCLRectangle( xParentComp->getBounds() ) );
        }

        return bShowing;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBox() throw ( lang::DisposedException )
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBox_Impl();
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen() throw ( lang::DisposedException )
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBoxOnScreen_Impl();
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBoxEntry::EnsureIsAlive() const throw ( lang::DisposedException )
    {
        if ( !IsAlive_Impl() )
            throw lang::DisposedException();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString AccessibleListBoxEntry::implGetText()
    {
        ::rtl::OUString sRet;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        //IAccessibility2 Implementation 2009-----
        if ( pEntry )
            sRet = getListBox()->SearchEntryTextWithHeadTitle( pEntry );
        //-----IAccessibility2 Implementation 2009
        return sRet;
    }
    // -----------------------------------------------------------------------------
    Locale AccessibleListBoxEntry::implGetLocale()
    {
        Locale aLocale;
        aLocale = Application::GetSettings().GetUILocale();

        return aLocale;
    }
    void AccessibleListBoxEntry::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }
    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    Sequence< sal_Int8 > AccessibleListBoxEntry::getImplementationId() throw (RuntimeException)
    {
        static ::cppu::OImplementationId* pId = NULL;

        if ( !pId )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    // -----------------------------------------------------------------------------
    // XComponent/ListBoxAccessibleBase
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::dispose() throw ( uno::RuntimeException )
    {
        AccessibleListBoxEntry_BASE::dispose();
    }

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::disposing()
    {
        ALBSolarGuard();
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessible > xKeepAlive( this );

        // Send a disposing to all listeners.
        if ( m_nClientId )
        {
            ::comphelper::AccessibleEventNotifier::TClientId nId = m_nClientId;
            m_nClientId =  0;
            ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
        }

        // clean up
        {

            ListBoxAccessibleBase::disposing();
        }
        m_aParent = WeakReference< XAccessible >();
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AccessibleListBoxEntry::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for ( ; pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported )
            ;

        return pSupported != pEnd;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > AccessibleListBoxEntry::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aSupported(3);
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleContext") );
        aSupported[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleComponent") );
        aSupported[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.AccessibleTreeListBoxEntry") );
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString AccessibleListBoxEntry::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.svtools.AccessibleTreeListBoxEntry") );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleListBoxEntry::getAccessibleContext(  ) throw (RuntimeException)
    {
        EnsureIsAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        sal_Int32 nCount = 0;
        if ( pEntry )
            nCount = getListBox()->GetLevelChildCount( pEntry );

        return nCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

//      SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
//      SvLBoxEntry* pEntry = pParent ? getListBox()->GetEntry( pParent, i ) : NULL;
        SvLBoxEntry* pEntry =GetRealChild(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > AccessibleListBoxEntry::implGetParentAccessible( ) const
    {
        Reference< XAccessible > xParent = (Reference< XAccessible >)m_aParent;
        if ( !xParent.is() )
        {
            DBG_ASSERT( m_aEntryPath.size(), "AccessibleListBoxEntry::getAccessibleParent: invalid path!" );
            if ( 1 == m_aEntryPath.size() )
            {   // we're a top level entry
                // -> our parent is the tree listbox itself
                if ( getListBox() )
                    xParent = getListBox()->GetAccessible( );
            }
            else
            {   // we have a entry as parent -> get it's accessible

                // shorten our access path by one
                ::std::deque< sal_Int32 > aParentPath( m_aEntryPath );
                aParentPath.pop_back();

                // get the entry for this shortened access path
                SvLBoxEntry* pParentEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
                DBG_ASSERT( pParentEntry, "AccessibleListBoxEntry::implGetParentAccessible: could not obtain a parent entry!" );

                //IAccessibility2 Implementation 2009-----
                if ( pParentEntry )
                    pParentEntry = getListBox()->GetParent(pParentEntry);
                //-----IAccessibility2 Implementation 2009
                if ( pParentEntry )
                    xParent = new AccessibleListBoxEntry( *getListBox(), pParentEntry, NULL );
                    // note that we pass NULL here as parent-accessible:
                    // this is allowed, as the AccessibleListBoxEntry class will create it's parent
                    // when needed
            }
        }

        return xParent;
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleParent(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return implGetParentAccessible( );
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        DBG_ASSERT( !m_aEntryPath.empty(), "empty path" );
        return m_aEntryPath.empty() ? -1 : m_aEntryPath.back();
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getRoleType()
    {
        sal_Int32 nCase = 0;
        SvLBoxEntry* pEntry = getListBox()->GetEntry(0);
        if ( pEntry )
        {
            if( pEntry->HasChildsOnDemand() || getListBox()->GetChildCount(pEntry) > 0  )
            {
                nCase = 1;
                return nCase;
            }
        }

        sal_Bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if( !(getListBox()->GetTreeFlags() & TREEFLAG_CHKBTN) )
        {
            if( bHasButtons )
                nCase = 1;
        }
        else
        {
            if( bHasButtons )
                nCase = 2;
             else
                nCase = 3;
        }
        return nCase;
    }
    sal_Int16 SAL_CALL AccessibleListBoxEntry::getAccessibleRole(  ) throw (RuntimeException)
    {
        SvTreeListBox* pBox = getListBox();
        if(pBox)
        {
            short nType = pBox->GetAllEntriesAccessibleRoleType();
            if( nType == TREEBOX_ALLITEM_ACCROLE_TYPE_TREE)
                    return AccessibleRole::TREE_ITEM;
            else if( nType == TREEBOX_ALLITEM_ACCROLE_TYPE_LIST)
                    return AccessibleRole::LIST_ITEM;
        }

        sal_uInt16 treeFlag = pBox->GetTreeFlags();
        if(treeFlag & TREEFLAG_CHKBTN )
        {
            SvLBoxEntry* pEntry = pBox->GetEntryFromPath( m_aEntryPath );
            SvButtonState eState = pBox->GetCheckButtonState( pEntry );
            switch( eState )
            {
                case SV_BUTTON_CHECKED:
                case SV_BUTTON_UNCHECKED:
                    return AccessibleRole::CHECK_BOX;
                case SV_BUTTON_TRISTATE:
                default:
                    return AccessibleRole::LABEL;
            }
        }
        else
        {

        if(getRoleType() == 0)
            return AccessibleRole::LIST_ITEM;
        else
            //o is: return AccessibleRole::LABEL;
            return AccessibleRole::TREE_ITEM;
        }
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getAccessibleDescription(  ) throw (RuntimeException)
    {
        // no description for every item
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if( getAccessibleRole() == AccessibleRole::TREE_ITEM )
        {
            return getListBox()->GetEntryLongDescription( pEntry );
        }
        //want to cout the real column nubmer in the list box.
        sal_uInt16 iRealItemCount = 0;
        sal_uInt16 iCount = 0;
        sal_uInt16 iTotleItemCount = pEntry->ItemCount();
        SvLBoxItem* pItem;
        while( iCount < iTotleItemCount )
        {
            pItem = pEntry->GetItem( iCount );
            if ( pItem->IsA() == SV_ITEM_ID_LBOXSTRING &&
                 static_cast<SvLBoxString*>( pItem )->GetText().Len() > 0 )
            {
                iRealItemCount++;
            }
            iCount++;
        }
        if(iRealItemCount<=1  )
        {
            return ::rtl::OUString();
        }
        else
        {
            return getListBox()->SearchEntryTextWithHeadTitle( pEntry );
        }

    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getAccessibleName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        ::rtl::OUString sRet;
        sRet = implGetText();

        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );

        String altText = getListBox()->GetEntryAltText( pEntry );
        if( altText.Len() > 0 )
        {
            sRet += ::rtl::OUString(' ');
            sRet += altText;
        }

        // IA2 CWS. Removed for now - only used in Sw/Sd/ScContentLBoxString, they should decide if they need this
        // if ( pEntry && pEntry->IsMarked())
        //  sRet = sRet + ::rtl::OUString(TK_RES_STRING(STR_SVT_ACC_LISTENTRY_SELCTED_STATE));

        return sRet;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL AccessibleListBoxEntry::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        Reference< XAccessibleRelationSet > xRelSet;
        Reference< XAccessible > xParent;
        if ( m_aEntryPath.size() > 1 ) // not a root entry
            xParent = implGetParentAccessible();
        if ( xParent.is() )
        {
            utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
            Sequence< Reference< XInterface > > aSequence(1);
            aSequence[0] = xParent;
            pRelationSetHelper->AddRelation(
                AccessibleRelation( AccessibleRelationType::NODE_CHILD_OF, aSequence ) );
            xRelSet = pRelationSetHelper;
        }
        return xRelSet;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL AccessibleListBoxEntry::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
        Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

        if ( IsAlive_Impl() )
        {
            switch(getAccessibleRole())
            {
                case AccessibleRole::LABEL:
                       pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
                       pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
                       pStateSetHelper->AddState( AccessibleStateType::ENABLED );
                    if ( getListBox()->IsInplaceEditingEnabled() )
                           pStateSetHelper->AddState( AccessibleStateType::EDITABLE );
                    if ( IsShowing_Impl() )
                            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
                    break;
                case AccessibleRole::CHECK_BOX:
                       pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
                       pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
                       pStateSetHelper->AddState( AccessibleStateType::ENABLED );
                    if ( IsShowing_Impl() )
                            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
                    break;
            }
            getListBox()->FillAccessibleEntryStateSet(
                getListBox()->GetEntryFromPath( m_aEntryPath ), *pStateSetHelper );
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return xStateSet;
    }
    // -----------------------------------------------------------------------------
    Locale SAL_CALL AccessibleListBoxEntry::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        return implGetLocale();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::containsPoint( const awt::Point& rPoint ) throw (RuntimeException)
    {
        return Rectangle( Point(), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntry( VCLPoint( _aPoint ) );
        if ( !pEntry )
            throw RuntimeException();

        Reference< XAccessible > xAcc;
        AccessibleListBoxEntry* pAccEntry = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
        Rectangle aRect = pAccEntry->GetBoundingBox_Impl();
        if ( aRect.IsInside( VCLPoint( _aPoint ) ) )
            xAcc = pAccEntry;
        return xAcc;
    }
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getBounds(  ) throw (RuntimeException)
    {
        return AWTRectangle( GetBoundingBox() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleListBoxEntry::getLocation(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBox().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleListBoxEntry::getLocationOnScreen(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Size SAL_CALL AccessibleListBoxEntry::getSize(  ) throw (RuntimeException)
    {
        return AWTSize( GetBoundingBox().GetSize() );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::grabFocus(  ) throw (RuntimeException)
    {
        // do nothing, because no focus for each item
    }
    // -----------------------------------------------------------------------------
    sal_Int32 AccessibleListBoxEntry::getForeground(    ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Int32 nColor = 0;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
            if ( xParentComp.is() )
                nColor = xParentComp->getForeground();
        }

        return nColor;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 AccessibleListBoxEntry::getBackground(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Int32 nColor = 0;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
            if ( xParentComp.is() )
                nColor = xParentComp->getBackground();
        }

        return nColor;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleText
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            ::vcl::ControlLayoutData aLayoutData;
            Rectangle aItemRect = GetBoundingBox();
            getListBox()->RecordLayoutData( &aLayoutData, aItemRect );
            Rectangle aCharRect = aLayoutData.GetCharacterBounds( nIndex );
            aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
            aBounds = AWTRectangle( aCharRect );
        }

        return aBounds;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        if(aPoint.X==0 && aPoint.Y==0) return 0;

        sal_Int32 nIndex = -1;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            ::vcl::ControlLayoutData aLayoutData;
            Rectangle aItemRect = GetBoundingBox();
            getListBox()->RecordLayoutData( &aLayoutData, aItemRect );
            Point aPnt( VCLPoint( aPoint ) );
            aPnt += aItemRect.TopLeft();
            nIndex = aLayoutData.GetIndexForPoint( aPnt );
        }

        return nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        String sText = getText();
        if  ( ( 0 > nStartIndex ) || ( sText.Len() <= nStartIndex )
            || ( 0 > nEndIndex ) || ( sText.Len() <= nEndIndex ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nLen = nEndIndex - nStartIndex + 1;
        ::svt::OStringTransfer::CopyString( sText.Copy( (sal_uInt16)nStartIndex, (sal_uInt16)nLen ), getListBox() );

        return sal_True;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleEventBroadcaster
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::addEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (!m_nClientId)
                m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::removeEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, xListener );
            if ( !nListenerCount )
            {
                // no listeners anymore
                // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case somebody calls
                // NotifyAccessibleEvent, again
                sal_Int32 nId = m_nClientId;
                m_nClientId = 0;
                comphelper::AccessibleEventNotifier::revokeClient( nId );

            }
        }
    }
    // -----------------------------------------------------------------------------
    // XAccessibleAction
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleActionCount(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // three actions supported
        SvTreeListBox* pBox = getListBox();
        sal_uInt16 treeFlag = pBox->GetTreeFlags();
        sal_Bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if( (treeFlag & TREEFLAG_CHKBTN) && !bHasButtons)
        {
            sal_Int16 role = getAccessibleRole();
            if ( role == AccessibleRole::CHECK_BOX )
                return 2;
            else if ( role == AccessibleRole::LABEL )
                return 0;
        }
        else
            return ACCESSIBLE_ACTION_COUNT;
        return 0;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::doAccessibleAction( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Bool bRet = sal_False;
        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();
        sal_uInt16 treeFlag = getListBox()->GetTreeFlags();
        if( nIndex == 0 && (treeFlag & TREEFLAG_CHKBTN) )
        {
            if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
            {
                SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
                SvButtonState state = getListBox()->GetCheckButtonState( pEntry );
                if ( state == SV_BUTTON_CHECKED )
                    getListBox()->SetCheckButtonState(pEntry, (SvButtonState)SV_BMP_UNCHECKED);
                else if (state == SV_BMP_UNCHECKED)
                    getListBox()->SetCheckButtonState(pEntry, (SvButtonState)SV_BUTTON_CHECKED);
            }
        }else if( (nIndex == 1 && (treeFlag & TREEFLAG_CHKBTN) ) || (nIndex == 0) )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
            if ( pEntry )
            {
                if ( getListBox()->IsExpanded( pEntry ) )
                    getListBox()->Collapse( pEntry );
                else
                    getListBox()->Expand( pEntry );
                bRet = sal_True;
            }
        }
        return bRet;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getAccessibleActionDescription( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        static const ::rtl::OUString sActionDesc( RTL_CONSTASCII_USTRINGPARAM( "toggleExpand" ) );
        static const ::rtl::OUString sActionDesc1( RTL_CONSTASCII_USTRINGPARAM( "Check" ) );
        static const ::rtl::OUString sActionDesc2( RTL_CONSTASCII_USTRINGPARAM( "UnCheck" ) );
        // sal_Bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvButtonState state = getListBox()->GetCheckButtonState( pEntry );
        sal_uInt16 treeFlag = getListBox()->GetTreeFlags();
        if(nIndex == 0 && (treeFlag & TREEFLAG_CHKBTN))
        {
            if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
            {
                if ( state == SV_BUTTON_CHECKED )
                    return sActionDesc2;
                else if (state == SV_BMP_UNCHECKED)
                    return sActionDesc1;
            }
            else
            {
                //Sometimes, a List or Tree may have both checkbox and label at the same time
                return ::rtl::OUString();
            }
        }else if( (nIndex == 1 && (treeFlag & TREEFLAG_CHKBTN)) || nIndex == 0 )
        {
            //IAccessibility2 Implementation 2009-----
            if( pEntry->HasChilds() || pEntry->HasChildsOnDemand() )
            //-----IAccessibility2 Implementation 2009
                return getListBox()->IsExpanded( pEntry ) ? \
                ::rtl::OUString(TK_RES_STRING(STR_SVT_ACC_ACTION_COLLAPSE)) :
                ::rtl::OUString(TK_RES_STRING(STR_SVT_ACC_ACTION_EXPAND));
            return ::rtl::OUString();

        }
        throw IndexOutOfBoundsException();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleKeyBinding > AccessibleListBoxEntry::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessibleKeyBinding > xRet;
        checkActionIndex_Impl( nIndex );
        // ... which key?
        return xRet;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
// IAccessible2 implementation, 2009
//      SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
//      SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nChildIndex );

        SvLBoxEntry* pEntry =GetRealChild(nChildIndex);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_False );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_True );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;
        }

        return nSelCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_False );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    sal_Unicode SAL_CALL AccessibleListBoxEntry::getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacter( nIndex );
    }
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL AccessibleListBoxEntry::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        ::rtl::OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacterCount(  );
    }

    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getText(  );
    }
    ::rtl::OUString SAL_CALL AccessibleListBoxEntry::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ALBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
    // -----------------------------------------------------------------------------
    // XAccessibleValue
    // -----------------------------------------------------------------------------

    Any AccessibleListBoxEntry::getCurrentValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aValue;
        sal_Int32 level = ((sal_Int32) m_aEntryPath.size() - 1);
        level = level < 0 ?  0: level;
        aValue <<= level;
        return aValue;
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleListBoxEntry::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );


        sal_Bool bReturn = sal_False;
        SvTreeListBox* pBox = getListBox();
        if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
        {
            SvLBoxEntry* pEntry = pBox->GetEntryFromPath( m_aEntryPath );
            if ( pEntry )
            {
                sal_Int32 nValue(0), nValueMin(0), nValueMax(0);
                aNumber >>= nValue;
                getMinimumValue() >>= nValueMin;
                getMaximumValue() >>= nValueMax;

                if ( nValue < nValueMin )
                    nValue = nValueMin;
                else if ( nValue > nValueMax )
                    nValue = nValueMax;

                pBox->SetCheckButtonState(pEntry,  (SvButtonState) nValue );
                bReturn = sal_True;
            }
        }

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    Any AccessibleListBoxEntry::getMaximumValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aValue;
        // SvTreeListBox* pBox = getListBox();
        switch(getAccessibleRole())
        {
            case AccessibleRole::CHECK_BOX:
                aValue <<= (sal_Int32)1;
                break;
            case AccessibleRole::LABEL:
            default:
                break;
        }

        return aValue;
    }

    // -----------------------------------------------------------------------------

    Any AccessibleListBoxEntry::getMinimumValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aValue;
        // SvTreeListBox* pBox = getListBox();
        switch(getAccessibleRole())
        {
            case AccessibleRole::CHECK_BOX:
                aValue <<= (sal_Int32)0;
                break;
            case AccessibleRole::LABEL:
            default:
                break;
        }

        return aValue;
    }

    // -----------------------------------------------------------------------------

    SvLBoxEntry * AccessibleListBoxEntry::GetRealChild(sal_Int32 nIndex)
    {
        SvLBoxEntry* pEntry =NULL;
        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if (pParent)
        {
            pEntry = getListBox()->GetEntry( pParent, nIndex );
            if ( !pEntry && getAccessibleChildCount() > 0 )
            {
                getListBox()->RequestingChilds(pParent);
                pEntry = getListBox()->GetEntry( pParent, nIndex );
            }
        }
        return pEntry;
    }
//........................................................................
}// namespace accessibility
//........................................................................

