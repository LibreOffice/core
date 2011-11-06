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



#include <awt/vclxbutton.hxx>
#include <tools/debug.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <vcl/button.hxx>

#include "dialogbuttonhbox.hxx"
#include "flow.hxx"
#include "proplist.hxx"

#if TEST_LAYOUT && !defined( DBG_UTIL )
#undef DBG_ERROR
#define DBG_ERROR OSL_TRACE
#undef DBG_ERROR1
#define DBG_ERROR1 OSL_TRACE
#undef DBG_ERROR2
#define DBG_ERROR2 OSL_TRACE
#endif /* TEST_LAYOUT && !DBG_UTIL */

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
        DBG_ERROR1( "DialogButtonHBox: no such ordering: %s", OUSTRING_CSTR( ordering ) );
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
        DBG_ERROR( "DialogButtonHBox: removeChild: no such child" );
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
