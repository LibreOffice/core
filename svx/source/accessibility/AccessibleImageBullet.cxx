/*************************************************************************
 *
 *  $RCSfile: AccessibleImageBullet.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:24 $
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

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleTextType.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

#ifndef _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX
#include "AccessibleEditableTextPara.hxx"
#endif

#ifndef _SVX_ACCESSIBLE_IMAGE_BULLET_HXX
#include "AccessibleImageBullet.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#include "editdata.hxx"
#include "editeng.hxx"
#include "outliner.hxx"
#include "accessibility.hrc"
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility
{
    DBG_NAME( AccessibleImageBullet )

    AccessibleImageBullet::AccessibleImageBullet ( const uno::Reference< XAccessible >& rParent ) :
        mnParagraphIndex( 0 ),
        mnIndexInParent( 0 ),
        mpEditSource( NULL ),
        maEEOffset( 0, 0 ),
        mxParent( rParent ),
        // well, that's strictly (UNO) exception safe, though not
        // really robust. We rely on the fact that this member is
        // constructed last, and that the constructor body catches
        // exceptions, thus no chance for exceptions once the Id is
        // fetched. Nevertheless, normally should employ RAII here...
        mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient())
    {
#ifdef DBG_UTIL
        DBG_CTOR( AccessibleImageBullet, NULL );
        OSL_TRACE( "Received ID: %d", mnNotifierClientId );
#endif

        try
        {
            // Create the state set.
            ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
            mxStateSet = pStateSet;

            // these are always on
            pStateSet->AddState( AccessibleStateType::VISIBLE );
            pStateSet->AddState( AccessibleStateType::SHOWING );
            pStateSet->AddState( AccessibleStateType::ENABLED );
        }
        catch( const uno::Exception& ) {}
    }

    AccessibleImageBullet::~AccessibleImageBullet()
    {
        DBG_DTOR( AccessibleImageBullet, NULL );

        // sign off from event notifier
        if( getNotifierClientId() != -1 )
        {
            try
            {
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
#ifdef DBG_UTIL
                OSL_TRACE( "AccessibleImageBullet revoked ID: %d\n", mnNotifierClientId );
#endif
            }
            catch( const uno::Exception& ) {}
        }
    }

    uno::Any SAL_CALL AccessibleImageBullet::queryInterface (const uno::Type & rType) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return AccessibleImageBulletInterfaceBase::queryInterface(rType);
    }

    uno::Reference< XAccessibleContext > SAL_CALL AccessibleImageBullet::getAccessibleContext(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // We implement the XAccessibleContext interface in the same object
        return uno::Reference< XAccessibleContext > ( this );
    }

    sal_Int32 SAL_CALL  AccessibleImageBullet::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return 0;
    }

    uno::Reference< XAccessible > SAL_CALL  AccessibleImageBullet::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No childs available")),
                                              uno::Reference< uno::XInterface >
                                              ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy
    }

    uno::Reference< XAccessible > SAL_CALL  AccessibleImageBullet::getAccessibleParent() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return mxParent;
    }

    sal_Int32 SAL_CALL  AccessibleImageBullet::getAccessibleIndexInParent() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL  AccessibleImageBullet::getAccessibleRole() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return AccessibleRole::GRAPHIC;
    }

    ::rtl::OUString SAL_CALL  AccessibleImageBullet::getAccessibleDescription() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // throws if defunc
        sal_Int32 nPara( GetParagraphIndex() );

        // Get the string from the resource for the specified id.
        String sStr = ::rtl::OUString( SVX_RESSTR (RID_SVXSTR_A11Y_IMAGEBULLET_DESCRIPTION) );
        String sParaIndex = ::rtl::OUString::valueOf( nPara );
        sStr.SearchAndReplace( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "$(ARG)" )),
                               sParaIndex );

        return ::rtl::OUString( sStr );
    }

    ::rtl::OUString SAL_CALL  AccessibleImageBullet::getAccessibleName() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // throws if defunc
        sal_Int32 nPara( GetParagraphIndex() );

        // Get the string from the resource for the specified id.
        String sStr = ::rtl::OUString( SVX_RESSTR (RID_SVXSTR_A11Y_IMAGEBULLET_NAME) );
        String sParaIndex = ::rtl::OUString::valueOf( nPara );
        sStr.SearchAndReplace( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "$(ARG)" )),
                               sParaIndex );

        return ::rtl::OUString( sStr );
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleImageBullet::getAccessibleRelationSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // no relations, therefore empty
        return uno::Reference< XAccessibleRelationSet >();
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleImageBullet::getAccessibleStateSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // Create a copy of the state set and return it.
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();

        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleImageBullet::getLocale() throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        lang::Locale        aLocale;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleImageBullet::getLocale: paragraph index value overflow");

        // return locale of first character in the paragraph
        return SvxLanguageToLocale(aLocale, GetTextForwarder().GetLanguage( static_cast< USHORT >( GetParagraphIndex() ), 0 ));
    }

    void SAL_CALL AccessibleImageBullet::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleImageBullet::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
    }

    sal_Bool SAL_CALL AccessibleImageBullet::contains( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::contains: index value overflow");

        awt::Rectangle aTmpRect = getBounds();
        Rectangle aRect( Point(aTmpRect.X, aTmpRect.Y), Size(aTmpRect.Width, aTmpRect.Height) );
        Point aPoint( rPoint.X, rPoint.Y );

        return aRect.IsInside( aPoint );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleImageBullet::getAccessibleAt( const awt::Point& aPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // as we have no children, empty reference
        return uno::Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleImageBullet::getBounds(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getBounds: index value overflow");

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< USHORT > (GetParagraphIndex()) );
        Rectangle aParentRect = rCacheTF.GetParaBounds( static_cast< USHORT >( GetParagraphIndex() ) );

        if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
            aBulletInfo.bVisible &&
            aBulletInfo.nType == SVX_NUM_BITMAP )
        {
            Rectangle aRect = aBulletInfo.aBounds;

            // subtract paragraph position (bullet pos is absolute in EditEngine/Outliner)
            aRect.Move( -aParentRect.Left(), -aParentRect.Top() );

            // convert to screen coordinates
            Rectangle aScreenRect = AccessibleEditableTextPara::LogicToPixel( aRect,
                                                                              rCacheTF.GetMapMode(),
                                                                              GetViewForwarder() );

            // offset from shape/cell
            Point aOffset = GetEEOffset();

            return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                                   aScreenRect.Top() + aOffset.Y(),
                                   aScreenRect.GetSize().Width(),
                                   aScreenRect.GetSize().Height() );
        }

        return awt::Rectangle();
    }

    awt::Point SAL_CALL AccessibleImageBullet::getLocation(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleImageBullet::getLocationOnScreen(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // relate us to parent
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if( xParent.is() )
        {
            uno::Reference< XAccessibleComponent > xParentComponent( xParent, uno::UNO_QUERY );
            if( xParentComponent.is() )
            {
                awt::Point aRefPoint = xParentComponent->getLocationOnScreen();
                awt::Point aPoint = getLocation();
                aPoint.X += aRefPoint.X;
                aPoint.Y += aRefPoint.Y;

                return aPoint;
            }
        }

        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Cannot access parent")),
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    awt::Size SAL_CALL AccessibleImageBullet::getSize(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    void SAL_CALL AccessibleImageBullet::grabFocus(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Not focusable")),
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    sal_Int32 SAL_CALL AccessibleImageBullet::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // #104444# Added to XAccessibleComponent interface
        svtools::ColorConfig aColorConfig;
        UINT32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        return static_cast<sal_Int32>(nColor);
    }

    sal_Int32 SAL_CALL AccessibleImageBullet::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // #104444# Added to XAccessibleComponent interface
        Color aColor( Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor() );

        // the background is transparent
        aColor.SetTransparency( 0xFF);

        return static_cast<sal_Int32>( aColor.GetColor() );
    }

    ::rtl::OUString SAL_CALL AccessibleImageBullet::getImplementationName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleImageBullet"));
    }

    sal_Bool SAL_CALL AccessibleImageBullet::supportsService (const ::rtl::OUString& sServiceName) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        //  Iterate over all supported service names and return true if on of them
        //  matches the given name.
        uno::Sequence< ::rtl::OUString> aSupportedServices (
            getSupportedServiceNames ());
        for (int i=0; i<aSupportedServices.getLength(); i++)
            if (sServiceName == aSupportedServices[i])
                return sal_True;
        return sal_False;
    }

    uno::Sequence< ::rtl::OUString> SAL_CALL AccessibleImageBullet::getSupportedServiceNames (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        const ::rtl::OUString sServiceName (RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
        return uno::Sequence< ::rtl::OUString > (&sServiceName, 1);
    }

    ::rtl::OUString SAL_CALL AccessibleImageBullet::getServiceName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.accessibility.AccessibleContext"));
    }

    void AccessibleImageBullet::SetIndexInParent( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        mnIndexInParent = nIndex;
    }

    sal_Int32 AccessibleImageBullet::GetIndexInParent() const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return mnIndexInParent;
    }

    void AccessibleImageBullet::SetEEOffset( const Point& rOffset )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        maEEOffset = rOffset;
    }

    void AccessibleImageBullet::Dispose()
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        // notify listeners
        if( getNotifierClientId() != -1 )
        {
            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                // #106234# Delegate to EventNotifier
                ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( getNotifierClientId(),
                                                                                    xThis );
#ifdef DBG_UTIL
                OSL_TRACE( "AccessibleImageBullet disposed ID: %d", mnNotifierClientId );
#endif
            }
            catch( const uno::Exception& ) {}
        }

        // drop all references
        mxParent = NULL;
        mnNotifierClientId = -1;
        mpEditSource = NULL;
    }

    void AccessibleImageBullet::SetEditSource( SvxEditSource* pEditSource )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        SvxEditSource* pOldEditSource = mpEditSource;

        mpEditSource = pEditSource;

        if( !mpEditSource )
        {
            // going defunc
            UnSetState( AccessibleStateType::SHOWING );
            UnSetState( AccessibleStateType::VISIBLE );
            SetState( AccessibleStateType::INVALID );
            SetState( AccessibleStateType::DEFUNC );

            Dispose();
        }
    }

    void AccessibleImageBullet::FireEvent(const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue ) const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleImageBullet* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        // #106234# Delegate to EventNotifier
        ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                         aEvent );
    }

    void AccessibleImageBullet::GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        FireEvent( nEventId, rNewValue );
    }

    void AccessibleImageBullet::LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        FireEvent( nEventId, uno::Any(), rOldValue );
    }

    void AccessibleImageBullet::SetState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            !pStateSet->contains(nStateId) )
        {
            pStateSet->AddState( nStateId );
            GotPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::ACCESSIBLE_STATE_EVENT );
        }
    }

    void AccessibleImageBullet::UnSetState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            pStateSet->contains(nStateId) )
        {
            pStateSet->RemoveState( nStateId );
            LostPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::ACCESSIBLE_STATE_EVENT );
        }
    }

    int AccessibleImageBullet::getNotifierClientId() const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return mnNotifierClientId;
    }

    void AccessibleImageBullet::SetParagraphIndex( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        uno::Any aOldDesc;
        uno::Any aOldName;

        try
        {
            aOldDesc <<= getAccessibleDescription();
            aOldName <<= getAccessibleName();
        }
        catch( const uno::Exception& ) {} // optional behaviour

        sal_Int32 nOldIndex = mnParagraphIndex;

        mnParagraphIndex = nIndex;

        try
        {
            if( nOldIndex != nIndex )
            {
                // index and therefore description changed
                FireEvent( AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT, uno::makeAny( getAccessibleDescription() ), aOldDesc );
                FireEvent( AccessibleEventId::ACCESSIBLE_NAME_EVENT, uno::makeAny( getAccessibleName() ), aOldName );
            }
        }
        catch( const uno::Exception& ) {} // optional behaviour
    }

    sal_Int32 AccessibleImageBullet::GetParagraphIndex() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return mnParagraphIndex;
    }

    SvxEditSource& AccessibleImageBullet::GetEditSource() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No edit source, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
    }

    SvxTextForwarder& AccessibleImageBullet::GetTextForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        SvxEditSource& rEditSource = GetEditSource();
        SvxTextForwarder* pTextForwarder = rEditSource.GetTextForwarder();

        if( !pTextForwarder )
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to fetch text forwarder, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text forwarder is invalid, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
    }

    SvxViewForwarder& AccessibleImageBullet::GetViewForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        SvxEditSource& rEditSource = GetEditSource();
        SvxViewForwarder* pViewForwarder = rEditSource.GetViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to fetch view forwarder, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("View forwarder is invalid, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) )  ) ); // disambiguate hierarchy
    }

    const Point& AccessibleImageBullet::GetEEOffset() const
    {
        DBG_CHKTHIS( AccessibleImageBullet, NULL );

        return maEEOffset;
    }

} // end of namespace accessibility

