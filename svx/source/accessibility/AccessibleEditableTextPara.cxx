/*************************************************************************
 *
 *  $RCSfile: AccessibleEditableTextPara.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: thb $ $Date: 2002-05-23 12:44:03 $
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

#pragma hdrstop

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

#include <limits.h>
#include <vector>
#include <algorithm>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
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

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#ifndef _SVX_UNOSHAPE_HXX
#include "unoshape.hxx"
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

#ifndef _SVX_UNOPRACC_HXX
#include "unopracc.hxx"
#endif

#ifndef _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX
#include "AccessibleEditableTextPara.hxx"
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;


//------------------------------------------------------------------------
//
// AccessibleEditableTextPara implementation
//
//------------------------------------------------------------------------

namespace accessibility
{

    AccessibleEditableTextPara::AccessibleEditableTextPara( const uno::Reference< XAccessible >& rParent ) :
        mnParagraphIndex( 0 ),
        mnIndexInParent( 0 ),
        mpEditSource( NULL ),
        maEEOffset( 0, 0 ),
        mxParent( rParent ),
        maStateListeners( maMutex )
    {
        // Create the state set.
        ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
        mxStateSet = pStateSet;

        // these are always on
        pStateSet->AddState( AccessibleStateType::MULTILINE );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
    }

    AccessibleEditableTextPara::~AccessibleEditableTextPara()
    {
    }

    void AccessibleEditableTextPara::SetIndexInParent( sal_Int32 nIndex )
    {
        mnIndexInParent = nIndex;
    }

    sal_Int32 AccessibleEditableTextPara::GetIndexInParent() const
    {
        return mnIndexInParent;
    }

    void AccessibleEditableTextPara::SetParagraphIndex( sal_Int32 nIndex )
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

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetParagraphIndex(mnParagraphIndex);

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

    sal_Int32 AccessibleEditableTextPara::GetParagraphIndex() const throw (uno::RuntimeException)
    {
        return mnParagraphIndex;
    }

    void AccessibleEditableTextPara::SetEditSource( SvxEditSourceAdapter* pEditSource )
    {
        SvxEditSource* pOldEditSource = mpEditSource;

        mpEditSource = pEditSource;

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEditSource(pEditSource);

        if( !mpEditSource )
        {
            // going defunc
            UnSetState( AccessibleStateType::SHOWING );
            UnSetState( AccessibleStateType::VISIBLE );
            SetState( AccessibleStateType::INVALID );
            SetState( AccessibleStateType::DEFUNC );

            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                lang::EventObject aEvent (xThis);
                maStateListeners.disposeAndClear( aEvent );
            }
            catch( const uno::Exception& ) {}
        }
        else if( !pOldEditSource )
        {
            // going alive
            UnSetState( AccessibleStateType::DEFUNC );
            UnSetState( AccessibleStateType::INVALID );
            SetState( AccessibleStateType::VISIBLE );
            SetState( AccessibleStateType::SHOWING );
        }
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex )
    {
        // check overflow
        DBG_ASSERT(nStartEEIndex >= 0 && nStartEEIndex <= USHRT_MAX &&
                   nEndEEIndex >= 0 && nEndEEIndex <= USHRT_MAX &&
                   GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::MakeSelection: index value overflow");

        return ESelection( static_cast< USHORT >( GetParagraphIndex() ), static_cast< USHORT >( nStartEEIndex ),
                           static_cast< USHORT >( GetParagraphIndex() ), static_cast< USHORT >( nEndEEIndex ) );
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nEEIndex )
    {
        return MakeSelection( nEEIndex, nEEIndex+1 );
    }

    void AccessibleEditableTextPara::CheckIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        if( nIndex < 0 || nIndex >= getCharacterCount() )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleEditableTextPara: character index out of bounds")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckPosition( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        if( nIndex < 0 || nIndex > getCharacterCount() )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleEditableTextPara: character position out of bounds")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        CheckPosition( nStart );
        CheckPosition( nEnd );
    }

    String AccessibleEditableTextPara::GetText( sal_Int32 nIndex ) throw (uno::RuntimeException)
    {
        return GetTextForwarder().GetText( MakeSelection(nIndex) );
    }

    String AccessibleEditableTextPara::GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (uno::RuntimeException)
    {
        return GetTextForwarder().GetText( MakeSelection(nStartIndex, nEndIndex) );
    }

    USHORT AccessibleEditableTextPara::GetTextLen() const throw (uno::RuntimeException)
    {
        return GetTextForwarder().GetTextLen( static_cast< USHORT >( GetParagraphIndex() ) );
    }

    sal_Bool AccessibleEditableTextPara::IsVisible() const
    {
        return mpEditSource ? sal_True : sal_False ;
    }

    uno::Reference< XAccessibleText > AccessibleEditableTextPara::GetParaInterface( sal_Int32 nIndex )
    {
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if( xParent.is() )
        {
            uno::Reference< XAccessibleContext > xParentContext = xParent->getAccessibleContext();
            if( xParentContext.is() )
            {
                uno::Reference< XAccessible > xPara = xParentContext->getAccessibleChild( nIndex );
                if( xPara.is() )
                    return uno::Reference< XAccessibleText > ( xPara, uno::UNO_QUERY );
            }
        }

        return uno::Reference< XAccessibleText >();
    }

    SvxEditSourceAdapter& AccessibleEditableTextPara::GetEditSource() const throw (uno::RuntimeException)
    {
        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No edit source, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
    }

    SvxAccessibleTextAdapter& AccessibleEditableTextPara::GetTextForwarder() const throw (uno::RuntimeException)
    {
        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextAdapter* pTextForwarder = rEditSource.GetTextForwarderAdapter();

        if( !pTextForwarder )
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to fetch text forwarder, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text forwarder is invalid, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
    }

    SvxViewForwarder& AccessibleEditableTextPara::GetViewForwarder() const throw (uno::RuntimeException)
    {
        SvxEditSource& rEditSource = GetEditSource();
        SvxViewForwarder* pViewForwarder = rEditSource.GetViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to fetch view forwarder, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("View forwarder is invalid, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
    }

    SvxAccessibleTextEditViewAdapter& AccessibleEditableTextPara::GetEditViewForwarder( sal_Bool bCreate ) const throw (uno::RuntimeException)
    {
        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextEditViewAdapter* pTextEditViewForwarder = rEditSource.GetEditViewForwarderAdapter( bCreate );

        if( !pTextEditViewForwarder )
        {
            if( bCreate )
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to fetch view forwarder, object is defunct")),
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
            else
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No view forwarder, object not in edit mode")),
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
        }

        if( pTextEditViewForwarder->IsValid() )
            return *pTextEditViewForwarder;
        else
        {
            if( bCreate )
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("View forwarder is invalid, object is defunct")),
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
            else
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("View forwarder is invalid, object not in edit mode")),
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
        }
    }

    sal_Bool AccessibleEditableTextPara::HaveEditView() const
    {
        SvxEditSource& rEditSource = GetEditSource();
        SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

        if( !pViewForwarder )
            return sal_False;

        if( !pViewForwarder->IsValid() )
            return sal_False;

        return sal_True;
    }

    sal_Bool AccessibleEditableTextPara::HaveChildren()
    {
        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::HaveChildren: paragraph index value overflow");

        return GetTextForwarder().HaveImageBullet( static_cast< USHORT >(GetParagraphIndex()) );
    }

    sal_Bool AccessibleEditableTextPara::IsActive() const throw (uno::RuntimeException)
    {
        SvxEditSource& rEditSource = GetEditSource();
        SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

        if( !pViewForwarder )
            return sal_False;

        if( pViewForwarder->IsValid() )
            return sal_False;
        else
            return sal_True;
    }

    Rectangle AccessibleEditableTextPara::LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder )
    {
        // convert to screen coordinates
        return Rectangle( rForwarder.LogicToPixel( rRect.TopLeft(), rMapMode ),
                          rForwarder.LogicToPixel( rRect.BottomRight(), rMapMode ) );
    }

    const Point& AccessibleEditableTextPara::GetEEOffset() const
    {
        return maEEOffset;
    }

    void AccessibleEditableTextPara::SetEEOffset( const Point& rOffset )
    {
        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEEOffset(rOffset);

        maEEOffset = rOffset;
    }

    void AccessibleEditableTextPara::FireEvent(const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue) const
    {
        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleEditableTextPara* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        // no locking necessary, OInterfaceIteratorHelper copies listeners if someone removes/adds in between
        ::cppu::OInterfaceIteratorHelper aIter( const_cast< AccessibleEditableTextPara* >(this)->maStateListeners );
        while( aIter.hasMoreElements() )
        {
            uno::Reference < XAccessibleEventListener > xListener( aIter.next(), uno::UNO_QUERY );

            if( xListener.is() )
            {
                try
                {
                    xListener->notifyEvent( aEvent );
                }
                catch( const uno::Exception& )
                {
#ifdef DBG_UTIL
                    DBG_ERROR("AccessibleEditableTextPara::FireEvent: Caught runtime exception from listener, removing object (bridge/listener dead?)");
#endif
                }
            }
        }
    }

    void AccessibleEditableTextPara::GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const
    {
        FireEvent( nEventId, rNewValue );
    }

    void AccessibleEditableTextPara::LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const
    {
        FireEvent( nEventId, uno::Any(), rOldValue );
    }

    void AccessibleEditableTextPara::SetState( const sal_Int16 nStateId )
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            !pStateSet->contains(nStateId) )
        {
            pStateSet->AddState( nStateId );
            GotPropertyEvent( uno::makeAny( nStateId), AccessibleEventId::ACCESSIBLE_STATE_EVENT );
        }
    }

    void AccessibleEditableTextPara::UnSetState( const sal_Int16 nStateId )
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            pStateSet->contains(nStateId) )
        {
            pStateSet->RemoveState( nStateId );
            LostPropertyEvent( uno::makeAny( nStateId), AccessibleEventId::ACCESSIBLE_STATE_EVENT );
        }
    }

    uno::Any SAL_CALL AccessibleEditableTextPara::queryInterface (const uno::Type & rType) throw (uno::RuntimeException)
    {
        uno::Any aRet;

        // must provide XAccesibleText by hand, since it comes publicly inherited by XAccessibleEditableText
        if ( rType == ::getCppuType((uno::Reference< XAccessibleText > *)0) )
        {
            uno::Reference< XAccessibleText > aAccText = this;
            aRet <<= aAccText;
        }
        else if ( rType == ::getCppuType((uno::Reference< XAccessibleEditableText > *)0) )
        {
            uno::Reference< XAccessibleEditableText > aAccEditText = this;
            aRet <<= aAccEditText;
        }
        else
        {
            aRet = AccessibleTextParaInterfaceBase::queryInterface(rType);
        }

        return aRet;
    }

    // XAccessible
    uno::Reference< XAccessibleContext > SAL_CALL AccessibleEditableTextPara::getAccessibleContext() throw (uno::RuntimeException)
    {
        // We implement the XAccessibleContext interface in the same object
        return uno::Reference< XAccessibleContext > ( this );
    }

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return HaveChildren() ? 1 : 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( !HaveChildren() )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No childs available")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy

        if( i != 0 )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid child index")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy

        WeakBullet::HardRefType aChild( maImageBullet.get() );

        if( !aChild.is() )
        {
            // there is no hard reference available, create object then
            AccessibleImageBullet* pChild = new AccessibleImageBullet( uno::Reference< XAccessible >( this ) );
            uno::Reference< XAccessible > xChild( static_cast< ::cppu::OWeakObject* > (pChild), uno::UNO_QUERY );

            if( !xChild.is() )
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Child creation failed")),
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* > (this) ) );

            aChild = WeakBullet::HardRefType( xChild, pChild );

            aChild->SetEditSource( &GetEditSource() );
            aChild->SetParagraphIndex( GetParagraphIndex() );
            aChild->SetIndexInParent( i );

            maImageBullet = aChild;
        }

        return aChild.getRef();
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleParent() throw (uno::RuntimeException)
    {
        return mxParent;
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleIndexInParent() throw (uno::RuntimeException)
    {
        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL AccessibleEditableTextPara::getAccessibleRole() throw (uno::RuntimeException)
    {
        // TODO
        return AccessibleRole::TEXT;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getAccessibleDescription() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString aStr( RTL_CONSTASCII_USTRINGPARAM("Paragraph containing: ") );

        // append first 40 characters from text, or first line, if shorter
        // (writer takes first sentence here, but that's not supported
        // from EditEngine)
        ::rtl::OUString aLine = getTextAtIndex(0, AccessibleTextType::LINE);

        if( aLine.getLength() > MaxDescriptionLen )
        {
            ::rtl::OUString aCurrWord;
            sal_Int32 i;

            // search backward from MaxDescriptionLen for previous word start
            for( aCurrWord=getTextAtIndex(MaxDescriptionLen, AccessibleTextType::WORD),
                     i=MaxDescriptionLen,
                     aLine=::rtl::OUString();
                 i>=0;
                 --i )
            {
                if( getTextAtIndex(i, AccessibleTextType::WORD) != aCurrWord )
                {
                    if( i == 0 )
                        // prevent completely empty string
                        aLine = getTextAtIndex(0, AccessibleTextType::WORD);
                    else
                        aLine = getTextRange(0, i);
                }
            }
        }

        return aStr + aLine;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getAccessibleName() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString aStr( RTL_CONSTASCII_USTRINGPARAM("Paragraph ") );

        aStr += ::rtl::OUString::valueOf( GetParagraphIndex() );

        return aStr;
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleEditableTextPara::getAccessibleRelationSet() throw (uno::RuntimeException)
    {
        // no relations, therefore empty
        return uno::Reference< XAccessibleRelationSet >();
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleEditableTextPara::getAccessibleStateSet() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // Create a copy of the state set and return it.
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();

        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleEditableTextPara::getLocale() throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        lang::Locale        aLocale;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        // return locale of first character in the paragraph
        return SvxLanguageToLocale(aLocale, GetTextForwarder().GetLanguage( static_cast< USHORT >( GetParagraphIndex() ), 0 ));
    }

    void SAL_CALL AccessibleEditableTextPara::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        maStateListeners.addInterface( xListener );
    }

    void SAL_CALL AccessibleEditableTextPara::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        maStateListeners.removeInterface( xListener );
    }

    // XAccessibleComponent
    sal_Bool SAL_CALL AccessibleEditableTextPara::contains( const awt::Point& aTmpPoint ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::contains: index value overflow");

        awt::Rectangle aTmpRect = getBounds();
        Rectangle aRect( Point(aTmpRect.X, aTmpRect.Y), Size(aTmpRect.Width, aTmpRect.Height) );
        Point aPoint( aTmpPoint.X, aTmpPoint.Y );

        return aRect.IsInside( aPoint );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleAt( const awt::Point& _aPoint ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( HaveChildren() )
        {
            // make given position relative
            Point aPoint( _aPoint.X, _aPoint.Y );
            awt::Point aRefPoint = getLocationOnScreen();
            aPoint -= Point( aRefPoint.X, aRefPoint.Y );

            // respect EditEngine offset to surrounding shape/cell
            aPoint -= GetEEOffset();

            // convert to EditEngine coordinate system
            SvxTextForwarder& rCacheTF = GetTextForwarder();
            Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

            EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< USHORT > (GetParagraphIndex()) );

            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
                aBulletInfo.bVisible &&
                aBulletInfo.nType == SVX_NUM_BITMAP )
            {
                Rectangle aRect = aBulletInfo.aBounds;

                if( aRect.IsInside( aLogPoint ) )
                    return getAccessibleChild(0);
            }
        }

        // no children at all, or none at given position
        return uno::Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getBounds() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getBounds: index value overflow");

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetParaBounds( static_cast< USHORT >( GetParagraphIndex() ) );

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

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocation(  ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocationOnScreen(  ) throw (uno::RuntimeException)
    {
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

    awt::Size SAL_CALL AccessibleEditableTextPara::getSize(  ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::isShowing(  ) throw (uno::RuntimeException)
    {
        return IsVisible();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::isVisible(  ) throw (uno::RuntimeException)
    {
        return IsVisible();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::isFocusTraversable(  ) throw (uno::RuntimeException)
    {
        return IsActive();
    }

    void SAL_CALL AccessibleEditableTextPara::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
    {
        // TODO: remove
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Not focusable")),
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    void SAL_CALL AccessibleEditableTextPara::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
    {
        // TODO: remove
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Not focusable")),
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    void SAL_CALL AccessibleEditableTextPara::grabFocus(  ) throw (uno::RuntimeException)
    {
        // set cursor to this paragraph
        setSelection(0,0);
    }

    uno::Any SAL_CALL AccessibleEditableTextPara::getAccessibleKeyBinding(  ) throw (uno::RuntimeException)
    {
        // AFAIK, none.
        return uno::Any();
    }

    // XAccessibleText
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCaretPosition() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( !HaveEditView() )
            return -1;

        ESelection aSelection;
        if( GetEditViewForwarder().GetSelection( aSelection ) &&
            GetParagraphIndex() == aSelection.nEndPara )
        {
            // caret is always nEndPara,nEndPos
            return aSelection.nEndPos;
        }

        // not within this paragraph
        return -1;
    }

    sal_Unicode SAL_CALL AccessibleEditableTextPara::getCharacter( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacter: index value overflow");

        CheckIndex( nIndex );

        return GetText( nIndex ).GetChar(0);
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getCharacterAttributes( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
#if 1
        // TODO: getCharacterAttributes() does not work for paragraphs containing fields (SIGSEV in SvxUnoTextField::~SvxUnoTextField() throw())
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Cannot query XPropertySetInfo")),
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
#else
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();
        USHORT nPara = static_cast< USHORT >( GetParagraphIndex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterAttributes: index value overflow");

        CheckIndex(nIndex);

        // use the portion property map, we're working on single characters
        // TODO: hold it as a member?
        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                               ImplGetSvxTextPortionPropertyMap() );

        aPropSet.SetSelection( MakeSelection( nIndex ) );

        // fetch property names
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = aPropSet.getPropertySetInfo();

        // convert from Any to PropertyValue
        if( !xPropSetInfo.is() )
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Cannot query XPropertySetInfo")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy

        uno::Sequence< beans::Property > aPropertyNames = xPropSetInfo->getProperties();
        sal_Int32 i, nLength( aPropertyNames.getLength() );
        uno::Sequence< beans::PropertyValue > aOutSequence(nLength);
        const beans::Property*  pPropArray = aPropertyNames.getConstArray();
        beans::PropertyValue* pOutArray = aOutSequence.getArray();
        for(i=0; i<nLength; ++i)
        {
            pOutArray->Name = pPropArray->Name;
            pOutArray->Handle = pPropArray->Handle;
            pOutArray->Value = aPropSet.getPropertyValue( pPropArray->Name );
            pOutArray->State = aPropSet.getPropertyState( pPropArray->Name );

            ++pPropArray;
            ++pOutArray;
        }

        return aOutSequence;
#endif
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterBounds: index value overflow");

        CheckIndex( nIndex );

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetCharBounds( static_cast< USHORT >( GetParagraphIndex() ), static_cast< USHORT >( nIndex ) );

        // convert to screen
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

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCharacterCount() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterCount: index value overflow");

        return GetTextLen();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        USHORT nPara, nIndex;

        // offset from surrounding cell/shape
        Point aOffset( GetEEOffset() );
        Point aPoint( rPoint.X - aOffset.X(), rPoint.Y - aOffset.Y() );

        // convert to logical coordinates
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

        if( rCacheTF.GetIndexAtPoint( aLogPoint, nPara, nIndex ) &&
            GetParagraphIndex() == nPara )
        {
            return nIndex;
        }
        else
        {
            // not within our paragraph
            return -1;
        }
    }

    sal_Bool AccessibleEditableTextPara::GetSelection( USHORT& nStartPos, USHORT& nEndPos ) throw (uno::RuntimeException)
    {
        ESelection aSelection;
        USHORT nPara = static_cast< USHORT > ( GetParagraphIndex() );
        if( !GetEditViewForwarder().GetSelection( aSelection ) )
            return sal_False;

        if( aSelection.nStartPara < aSelection.nEndPara )
        {
            if( aSelection.nStartPara > nPara ||
                aSelection.nEndPara < nPara )
                return sal_False;

            if( nPara == aSelection.nStartPara )
                nStartPos = aSelection.nStartPos;
            else
                nStartPos = 0;

            if( nPara == aSelection.nEndPara )
                nEndPos = aSelection.nEndPos;
            else
                nEndPos = GetTextLen();
        }
        else
        {
            if( aSelection.nStartPara < nPara ||
                aSelection.nEndPara > nPara )
                return sal_False;

            if( nPara == aSelection.nStartPara )
                nStartPos = aSelection.nStartPos;
            else
                nStartPos = GetTextLen();

            if( nPara == aSelection.nEndPara )
                nEndPos = aSelection.nEndPos;
            else
                nEndPos = 0;
        }

        return sal_True;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getSelectedText() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectedText: index value overflow");

        if( !HaveEditView() )
            return ::rtl::OUString();

        USHORT nStartPos, nEndPos;
        if( GetSelection( nStartPos, nEndPos ) )
            return GetTextRange( nStartPos, nEndPos );

        return ::rtl::OUString();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionStart() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionStart: index value overflow");

        if( !HaveEditView() )
            return -1;

        USHORT nStartPos, nEndPos;
        if( GetSelection( nStartPos, nEndPos) )
            return nStartPos;
        else
            return -1;
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionEnd() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionEnd: index value overflow");

        if( !HaveEditView() )
            return -1;

        USHORT nStartPos, nEndPos;
        if( GetSelection( nStartPos, nEndPos) )
            return nEndPos;
        else
            return -1;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::setSelection: paragraph index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        return GetEditViewForwarder( sal_True ).SetSelection( MakeSelection(nStartIndex, nEndIndex) );
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getText() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getText: paragraph index value overflow");

        return GetTextRange( 0, GetTextLen() );
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        return GetTextRange(nStartIndex, nEndIndex);
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxTextForwarder&   rCacheTF = GetTextForwarder();
        sal_Int32           nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextAtIndex: paragraph index value overflow");

        sal_Int32 nTextLen = rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) );

        CheckIndex(nIndex);

        ::rtl::OUString aRetVal;

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
                // GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const;
                // beware: array is sorted in start positions, the end positions can overlap!
                // TODO
                break;

            case AccessibleTextType::GLYPH:
                // TODO: CTL?
            case AccessibleTextType::CHARACTER:
                aRetVal = String( getCharacter( nIndex ) );
                break;

            case AccessibleTextType::WORD:
            {
                USHORT nStart, nEnd;
                if( rCacheTF.GetWordIndices( static_cast< USHORT >( nParaIndex ), static_cast< USHORT >( nIndex ), nStart, nEnd ) )
                    aRetVal = GetTextRange( nStart, nEnd );
                break;
            }

            case AccessibleTextType::LINE:
            {
                USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
                sal_Int32 nCurIndex;
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    nCurIndex += rCacheTF.GetLineLen( static_cast< USHORT >( nParaIndex ), nLine);

                    if( nCurIndex > nIndex )
                    {
                        aRetVal = GetTextRange( nCurIndex - rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine), nCurIndex );
                        break;
                    }
                }
                break;
            }

            case AccessibleTextType::SENTENCE:
                // TODO. currently fallback to paragraph.
            case AccessibleTextType::PARAGRAPH:
                aRetVal = getText();
                break;

            default:
                throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text type value out of range")),
                                                      uno::Reference< uno::XInterface >
                                                      ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
        } /* end of switch( aTextType ) */

        return aRetVal;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxTextForwarder&   rCacheTF = GetTextForwarder();
        sal_Int32           nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBeforeIndex: paragraph index value overflow");

        sal_Int32 nTextLen( rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) ) );

        CheckIndex(nIndex);

        ::rtl::OUString aRetVal;

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
                // TODO
                break;

            case AccessibleTextType::GLYPH:
                // TODO: CTL?
            case AccessibleTextType::CHARACTER:
                aRetVal = nIndex > 0 ? String(getCharacter( nIndex-1 )) : String();
                break;

            case AccessibleTextType::WORD:
            {
                USHORT nStart, nEnd;
                if( rCacheTF.GetWordIndices( static_cast< USHORT >( nParaIndex ), static_cast< USHORT >( nIndex ), nStart, nEnd ) )
                {
                    // semantics:
                    //
                    // word w|ord word # word |word word # word| word
                    // ^^^^              ^^^^              ^^^^
                    //

                    // already at the left border
                    if( nStart == 0 )
                        break;

                    // one word to the left
                    if( rCacheTF.GetWordIndices( static_cast< USHORT >( nParaIndex ), static_cast< USHORT >( nStart ), nStart, nEnd ) )
                        aRetVal = GetTextRange(nStart, nEnd);

                    break;
                }
            }

            case AccessibleTextType::LINE:
            {
                USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
                sal_Int32 nCurIndex, nLastIndex, nCurLineLen;
                // get the line before the line the index points into
                for( nLine=0, nCurIndex=0, nLastIndex=0; nLine<nLineCount; ++nLine )
                {
                    nLastIndex = nCurIndex;
                    nCurLineLen = rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine);
                    nCurIndex += nCurLineLen;

                    if( nCurIndex > nIndex &&
                        nLastIndex > nCurLineLen )
                    {
                        aRetVal = GetTextRange( nLastIndex - nCurLineLen, static_cast< USHORT >( nLastIndex ) );
                        break;
                    }
                }
                break;
            }

            case AccessibleTextType::SENTENCE:
                // TODO. currently fallback to paragraph.
            case AccessibleTextType::PARAGRAPH:
                // get paragraph before (convenience? bug? feature?)
                if( nParaIndex &&
                    GetParaInterface( nParaIndex - 1 ).is() )
                {
                    aRetVal = GetParaInterface( nParaIndex - 1 )->getText();
                }
                break;

            default:
                throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text type value out of range")),
                                                      uno::Reference< uno::XInterface >
                                                      ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
        } /* end of switch( aTextType ) */

        return aRetVal;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        sal_Int32 nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBehindIndex: paragraph index value overflow");

        sal_Int32 nTextLen( rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) ) );

        CheckIndex(nIndex);

        ::rtl::OUString aRetVal;

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
                // TODO
                break;

            case AccessibleTextType::GLYPH:
                // TODO: CTL?
            case AccessibleTextType::CHARACTER:
                aRetVal = nIndex < nTextLen-1 ? String(getCharacter( nIndex+1 )) : String();
                break;

            case AccessibleTextType::WORD:
            {
                USHORT nStart, nEnd;
                if( rCacheTF.GetWordIndices( static_cast< USHORT >( nParaIndex ), static_cast< USHORT >( nIndex ), nStart, nEnd ) )
                {
                    // semantics:
                    //
                    // word w|ord word # word |word # word word| word
                    //            ^^^^         ^^^^              ^^^^
                    //

                    // already at the right border
                    if( nEnd >= nTextLen-2 )
                        break;

                    // one word to the right
                    if( rCacheTF.GetWordIndices( static_cast< USHORT >( nParaIndex ), nEnd+1, nStart, nEnd ) )
                        aRetVal = GetTextRange(nStart, nEnd);

                    break;
                }
            }

            case AccessibleTextType::LINE:
            {
                USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
                sal_Int32 nCurIndex;
                // get the line after the line the index points into
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    nCurIndex += rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine);

                    if( nCurIndex > nIndex &&
                        nLine < nLineCount-1 )
                    {
                        aRetVal = GetTextRange( nCurIndex, nCurIndex + rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine+1) );
                        break;
                    }
                }
                break;
            }

            case AccessibleTextType::SENTENCE:
                // TODO. currently fallback to paragraph.
            case AccessibleTextType::PARAGRAPH:
                // get paragraph behind (convenience? bug? feature?)
                if( nParaIndex < rCacheTF.GetParagraphCount() - 1 &&
                    GetParaInterface( nParaIndex + 1 ).is() )
                {
                    aRetVal = GetParaInterface( nParaIndex + 1 )->getText();
                }
                break;

            default:
                throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text type value out of range")),
                                                      uno::Reference< uno::XInterface >
                                                      ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
        } /* end of switch( aTextType ) */

        return aRetVal;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
        SvxTextForwarder& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
        sal_Bool aRetVal;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::copyText: index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        // save current selection
        ESelection aOldSelection;

        rCacheVF.GetSelection( aOldSelection );
        rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );
        aRetVal = rCacheVF.Copy();
        rCacheVF.SetSelection( aOldSelection ); // restore

        return aRetVal;
    }

    // XAccessibleEditableText
    sal_Bool SAL_CALL AccessibleEditableTextPara::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::cutText: index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
            return sal_False; // non-editable area selected

        // don't save selection, might become invalid after cut!
        rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );

        return rCacheVF.Cut();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::pasteText( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::pasteText: index value overflow");

        CheckPosition(nIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
            return sal_False; // non-editable area selected

        // set empty selection (=> cursor) to given index
        rCacheVF.SetSelection( MakeSelection(nIndex) );

        return rCacheVF.Paste();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::deleteText: index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
            return sal_False; // non-editable area selected

        sal_Bool nRet = rCacheTF.Delete( MakeSelection(nStartIndex, nEndIndex) );

        GetEditSource().UpdateData();

        return nRet;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::insertText: index value overflow");

        CheckPosition(nIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
            return sal_False; // non-editable area selected

        // insert given text at empty selection (=> cursor)
        sal_Bool nRet = rCacheTF.InsertText( sText, MakeSelection(nIndex) );

        rCacheTF.QuickFormatDoc();
        GetEditSource().UpdateData();

        return nRet;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::replaceText: index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
            return sal_False; // non-editable area selected

        // insert given text into given range => replace
        sal_Bool nRet = rCacheTF.InsertText( sReplacement, MakeSelection(nStartIndex, nEndIndex) );

        rCacheTF.QuickFormatDoc();
        GetEditSource().UpdateData();

        return nRet;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const uno::Sequence< beans::PropertyValue >& aAttributeSet ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();
        USHORT nPara = static_cast< USHORT >( GetParagraphIndex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::setAttributes: index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
            return sal_False; // non-editable area selected

        // do the indices span the whole paragraph? Then use the outliner map
        // TODO: hold it as a member?
        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                               0 == nStartIndex &&
                                               rCacheTF.GetTextLen(nPara) == nEndIndex ?
                                               ImplGetSvxUnoOutlinerTextCursorPropertyMap() :
                                               ImplGetSvxTextPortionPropertyMap() );

        aPropSet.SetSelection( MakeSelection(nStartIndex, nEndIndex) );

        // convert from PropertyValue to Any
        sal_Int32 i, nLength( aAttributeSet.getLength() );
        const beans::PropertyValue* pPropArray = aAttributeSet.getConstArray();
        for(i=0; i<nLength; ++i)
        {
            try
            {
                aPropSet.setPropertyValue(pPropArray->Name, pPropArray->Value);
            }
            catch( const uno::Exception& )
            {
                DBG_ERROR("AccessibleEditableTextPara::setAttributes exception in setPropertyValue");
            }

            ++pPropArray;
        }

        rCacheTF.QuickFormatDoc();
        GetEditSource().UpdateData();

        return sal_True;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setText( const ::rtl::OUString& sText ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return replaceText(0, getCharacterCount(), sText);
    }

    // XServiceInfo
    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getImplementationName (void) throw (uno::RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleEditableTextPara"));
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::supportsService (const ::rtl::OUString& sServiceName) throw (uno::RuntimeException)
    {
        //  Iterate over all supported service names and return true if on of them
        //  matches the given name.
        uno::Sequence< ::rtl::OUString> aSupportedServices (
            getSupportedServiceNames ());
        for (int i=0; i<aSupportedServices.getLength(); i++)
            if (sServiceName == aSupportedServices[i])
                return sal_True;
        return sal_False;
    }

    uno::Sequence< ::rtl::OUString> SAL_CALL AccessibleEditableTextPara::getSupportedServiceNames (void) throw (uno::RuntimeException)
    {
        const ::rtl::OUString sServiceName (RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
        return uno::Sequence< ::rtl::OUString > (&sServiceName, 1);
    }

    // XServiceName
    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getServiceName (void) throw (uno::RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.accessibility.AccessibleContext"));
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------
