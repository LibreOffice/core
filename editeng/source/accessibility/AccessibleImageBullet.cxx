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

#include <tools/gen.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <editeng/unolingu.hxx>
#include "editeng/AccessibleEditableTextPara.hxx"
#include "editeng/AccessibleImageBullet.hxx"
#include <editeng/eerdll.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include "editeng.hrc"
#include <svtools/colorcfg.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{

    AccessibleImageBullet::AccessibleImageBullet ( const uno::Reference< XAccessible >& rParent ) :
        mnParagraphIndex( 0 ),
        mnIndexInParent( 0 ),
        mpEditSource( nullptr ),
        maEEOffset( 0, 0 ),
        mxParent( rParent ),
        // well, that's strictly (UNO) exception safe, though not
        // really robust. We rely on the fact that this member is
        // constructed last, and that the constructor body catches
        // exceptions, thus no chance for exceptions once the Id is
        // fetched. Nevertheless, normally should employ RAII here...
        mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient())
    {
        try
        {
            // Create the state set.
            ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
            mxStateSet = pStateSet;

            // these are always on
            pStateSet->AddState( AccessibleStateType::VISIBLE );
            pStateSet->AddState( AccessibleStateType::SHOWING );
            pStateSet->AddState( AccessibleStateType::ENABLED );
            pStateSet->AddState( AccessibleStateType::SENSITIVE );
        }
        catch( const uno::Exception& ) {}
    }

    AccessibleImageBullet::~AccessibleImageBullet()
    {

        // sign off from event notifier
        if( getNotifierClientId() != -1 )
        {
            try
            {
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
            }
            catch( const uno::Exception& ) {}
        }
    }

    uno::Reference< XAccessibleContext > SAL_CALL AccessibleImageBullet::getAccessibleContext(  )
    {

        // We implement the XAccessibleContext interface in the same object
        return uno::Reference< XAccessibleContext > ( this );
    }

    sal_Int32 SAL_CALL  AccessibleImageBullet::getAccessibleChildCount()
    {

        return 0;
    }

    uno::Reference< XAccessible > SAL_CALL  AccessibleImageBullet::getAccessibleChild( sal_Int32 i )
    {
        (void)i;

        throw lang::IndexOutOfBoundsException("No children available",
                                              uno::Reference< uno::XInterface >
                                              ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy
    }

    uno::Reference< XAccessible > SAL_CALL  AccessibleImageBullet::getAccessibleParent()
    {

        return mxParent;
    }

    sal_Int32 SAL_CALL  AccessibleImageBullet::getAccessibleIndexInParent()
    {

        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL  AccessibleImageBullet::getAccessibleRole()
    {

        return AccessibleRole::GRAPHIC;
    }

    OUString SAL_CALL  AccessibleImageBullet::getAccessibleDescription()
    {

        SolarMutexGuard aGuard;

        // Get the string from the resource for the specified id.
        return EE_RESSTR(RID_SVXSTR_A11Y_IMAGEBULLET_DESCRIPTION);
    }

    OUString SAL_CALL  AccessibleImageBullet::getAccessibleName()
    {

        SolarMutexGuard aGuard;

        // Get the string from the resource for the specified id.
        return EE_RESSTR(RID_SVXSTR_A11Y_IMAGEBULLET_NAME);
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleImageBullet::getAccessibleRelationSet()
    {

        // no relations, therefore empty
        return uno::Reference< XAccessibleRelationSet >();
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleImageBullet::getAccessibleStateSet()
    {

        SolarMutexGuard aGuard;

        // Create a copy of the state set and return it.
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();

        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleImageBullet::getLocale()
    {

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleImageBullet::getLocale: paragraph index value overflow");

        // return locale of first character in the paragraph
        return LanguageTag(GetTextForwarder().GetLanguage( GetParagraphIndex(), 0 )).getLocale();
    }

    void SAL_CALL AccessibleImageBullet::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleImageBullet::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {

        if( getNotifierClientId() != -1 )
        {
            const sal_Int32 nListenerCount = ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
            if ( !nListenerCount )
            {
                // no listeners anymore
                // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case somebody calls
                // NotifyAccessibleEvent, again
                ::comphelper::AccessibleEventNotifier::TClientId nId( getNotifierClientId() );
                mnNotifierClientId = -1;
                ::comphelper::AccessibleEventNotifier::revokeClient( nId );
            }
        }
    }

    sal_Bool SAL_CALL AccessibleImageBullet::containsPoint( const awt::Point& rPoint )
    {

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::contains: index value overflow");

        awt::Rectangle aTmpRect = getBounds();
        Rectangle aRect( Point(aTmpRect.X, aTmpRect.Y), Size(aTmpRect.Width, aTmpRect.Height) );
        Point aPoint( rPoint.X, rPoint.Y );

        return aRect.IsInside( aPoint );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleImageBullet::getAccessibleAtPoint( const awt::Point& /*aPoint*/ )
    {

        // as we have no children, empty reference
        return uno::Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleImageBullet::getBounds(  )
    {

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getBounds: index value overflow");

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( GetParagraphIndex() );
        Rectangle aParentRect = rCacheTF.GetParaBounds( GetParagraphIndex() );

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
            Point aOffset = maEEOffset;

            return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                                   aScreenRect.Top() + aOffset.Y(),
                                   aScreenRect.GetSize().Width(),
                                   aScreenRect.GetSize().Height() );
        }

        return awt::Rectangle();
    }

    awt::Point SAL_CALL AccessibleImageBullet::getLocation(  )
    {

        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleImageBullet::getLocationOnScreen(  )
    {

        SolarMutexGuard aGuard;

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

        throw uno::RuntimeException("Cannot access parent",
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    awt::Size SAL_CALL AccessibleImageBullet::getSize(  )
    {

        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    void SAL_CALL AccessibleImageBullet::grabFocus(  )
    {

        throw uno::RuntimeException("Not focusable",
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    sal_Int32 SAL_CALL AccessibleImageBullet::getForeground(  )
    {

        // #104444# Added to XAccessibleComponent interface
        svtools::ColorConfig aColorConfig;
        sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        return static_cast<sal_Int32>(nColor);
    }

    sal_Int32 SAL_CALL AccessibleImageBullet::getBackground(  )
    {

        // #104444# Added to XAccessibleComponent interface
        Color aColor( Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor() );

        // the background is transparent
        aColor.SetTransparency( 0xFF);

        return static_cast<sal_Int32>( aColor.GetColor() );
    }

    OUString SAL_CALL AccessibleImageBullet::getImplementationName()
    {

        return OUString("AccessibleImageBullet");
    }

    sal_Bool SAL_CALL AccessibleImageBullet::supportsService (const OUString& sServiceName)
    {

        return cppu::supportsService(this, sServiceName);
    }

    uno::Sequence< OUString > SAL_CALL AccessibleImageBullet::getSupportedServiceNames()
    {

        const OUString sServiceName ("com.sun.star.accessibility.AccessibleContext");
        return uno::Sequence< OUString > (&sServiceName, 1);
    }

    void AccessibleImageBullet::SetIndexInParent( sal_Int32 nIndex )
    {

        mnIndexInParent = nIndex;
    }

    void AccessibleImageBullet::SetEEOffset( const Point& rOffset )
    {

        maEEOffset = rOffset;
    }

    void AccessibleImageBullet::Dispose()
    {

        int nClientId( getNotifierClientId() );

        // #108212# drop all references before notifying dispose
        mxParent = nullptr;
        mnNotifierClientId = -1;
        mpEditSource = nullptr;

        // notify listeners
        if( nClientId != -1 )
        {
            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                // #106234# Delegate to EventNotifier
                ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, xThis );
            }
            catch( const uno::Exception& ) {}
        }
    }

    void AccessibleImageBullet::SetEditSource( SvxEditSource* pEditSource )
    {

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

        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleImageBullet* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        // #106234# Delegate to EventNotifier
        ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                         aEvent );
    }

    void AccessibleImageBullet::SetState( const sal_Int16 nStateId )
    {

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != nullptr &&
            !pStateSet->contains(nStateId) )
        {
            pStateSet->AddState( nStateId );
            FireEvent( AccessibleEventId::STATE_CHANGED, uno::makeAny( nStateId ) );
        }
    }

    void AccessibleImageBullet::UnSetState( const sal_Int16 nStateId )
    {

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != nullptr &&
            pStateSet->contains(nStateId) )
        {
            pStateSet->RemoveState( nStateId );
            FireEvent( AccessibleEventId::STATE_CHANGED, uno::Any(), uno::makeAny( nStateId ) );
        }
    }


    void AccessibleImageBullet::SetParagraphIndex( sal_Int32 nIndex )
    {

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
                FireEvent( AccessibleEventId::DESCRIPTION_CHANGED, uno::makeAny( getAccessibleDescription() ), aOldDesc );
                FireEvent( AccessibleEventId::NAME_CHANGED, uno::makeAny( getAccessibleName() ), aOldName );
            }
        }
        catch( const uno::Exception& ) {} // optional behaviour
    }


    SvxEditSource& AccessibleImageBullet::GetEditSource() const
    {

        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException("No edit source, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
    }

    SvxTextForwarder& AccessibleImageBullet::GetTextForwarder() const
    {

        SvxEditSource& rEditSource = GetEditSource();
        SvxTextForwarder* pTextForwarder = rEditSource.GetTextForwarder();

        if( !pTextForwarder )
            throw uno::RuntimeException("Unable to fetch text forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException("Text forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
    }

    SvxViewForwarder& AccessibleImageBullet::GetViewForwarder() const
    {

        SvxEditSource& rEditSource = GetEditSource();
        SvxViewForwarder* pViewForwarder = rEditSource.GetViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException("Unable to fetch view forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) ) ) );  // disambiguate hierarchy
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException("View forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleImageBullet* > (this) )  ) ); // disambiguate hierarchy
    }


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
