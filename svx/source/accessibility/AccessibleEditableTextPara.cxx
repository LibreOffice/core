/*************************************************************************
 *
 *  $RCSfile: AccessibleEditableTextPara.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:53:15 $
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

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
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

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#include "accessibility.hrc"
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;


//------------------------------------------------------------------------
//
// AccessibleEditableTextPara implementation
//
//------------------------------------------------------------------------

namespace accessibility
{
    DBG_NAME( AccessibleEditableTextPara )

    AccessibleEditableTextPara::AccessibleEditableTextPara( const uno::Reference< XAccessible >& rParent ) :
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
        DBG_CTOR( AccessibleEditableTextPara, NULL );
        OSL_TRACE( "AccessibleEditableTextPara received ID: %d\n", mnNotifierClientId );
#endif

        try
        {
            // Create the state set.
            ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
            mxStateSet = pStateSet;

            // these are always on
            pStateSet->AddState( AccessibleStateType::MULTI_LINE );
            pStateSet->AddState( AccessibleStateType::FOCUSABLE );
            pStateSet->AddState( AccessibleStateType::VISIBLE );
            pStateSet->AddState( AccessibleStateType::SHOWING );
            pStateSet->AddState( AccessibleStateType::ENABLED );
        }
        catch( const uno::Exception& ) {}
    }

    AccessibleEditableTextPara::~AccessibleEditableTextPara()
    {
        DBG_DTOR( AccessibleEditableTextPara, NULL );

        // sign off from event notifier
        if( getNotifierClientId() != -1 )
        {
            try
            {
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
#ifdef DBG_UTIL
                OSL_TRACE( "AccessibleEditableTextPara revoked ID: %d\n", mnNotifierClientId );
#endif
            }
            catch( const uno::Exception& ) {}
        }
    }

    ::rtl::OUString AccessibleEditableTextPara::implGetText()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextRange( 0, GetTextLen() );
    }

    ::com::sun::star::lang::Locale AccessibleEditableTextPara::implGetLocale()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        lang::Locale        aLocale;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        // return locale of first character in the paragraph
        return SvxLanguageToLocale(aLocale, GetTextForwarder().GetLanguage( static_cast< USHORT >( GetParagraphIndex() ), 0 ));
    }

    void AccessibleEditableTextPara::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        USHORT nStart, nEnd;

        if( GetSelection( nStart, nEnd ) )
        {
            nStartIndex = nStart;
            nEndIndex = nEnd;
        }
        else
        {
            // #102234# No exception, just set to 'invalid'
            nStartIndex = -1;
            nEndIndex = -1;
        }
    }

    void AccessibleEditableTextPara::implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        rBoundary.startPos = 0;
        rBoundary.endPos = GetTextLen();
    }

    void AccessibleEditableTextPara::implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxTextForwarder&   rCacheTF = GetTextForwarder();
        sal_Int32           nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextAtIndex: paragraph index value overflow");

        sal_Int32 nTextLen = rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) );

        CheckIndex(nIndex);

        USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
        sal_Int32 nCurIndex;
        for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
        {
            nCurIndex += rCacheTF.GetLineLen( static_cast< USHORT >( nParaIndex ), nLine);

            if( nCurIndex > nIndex )
            {
                rBoundary.startPos = nCurIndex - rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine);
                rBoundary.endPos = nCurIndex;
                break;
            }
        }

        rBoundary.startPos = nTextLen;
        rBoundary.endPos = nTextLen;
    }

    int AccessibleEditableTextPara::getNotifierClientId() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnNotifierClientId;
    }

    void AccessibleEditableTextPara::SetIndexInParent( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        mnIndexInParent = nIndex;
    }

    sal_Int32 AccessibleEditableTextPara::GetIndexInParent() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnIndexInParent;
    }

    void AccessibleEditableTextPara::SetParagraphIndex( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        sal_Int32 nOldIndex = mnParagraphIndex;

        mnParagraphIndex = nIndex;

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetParagraphIndex(mnParagraphIndex);

        try
        {
            if( nOldIndex != nIndex )
            {
                uno::Any aOldDesc;
                uno::Any aOldName;

                try
                {
                    aOldDesc <<= getAccessibleDescription();
                    aOldName <<= getAccessibleName();
                }
                catch( const uno::Exception& ) {} // optional behaviour
                // index and therefore description changed
                FireEvent( AccessibleEventId::DESCRIPTION_CHANGED, uno::makeAny( getAccessibleDescription() ), aOldDesc );
                FireEvent( AccessibleEventId::NAME_CHANGED, uno::makeAny( getAccessibleName() ), aOldName );
            }
        }
        catch( const uno::Exception& ) {} // optional behaviour
    }

    sal_Int32 AccessibleEditableTextPara::GetParagraphIndex() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnParagraphIndex;
    }

    void AccessibleEditableTextPara::Dispose()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        int nClientId( getNotifierClientId() );

        // #108212# drop all references before notifying dispose
        mxParent = NULL;
        mnNotifierClientId = -1;
        mpEditSource = NULL;

        // notify listeners
        if( nClientId != -1 )
        {
            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                // #106234# Delegate to EventNotifier
                ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, xThis );
#ifdef DBG_UTIL
                OSL_TRACE( "Disposed ID: %d\n", nClientId );
#endif
            }
            catch( const uno::Exception& ) {}
        }
    }

    void AccessibleEditableTextPara::SetEditSource( SvxEditSourceAdapter* pEditSource )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

            Dispose();
        }

        // #108900# Init last text content
        try
        {
            TextChanged();
        }
        catch( const uno::RuntimeException& ) {}
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // check overflow
        DBG_ASSERT(nStartEEIndex >= 0 && nStartEEIndex <= USHRT_MAX &&
                   nEndEEIndex >= 0 && nEndEEIndex <= USHRT_MAX &&
                   GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::MakeSelection: index value overflow");

        USHORT nParaIndex = static_cast< USHORT >( GetParagraphIndex() );
        return ESelection( nParaIndex, static_cast< USHORT >( nStartEEIndex ),
                           nParaIndex, static_cast< USHORT >( nEndEEIndex ) );
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nEEIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return MakeSelection( nEEIndex, nEEIndex+1 );
    }

    ESelection AccessibleEditableTextPara::MakeCursor( sal_Int32 nEEIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return MakeSelection( nEEIndex, nEEIndex );
    }

    void AccessibleEditableTextPara::CheckIndex( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( nIndex < 0 || nIndex >= getCharacterCount() )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleEditableTextPara: character index out of bounds")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckPosition( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( nIndex < 0 || nIndex > getCharacterCount() )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleEditableTextPara: character position out of bounds")),
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        CheckPosition( nStart );
        CheckPosition( nEnd );
    }

    sal_Bool AccessibleEditableTextPara::GetSelection( USHORT& nStartPos, USHORT& nEndPos ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    String AccessibleEditableTextPara::GetText( sal_Int32 nIndex ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextForwarder().GetText( MakeSelection(nIndex) );
    }

    String AccessibleEditableTextPara::GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextForwarder().GetText( MakeSelection(nStartIndex, nEndIndex) );
    }

    USHORT AccessibleEditableTextPara::GetTextLen() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextForwarder().GetTextLen( static_cast< USHORT >( GetParagraphIndex() ) );
    }

    sal_Bool AccessibleEditableTextPara::IsVisible() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mpEditSource ? sal_True : sal_False ;
    }

    uno::Reference< XAccessibleText > AccessibleEditableTextPara::GetParaInterface( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    SvxEditSourceAdapter& AccessibleEditableTextPara::GetEditSource() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No edit source, object is defunct")),
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
    }

    SvxAccessibleTextAdapter& AccessibleEditableTextPara::GetTextForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    SvxViewForwarder& AccessibleEditableTextPara::GetViewForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    SvxAccessibleTextEditViewAdapter& AccessibleEditableTextPara::GetEditViewForwarder( sal_Bool bCreate ) const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::HaveChildren: paragraph index value overflow");

        return GetTextForwarder().HaveImageBullet( static_cast< USHORT >(GetParagraphIndex()) );
    }

    sal_Bool AccessibleEditableTextPara::IsActive() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return maEEOffset;
    }

    void AccessibleEditableTextPara::SetEEOffset( const Point& rOffset )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEEOffset(rOffset);

        maEEOffset = rOffset;
    }

    void AccessibleEditableTextPara::FireEvent(const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleEditableTextPara* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        // #102261# Call global queue for focus events
        if( nEventId == AccessibleStateType::FOCUSED )
            vcl::unohelper::NotifyAccessibleStateEventGlobally( aEvent );

        // #106234# Delegate to EventNotifier
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                             aEvent );
    }

    void AccessibleEditableTextPara::GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        FireEvent( nEventId, rNewValue );
    }

    void AccessibleEditableTextPara::LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        FireEvent( nEventId, uno::Any(), rOldValue );
    }

    bool AccessibleEditableTextPara::HasState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL )
            return pStateSet->contains(nStateId) ? true : false;

        return false;
    }

    void AccessibleEditableTextPara::SetState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            !pStateSet->contains(nStateId) )
        {
            pStateSet->AddState( nStateId );
            GotPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::STATE_CHANGED );
        }
    }

    void AccessibleEditableTextPara::UnSetState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            pStateSet->contains(nStateId) )
        {
            pStateSet->RemoveState( nStateId );
            LostPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::STATE_CHANGED );
        }
    }

    void AccessibleEditableTextPara::TextChanged()
    {
        ::rtl::OUString aCurrentString( OCommonAccessibleText::getText() );
        uno::Any aDeleted;
        uno::Any aInserted;
        if( OCommonAccessibleText::implInitTextChangedEvent( maLastTextString, aCurrentString,
                                                             aDeleted, aInserted) )
        {
            FireEvent( AccessibleEventId::TEXT_CHANGED, aInserted, aDeleted );
            maLastTextString = aCurrentString;
        }
    }

    sal_Bool AccessibleEditableTextPara::GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        DBG_ASSERT(nIndex >= 0 && nIndex <= USHRT_MAX,
                   "AccessibleEditableTextPara::GetAttributeRun: index value overflow");

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        return GetTextForwarder().GetAttributeRun( nStartIndex, nEndIndex, static_cast< USHORT >(GetParagraphIndex()), static_cast< USHORT >(nIndex) );
    }

    uno::Any SAL_CALL AccessibleEditableTextPara::queryInterface (const uno::Type & rType) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // We implement the XAccessibleContext interface in the same object
        return uno::Reference< XAccessibleContext > ( this );
    }

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return HaveChildren() ? 1 : 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

#ifdef DBG_UTIL
        if( !mxParent.is() )
            DBG_TRACE( "AccessibleEditableTextPara::getAccessibleParent: no frontend set, did somebody forgot to call AccessibleTextHelper::SetEventSource()?");
#endif

        return mxParent;
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleIndexInParent() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL AccessibleEditableTextPara::getAccessibleRole() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return AccessibleRole::PARAGRAPH;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getAccessibleDescription() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // append first 40 characters from text, or first line, if shorter
        // (writer takes first sentence here, but that's not supported
        // from EditEngine)
        // throws if defunc
        ::rtl::OUString aLine;

        if( getCharacterCount() )
            aLine = getTextAtIndex(0, AccessibleTextType::LINE);

        // Get the string from the resource for the specified id.
        String sStr = ::rtl::OUString( SVX_RESSTR (RID_SVXSTR_A11Y_PARAGRAPH_DESCRIPTION ) );
        String sParaIndex = ::rtl::OUString::valueOf( GetParagraphIndex() );
        sStr.SearchAndReplace( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "$(ARG)" )),
                               sParaIndex );

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

        return ::rtl::OUString( sStr ) + aLine;
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getAccessibleName() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // throws if defunc
        sal_Int32 nPara( GetParagraphIndex() );

        // Get the string from the resource for the specified id.
        String sStr = ::rtl::OUString( SVX_RESSTR (RID_SVXSTR_A11Y_PARAGRAPH_NAME) );
        String sParaIndex = ::rtl::OUString::valueOf( nPara );
        sStr.SearchAndReplace( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "$(ARG)" )),
                               sParaIndex );

        return ::rtl::OUString( sStr );
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleEditableTextPara::getAccessibleRelationSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // no relations, therefore empty
        return uno::Reference< XAccessibleRelationSet >();
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleEditableTextPara::getAccessibleStateSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // Create a copy of the state set and return it.
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();

        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleEditableTextPara::getLocale() throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return implGetLocale();
    }

    void SAL_CALL AccessibleEditableTextPara::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleEditableTextPara::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
    }

    // XAccessibleComponent
    sal_Bool SAL_CALL AccessibleEditableTextPara::containsPoint( const awt::Point& aTmpPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::contains: index value overflow");

        awt::Rectangle aTmpRect = getBounds();
        Rectangle aRect( Point(aTmpRect.X, aTmpRect.Y), Size(aTmpRect.Width, aTmpRect.Height) );
        Point aPoint( aTmpPoint.X, aTmpPoint.Y );

        return aRect.IsInside( aPoint );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( HaveChildren() )
        {
            // #103862# No longer need to make given position relative
            Point aPoint( _aPoint.X, _aPoint.Y );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocationOnScreen(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    void SAL_CALL AccessibleEditableTextPara::grabFocus(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // set cursor to this paragraph
        setSelection(0,0);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #104444# Added to XAccessibleComponent interface
        svtools::ColorConfig aColorConfig;
        UINT32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        return static_cast<sal_Int32>(nColor);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #104444# Added to XAccessibleComponent interface
        Color aColor( Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor() );

        // the background is transparent
        aColor.SetTransparency( 0xFF);

        return static_cast<sal_Int32>( aColor.GetColor() );
    }

    // XAccessibleText
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCaretPosition() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    sal_Bool SAL_CALL AccessibleEditableTextPara::setCaretPosition( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return setSelection(nIndex, nIndex);
    }

    sal_Unicode SAL_CALL AccessibleEditableTextPara::getCharacter( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacter: index value overflow");

        return OCommonAccessibleText::getCharacter( nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterBounds: index value overflow");

        // #108900# Have position semantics now for nIndex, as
        // one-past-the-end values are legal, too.
        CheckPosition( nIndex );

        SvxTextForwarder& rCacheTF = GetTextForwarder();
#ifdef DBG_UTIL
        Rectangle aTestRect = rCacheTF.GetCharBounds( static_cast< USHORT >( GetParagraphIndex() ),
                                                      rCacheTF.GetTextLen(GetParagraphIndex()) );
#endif
        Rectangle aRect = rCacheTF.GetCharBounds( static_cast< USHORT >( GetParagraphIndex() ), static_cast< USHORT >( nIndex ) );

        // offset from parent (paragraph)
        Rectangle aParaRect = rCacheTF.GetParaBounds( static_cast< USHORT >( GetParagraphIndex() ) );
        aRect.Move( -aParaRect.Left(), -aParaRect.Top() );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterCount: index value overflow");

        return OCommonAccessibleText::getCharacterCount();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        USHORT nPara, nIndex;

        // offset from surrounding cell/shape
        Point aOffset( GetEEOffset() );
        Point aPoint( rPoint.X - aOffset.X(), rPoint.Y - aOffset.Y() );

        // convert to logical coordinates
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

        // re-offset to parent (paragraph)
        Rectangle aParaRect = rCacheTF.GetParaBounds( static_cast< USHORT >( GetParagraphIndex() ) );
        aLogPoint.Move( aParaRect.Left(), aParaRect.Top() );

        if( rCacheTF.GetIndexAtPoint( aLogPoint, nPara, nIndex ) &&
            GetParagraphIndex() == nPara )
        {
            // #102259# Double-check if we're _really_ on the given character
            try
            {
                awt::Rectangle aRect1( getCharacterBounds(nIndex) );
                Rectangle aRect2( aRect1.X, aRect1.Y,
                                  aRect1.Width + aRect1.X, aRect1.Height + aRect1.Y );
                if( aRect2.IsInside( Point( rPoint.X, rPoint.Y ) ) )
                    return nIndex;
                else
                    return -1;
            }
            catch( const lang::IndexOutOfBoundsException& )
            {
                // #103927# Don't throw for invalid nIndex values
                return -1;
            }
        }
        else
        {
            // not within our paragraph
            return -1;
        }
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getSelectedText() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectedText: index value overflow");

        if( !HaveEditView() )
            return ::rtl::OUString();

        return OCommonAccessibleText::getSelectedText();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionStart() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionStart: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionStart();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionEnd() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionEnd: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionEnd();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::setSelection: paragraph index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            return rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getText() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getText: paragraph index value overflow");

        return OCommonAccessibleText::getText();
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        return OCommonAccessibleText::getTextRange(nStartIndex, nEndIndex);
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                USHORT nStartIndex, nEndIndex;

                if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                    return GetTextRange(nStartIndex, nEndIndex);
                else
                    return ::rtl::OUString();
            }

            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();
                sal_Int32 nTextLen = rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) );

                CheckPosition(nIndex);

                USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
                sal_Int32 nCurIndex;
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    nCurIndex += rCacheTF.GetLineLen( static_cast< USHORT >( nParaIndex ), nLine);

                    if( nCurIndex > nIndex )
                        return GetTextRange( nCurIndex - rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine), nCurIndex );
                }
                return ::rtl::OUString();
            }

            default:
                return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
        } /* end of switch( aTextType ) */
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                USHORT nStartIndex, nEndIndex;

                if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                {
                    // already at the left border?
                    if( nStartIndex != 0 )
                    {
                        if( GetAttributeRun(nStartIndex, nEndIndex, nStartIndex-1) )
                            return GetTextRange(nStartIndex, nEndIndex);
                    }
                }
                return ::rtl::OUString();
            }

            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();
                sal_Int32 nTextLen = rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) );

                CheckPosition(nIndex);

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
                        return GetTextRange( nLastIndex - nCurLineLen, static_cast< USHORT >( nLastIndex ) );
                    }
                }
                return ::rtl::OUString();
            }

            default:
                return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
        } /* end of switch( aTextType ) */
    }

    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                USHORT nStartIndex, nEndIndex;

                if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                {
                    // already at the right border?
                    if( nEndIndex < GetTextLen() )
                    {
                        if( GetAttributeRun(nStartIndex, nEndIndex, nEndIndex) )
                            return GetTextRange(nStartIndex, nEndIndex);
                    }
                }
                return ::rtl::OUString();
            }

            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();
                sal_Int32 nTextLen = rCacheTF.GetTextLen( static_cast< USHORT >( nParaIndex ) );

                CheckPosition(nIndex);

                USHORT nLine, nLineCount=rCacheTF.GetLineCount( static_cast< USHORT >( nParaIndex ) );
                sal_Int32 nCurIndex;
                // get the line after the line the index points into
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    nCurIndex += rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine);

                    if( nCurIndex > nIndex &&
                        nLine < nLineCount-1 )
                    {
                        return GetTextRange( nCurIndex, nCurIndex + rCacheTF.GetLineLen(static_cast< USHORT >( nParaIndex ), nLine+1) );
                    }
                }
                return ::rtl::OUString();
            }

            default:
                return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
        } /* end of switch( aTextType ) */
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
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
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    // XAccessibleEditableText
    sal_Bool SAL_CALL AccessibleEditableTextPara::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
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
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::pasteText( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::pasteText: index value overflow");

            CheckPosition(nIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
                return sal_False; // non-editable area selected

            // #104400# set empty selection (=> cursor) to given index
            rCacheVF.SetSelection( MakeCursor(nIndex) );

            return rCacheVF.Paste();
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::deleteText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            sal_Bool bRet = rCacheTF.Delete( MakeSelection(nStartIndex, nEndIndex) );

            GetEditSource().UpdateData();

            return bRet;
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::insertText: index value overflow");

            CheckPosition(nIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
                return sal_False; // non-editable area selected

            // #104400# insert given text at empty selection (=> cursor)
            sal_Bool bRet = rCacheTF.InsertText( sText, MakeCursor(nIndex) );

            rCacheTF.QuickFormatDoc();
            GetEditSource().UpdateData();

            return bRet;
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::replaceText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            // insert given text into given range => replace
            sal_Bool bRet = rCacheTF.InsertText( sReplacement, MakeSelection(nStartIndex, nEndIndex) );

            rCacheTF.QuickFormatDoc();
            GetEditSource().UpdateData();

            return bRet;
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const uno::Sequence< beans::PropertyValue >& aAttributeSet ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
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
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setText( const ::rtl::OUString& sText ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return replaceText(0, getCharacterCount(), sText);
    }

    // XServiceInfo
    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getImplementationName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleEditableTextPara"));
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::supportsService (const ::rtl::OUString& sServiceName) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        const ::rtl::OUString sServiceName( getServiceName() );
        return uno::Sequence< ::rtl::OUString > (&sServiceName, 1);
    }

    // XServiceName
    ::rtl::OUString SAL_CALL AccessibleEditableTextPara::getServiceName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #105185# Using correct service now
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleParagraphView"));
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------
