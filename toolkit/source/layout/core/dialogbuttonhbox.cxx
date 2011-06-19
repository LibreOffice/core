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

#include <awt/vclxbutton.hxx>
#include <tools/debug.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <vcl/button.hxx>

#include "dialogbuttonhbox.hxx"
#include "flow.hxx"
#include "proplist.hxx"

namespace layoutimpl
{

using namespace css;

//FIXME: how to set platform-dependant variables?
DialogButtonHBox::Ordering const DialogButtonHBox::DEFAULT_ORDERING =
#if defined( MACOSX )
    DialogButtonHBox::MACOS;
#elif defined( SAL_W32 )
DialogButtonHBox::WINDOWS;
#elif defined( ENABLE_KDE )
DialogButtonHBox::KDE;
#else /* !MACOSX && !SAL_W32 && !ENABLE_KDE */
DialogButtonHBox::GNOME;
#endif /* !MACOSX && !SAL_W32 && !ENABLE_KDE */

DialogButtonHBox::DialogButtonHBox()
    : HBox()
    , mnOrdering( DEFAULT_ORDERING )
    , mFlow()
    , mpAction( 0 )
    , mpAffirmative( 0 )
    , mpAlternate( 0 )
    , mpApply( 0 )
    , mpCancel( 0 )
    , mpFlow( createChild( uno::Reference< awt::XLayoutConstrains > ( &mFlow ) ) )
    , mpHelp( 0 )
    , mpReset( 0 )
{
    mbHomogeneous = true;
}

void
DialogButtonHBox::setOrdering( rtl::OUString const& ordering )
{
    if ( ordering.equalsIgnoreAsciiCaseAscii( "GNOME" ) )
        mnOrdering = GNOME;
    else if ( ordering.equalsIgnoreAsciiCaseAscii( "KDE" ) )
        mnOrdering = KDE;
    else if ( ordering.equalsIgnoreAsciiCaseAscii( "MacOS" ) )
        mnOrdering = MACOS;
    else if ( ordering.equalsIgnoreAsciiCaseAscii( "Windows" ) )
        mnOrdering = WINDOWS;
    else
    {
        OSL_TRACE( "DialogButtonHBox: no such ordering: %s", OUSTRING_CSTR( ordering ) );
    }
}

void
DialogButtonHBox::addChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
    throw ( uno::RuntimeException, awt::MaxChildrenException )
{
    if ( !xChild.is() )
        return;

    ChildData *p = createChild( xChild );

#define IS_BUTTON(t) dynamic_cast<VCLX##t##Button *>( xChild.get () )

    /* Sort Retry as Action */
    if ( !mpAction && IS_BUTTON( Retry ) )
        mpAction = p;
    else if ( !mpAffirmative && IS_BUTTON( OK ) )
        mpAffirmative = p;
    else if ( !mpAffirmative && IS_BUTTON( Yes ) )
        mpAffirmative = p;
    else if ( !mpAlternate && IS_BUTTON( No ) )
        mpAlternate = p;
    /* Sort Ignore as Alternate */
    else if ( !mpAlternate && IS_BUTTON( Ignore ) )
        mpAlternate = p;
    else if ( !mpApply && IS_BUTTON( Apply ) )
        mpApply = p;
    else if ( !mpCancel && IS_BUTTON( Cancel ) )
        mpCancel = p;
    /* Let the user overwrite Flow */
    else if ( /* !mpFlow && */ dynamic_cast<Flow *>( xChild.get () ) )
        mpFlow = p;
    else if ( !mpHelp && IS_BUTTON( Help ) )
        mpHelp = p;
    else if ( !mpReset && IS_BUTTON( Reset ) )
        mpReset = p;
    else
        maOther.push_back( p );
    orderChildren();
    setChildParent( xChild );
    queueResize();
}

void
DialogButtonHBox::orderChildren()
{
    if ( mnOrdering == WINDOWS )
        windowsOrdering();
    else if ( mnOrdering == MACOS )
        macosOrdering();
    else if ( mnOrdering == KDE )
        kdeOrdering();
    else if ( 1 || mnOrdering == GNOME )
        gnomeOrdering();
}

void SAL_CALL
DialogButtonHBox::removeChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
    throw ( uno::RuntimeException)
{
    if ( !xChild.is ())
        return;

    Box_Base::ChildData *p = 0;

    if ( mpAction && mpAction->mxChild == xChild )
        p = mpAction;
    else if ( mpAffirmative && mpAffirmative->mxChild == xChild )
        p = mpAffirmative;
    else if ( mpAlternate && mpAlternate->mxChild == xChild )
        p = mpAlternate;
    else if ( mpApply && mpApply->mxChild == xChild )
        p = mpApply;
    else if ( mpCancel && mpCancel->mxChild == xChild )
        p = mpCancel;
    else if ( mpFlow && mpFlow->mxChild == xChild )
        p = mpFlow;
    else if ( mpReset && mpReset->mxChild == xChild )
        p = mpReset;
    else if ( mpHelp && mpHelp->mxChild == xChild )
        p = mpHelp;
    else
        p = removeChildData( maOther, xChild );

    if ( p )
    {
        delete p;
        unsetChildParent( xChild );
        orderChildren();
        queueResize();
    }
    else
    {
        OSL_FAIL( "DialogButtonHBox: removeChild: no such child" );
    }
}

void
DialogButtonHBox::gnomeOrdering()
{
    std::list< Box_Base::ChildData * > ordered;
    if ( mpHelp )
        ordered.push_back( mpHelp );
    if ( mpReset )
        ordered.push_back( mpReset );
    if ( mpFlow && ( mpHelp || mpReset ) )
        ordered.push_back( mpFlow );
    ordered.insert( ordered.end(), maOther.begin(), maOther.end() );
    if ( mpAction )
        ordered.push_back( mpAction );
    if ( mpApply )
        ordered.push_back( mpApply );
    if ( mpAlternate )
        ordered.push_back( mpAlternate );
    if ( mpCancel )
        ordered.push_back( mpCancel );
    if ( mpAffirmative )
        ordered.push_back( mpAffirmative );
    maChildren = ordered;
}

void
DialogButtonHBox::kdeOrdering()
{
    std::list< Box_Base::ChildData * > ordered;
    if ( mpHelp )
        ordered.push_back( mpHelp );
    if ( mpReset )
        ordered.push_back( mpReset );
    if ( mpFlow && ( mpHelp || mpReset ) )
        ordered.push_back( mpFlow );
    ordered.insert( ordered.end(), maOther.begin(), maOther.end() );
    if ( mpAction )
        ordered.push_back( mpAction );
    if ( mpAffirmative )
        ordered.push_back( mpAffirmative );
    if ( mpApply )
        ordered.push_back( mpApply );
    if ( mpAlternate )
        ordered.push_back( mpAlternate );
    if ( mpCancel )
        ordered.push_back( mpCancel );
    maChildren = ordered;
}

void
DialogButtonHBox::macosOrdering()
{
    std::list< Box_Base::ChildData * > ordered;
    if ( mpHelp )
        ordered.push_back( mpHelp );
    if ( mpReset )
        ordered.push_back( mpReset );
    if ( mpApply )
        ordered.push_back( mpApply );
    if ( mpAction )
        ordered.push_back( mpAction );
    ordered.insert( ordered.end(), maOther.begin(), maOther.end() );
    if ( mpFlow ) // Always flow? && ( maOther.size () || mpHelp || mpReset || mpAction ) )
        ordered.push_back( mpFlow );
    if ( mpAlternate )
        ordered.push_back( mpAlternate );
    if ( mpFlow && mpAlternate )
        ordered.push_back( mpFlow );
    if ( mpCancel )
        ordered.push_back( mpCancel );
    if ( mpAffirmative )
        ordered.push_back( mpAffirmative );
    maChildren = ordered;
}

void
DialogButtonHBox::windowsOrdering()
{
    std::list< Box_Base::ChildData * > ordered;
    if ( mpReset )
        ordered.push_back( mpReset );
    if ( mpReset && mpFlow )
        ordered.push_back( mpFlow );
    if ( mpAffirmative )
        ordered.push_back( mpAffirmative );
    if ( mpAlternate )
        ordered.push_back( mpAlternate );
    if ( mpAction )
        ordered.push_back( mpAction );
    if ( mpCancel )
        ordered.push_back( mpCancel );
    if ( mpApply )
        ordered.push_back( mpApply );
    ordered.insert( ordered.end(), maOther.begin(), maOther.end() );
    if ( mpHelp )
        ordered.push_back( mpHelp );
    maChildren = ordered;
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
