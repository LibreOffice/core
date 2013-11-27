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
#include "precompiled_sw.hxx"


#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <frmfmt.hxx>
#include <ndnotxt.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
// --> OD 2009-07-14 #i73249#
#include <hints.hxx>
// <--
#include "accnotextframe.hxx"
//IAccessibility2 Implementation 2009-----
#include <fmturl.hxx>
#include <accnotexthyperlink.hxx>
#include <svtools/imap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <doc.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using utl::AccessibleRelationSetHelper;
//-----IAccessibility2 Implementation 2009

const SwNoTxtNode *SwAccessibleNoTextFrame::GetNoTxtNode() const
{
    const SwNoTxtNode *pNd  = 0;
    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm *>( GetFrm() );
    if( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
    {
        const SwCntntFrm *pCntFrm =
            static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
        //IAccessibility2 Implementation 2009-----
        const SwCntntNode* pSwCntntNode = pCntFrm->GetNode();
        if(pSwCntntNode != NULL)
        {
            pNd = pSwCntntNode->GetNoTxtNode();
        }
        //-----IAccessibility2 Implementation 2009
    }

    return pNd;
}

SwAccessibleNoTextFrame::SwAccessibleNoTextFrame(
        SwAccessibleMap* pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleFrameBase( pInitMap, nInitRole, pFlyFrm ),
    aDepend( this, const_cast < SwNoTxtNode * >( GetNoTxtNode() ) ),
    msTitle(),
    msDesc()
{
    const SwNoTxtNode* pNd = GetNoTxtNode();
    // --> OD 2009-07-14 #i73249#
    // consider new attributes Title and Description
    if( pNd )
    {
        msTitle = pNd->GetTitle();

        msDesc = pNd->GetDescription();
        if ( msDesc.getLength() == 0 &&
             msTitle != GetName() )
        {
            msDesc = msTitle;
        }
    }
    // <--
}

SwAccessibleNoTextFrame::~SwAccessibleNoTextFrame()
{
}

void SwAccessibleNoTextFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    // --> OD 2009-07-14 #i73249#
    // suppress handling of RES_NAME_CHANGED in case that attribute Title is
    // used as the accessible name.
    if ( nWhich != RES_NAME_CHANGED ||
         msTitle.getLength() == 0 )
    {
        SwAccessibleFrameBase::Modify( pOld, pNew );
    }

    const SwNoTxtNode *pNd = GetNoTxtNode();
    ASSERT( pNd == aDepend.GetRegisteredIn(), "invalid frame" );
    switch( nWhich )
    {
        // --> OD 2009-07-14 #i73249#
        case RES_TITLE_CHANGED:
        {
            const String& sOldTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pOld)->GetString() );
            const String& sNewTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pNew)->GetString() );
            if ( sOldTitle == sNewTitle )
            {
                break;
            }
            msTitle = sNewTitle;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.OldValue <<= OUString( sOldTitle );
            aEvent.NewValue <<= msTitle;
            FireAccessibleEvent( aEvent );

            if ( pNd->GetDescription().Len() != 0 )
            {
                break;
            }
        }
        // intentional no break here
        case RES_DESCRIPTION_CHANGED:
        {
            if ( pNd && GetFrm() )
            {
                const OUString sOldDesc( msDesc );

                const String& rDesc = pNd->GetDescription();
                msDesc = rDesc;
                if ( msDesc.getLength() == 0 &&
                     msTitle != GetName() )
                {
                    msDesc = msTitle;
                }

                if ( msDesc != sOldDesc )
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
                    aEvent.OldValue <<= sOldDesc;
                    aEvent.NewValue <<= msDesc;
                    FireAccessibleEvent( aEvent );
                }
            }
        }
        break;
        // <--
        /*
    case RES_OBJECTDYING:
        if( aDepend.GetRegisteredIn() ==
                static_cast< SwModify *>( static_cast< SwPtrMsgPoolItem * >( pOld )->pObject ) )
            const_cast < SwModify *>( aDepend.GetRegisteredIn()->Remove( aDepend );
        break;

    case RES_FMT_CHG:
        if( static_cast< SwFmtChg * >(pNew)->pChangedFmt == GetRegisteredIn() &&
            static_cast< SwFmtChg * >(pOld)->pChangedFmt->IsFmtInDTOR() )
            GetRegisteredIn()->Remove( this );
        break;
    */
    }
}

void SwAccessibleNoTextFrame::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( aDepend.GetRegisteredIn() )
        const_cast < SwModify *>( aDepend.GetRegisteredIn() )->Remove( &aDepend );

    SwAccessibleFrameBase::Dispose( bRecursive );
}

// --> OD 2009-07-14 #i73249#
OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleName (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    if ( msTitle.getLength() != 0 )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}
// <--

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )
    //IAccessibility2 Implementation 2009-----
    // OUString longDesc;
    // const SwFlyFrmFmt* pFlyFmt = GetShell()->GetDoc()->FindFlyByName( GetName(), 0);
    // longDesc = OUString( pFlyFmt->GetLongDescription() );
    // return  longDesc; MT: Do not return longDesc, which still is empty - why was the line above commented out?
    //-----IAccessibility2 Implementation 2009

    return msDesc;
}



//
// XInterface
//

uno::Any SAL_CALL SwAccessibleNoTextFrame::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    if( aType ==
        ::getCppuType( static_cast<uno::Reference<XAccessibleImage>*>( NULL ) ) )
    {
        uno::Reference<XAccessibleImage> xImage = this;
        uno::Any aAny;
        aAny <<= xImage;
        return aAny;
    }
    //IAccessibility2 Implementation 2009-----
    else if ( aType == ::getCppuType((uno::Reference<XAccessibleHypertext> *)0) )
    {
        uno::Reference<XAccessibleHypertext> aAccHypertext = this;
        uno::Any aAny;
        aAny <<= aAccHypertext;
        return aAny;
    }
    //-----IAccessibility2 Implementation 2009
    else
        return SwAccessibleContext::queryInterface( aType );
}


//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleNoTextFrame::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleFrameBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleImage > * >( 0 ) );

    return aTypes;
}


//
// XAccessibleImage
//

// implementation of the XAccessibleImage methods is a no-brainer, as
// all releveant information is already accessible through other
// methods. So we just delegate to those.

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageDescription()
    throw ( uno::RuntimeException )
{
    return getAccessibleDescription();
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageHeight(  )
    throw ( uno::RuntimeException )
{
    return getSize().Height;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageWidth(  )
    throw ( uno::RuntimeException )
{
    return getSize().Width;
}
//IAccessibility2 Implementation 2009-----
//=====  XAccesibleText  ==================================================
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setCaretPosition( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return 0;}
sal_Unicode SAL_CALL SwAccessibleNoTextFrame::getCharacter( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return 0;}
::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SwAccessibleNoTextFrame::getCharacterAttributes( sal_Int32 , const ::com::sun::star::uno::Sequence< ::rtl::OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aValues(0);
    return aValues;
}
::com::sun::star::awt::Rectangle SAL_CALL SwAccessibleNoTextFrame::getCharacterBounds( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    return com::sun::star::awt::Rectangle(0, 0, 0, 0 );
}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getIndexAtPoint( const ::com::sun::star::awt::Point& ) throw (::com::sun::star::uno::RuntimeException){return 0;}
::rtl::OUString SAL_CALL SwAccessibleNoTextFrame::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException){return OUString();}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setSelection( sal_Int32 , sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return sal_True;}
::rtl::OUString SAL_CALL SwAccessibleNoTextFrame::getText(  ) throw (::com::sun::star::uno::RuntimeException){return OUString();}
::rtl::OUString SAL_CALL SwAccessibleNoTextFrame::getTextRange( sal_Int32 , sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return OUString();}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextAtIndex( sal_Int32 , sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBeforeIndex( sal_Int32, sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBehindIndex( sal_Int32 , sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}

sal_Bool SAL_CALL SwAccessibleNoTextFrame::copyText( sal_Int32, sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return sal_True;}


//
//  XAccessibleHyperText
//
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkCount()
throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    SwFmtURL aURL( ((SwLayoutFrm*)GetFrm())->GetFmt()->GetURL() );

    if(aURL.GetMap() || aURL.GetURL().Len())
        nCount = 1;

    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleNoTextFrame::getHyperLink( sal_Int32 nLinkIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    uno::Reference< XAccessibleHyperlink > xRet;

    SwFmtURL aURL( ((SwLayoutFrm*)GetFrm())->GetFmt()->GetURL() );

    if( nLinkIndex > 0 )
        throw lang::IndexOutOfBoundsException();

    if( aURL.GetMap() || aURL.GetURL().Len() )
    {
        if ( !alink.is() )
        {
            alink = new SwAccessibleNoTextHyperlink( this, GetFrm() );
        }

        return alink;
    }

    return NULL;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkIndex( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nRet = 0;

    return nRet;
}

AccessibleRelation SwAccessibleNoTextFrame::makeRelation( sal_Int16 nType, const SwFlyFrm* pFrm )
{
    uno::Sequence<uno::Reference<XInterface> > aSequence(1);
    aSequence[0] = GetMap()->GetContext( pFrm );
    return AccessibleRelation( nType, aSequence );
}


uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleNoTextFrame::getAccessibleRelationSet( )
    throw ( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get the caption frame, and insert label relations into helper

    AccessibleRelationSetHelper* pHelper = new AccessibleRelationSetHelper();

    SwFlyFrm* pFlyFrm = getFlyFrm();
    DBG_ASSERT( pFlyFrm != NULL, "fly frame expected" );

    SwFlyFrm* pCaptionFrm = NULL;
    const SwFrmFmt* pFrm = pFlyFrm ->GetFmt()->GetCaptionFmt();
    if (pFrm)
    {
        SwClientIter aIter (*(SwModify*)pFrm);
        pCaptionFrm = dynamic_cast< SwFlyFrm* >(aIter.SwClientIter_First());
    }
    if(pCaptionFrm!=NULL)
    {
        pHelper->AddRelation( makeRelation( AccessibleRelationType::DESCRIBED_BY, pCaptionFrm ) );
    }

    return pHelper;
}
//-----IAccessibility2 Implementation 2009
