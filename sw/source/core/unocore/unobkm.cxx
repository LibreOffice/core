/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unobkm.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <bookmrk.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#include <undobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::rtl::OUString;

/******************************************************************
 * SwXBookmark
 ******************************************************************/
TYPEINIT1(SwXBookmark, SwClient)
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXBookmark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXBookmark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/* -----------------10.12.98 10:16-------------------
 *
 * --------------------------------------------------*/
SwXBookmark::SwXBookmark(SwBookmark* pBkm, SwDoc* pDc) :
        aLstnrCntnr( (text::XTextContent*)this ),
        pDoc(pDc),
        bIsDescriptor(0 == pBkm)
{
    if(pBkm)
        pBkm->Add(this);
}
/*-- 10.12.98 10:14:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXBookmark::~SwXBookmark()
{

}
/*-- 10.12.98 10:14:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::attachToRangeEx(const uno::Reference< text::XTextRange > & xTextRange, IDocumentBookmarkAccess::BookmarkType eMark)
                                        throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!bIsDescriptor)
        throw uno::RuntimeException();

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {

        pRange = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId() )));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    }

    SwDoc* pDc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ?
        (SwDoc*)pCursor->GetDoc() : 0;
    if(pDc)
    {
        pDoc = pDc;
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        UnoActionContext aCont(pDoc);
        SwBookmark* pBkm = 0;
        {
            if(!m_aName.Len())
                 m_aName =  C2S("Bookmark");
            // --> OD 2007-10-23 #i81002#
            // determine bookmark type due to its proposed name
            if( eMark == IDocumentBookmarkAccess::BOOKMARK &&
                pDoc->isCrossRefBookmarkName( m_aName ) )
                eMark = IDocumentBookmarkAccess::CROSSREF_BOOKMARK;
            // <--
            if( USHRT_MAX != pDoc->findBookmark(m_aName) )
                pDoc->makeUniqueBookmarkName( m_aName );
            KeyCode aCode;
            pBkm = pDoc->makeBookmark( aPam, aCode, m_aName, aEmptyStr, eMark);
            // --> OD 2007-10-23 #i81002#
            // Check, if bookmark has been created.
            // E.g., the creation of a cross-reference bookmark is suppress,
            // if the PaM isn't a valid one for cross-reference bookmarks.
            if ( pBkm )
            {
                pBkm->Add(this);
            }
            else
            {
                ASSERT( false,
                        "<SwXBookmark::attachToRange(..)> - could not create <SwBookmark> instance." );
            }
            // <--
            bIsDescriptor = sal_False;
        }
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXBookmark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    attachToRangeEx(xTextRange, IDocumentBookmarkAccess::BOOKMARK);
}

SwXFieldmark::SwXFieldmark(bool _isReplacementObject, SwBookmark* pBkm, SwDoc* pDc)
    : SwXFieldmark_BASE(pBkm, pDc)
    , isReplacementObject(_isReplacementObject)
{ }


void SwXFieldmark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    attachToRangeEx(xTextRange, (isReplacementObject?IDocumentBookmarkAccess::FORM_FIELDMARK_NO_TEXT:IDocumentBookmarkAccess::FORM_FIELDMARK_TEXT));
}

::rtl::OUString SwXFieldmark::getDescription(void) throw( ::com::sun::star::uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    OUString sRet;
    if(pBkm)
        sRet = pBkm->GetFFHelpText();
    /* //@TODO implement...
    else if(bIsDescriptor)
        sRet = m_aName;
        */
    else
        throw uno::RuntimeException();
    return sRet;
}

::sal_Int16 SAL_CALL SwXFieldmark::getType(  ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    ::sal_Int16 sRet;
    if(pBkm)
        sRet = static_cast<sal_Int16>(pBkm->GetFieldType());
    /* //@TODO implement...
    else if(bIsDescriptor)
        sRet = m_aName;
        */
    else
        throw uno::RuntimeException();
    return sRet;
}

::sal_Int16 SAL_CALL SwXFieldmark::getRes(  ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    ::sal_Int16 sRet;
    if(pBkm)
        sRet = static_cast<sal_Int16>(pBkm->GetFFRes());
    /* //@TODO implement...
    else if(bIsDescriptor)
        sRet = m_aName;
        */
    else
        throw uno::RuntimeException();
    return sRet;
}


void SAL_CALL SwXFieldmark::setType( ::sal_Int16 fieldType ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    if (pBkm)
        pBkm->SetFieldType(fieldType);
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXFieldmark::setRes( ::sal_Int16 res ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    if (pBkm)
        pBkm->SetFFRes(res);
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXFieldmark::setDescription( const ::rtl::OUString& description ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFieldBookmark* pBkm = (SwFieldBookmark*)GetBookmark();
    if (pBkm)
        pBkm->SetFFHelpText(description);
    else
        throw uno::RuntimeException();
}



/*

::com::sun::star::uno::Any SAL_CALL SwXFieldmark::queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
{
        return SwXBookmark::queryInterface(rType);
}
*/


/* -----------------18.02.99 13:31-------------------
 *
 * --------------------------------------------------*/
void SwXBookmark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}
/*-- 10.12.98 10:14:39---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXBookmark::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  aRet;
    SwBookmark* pBkm = GetBookmark();

    if(pBkm)
    {
        const SwPosition& rPos = pBkm->GetBookmarkPos();
        const SwPosition* pMarkPos = pBkm->GetOtherBookmarkPos();

        aRet = SwXTextRange::CreateTextRangeFromPosition(pDoc, rPos, pMarkPos);
    }
    else
        throw uno::RuntimeException();
    return aRet;


}
/*-- 10.12.98 10:14:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
        GetDoc()->deleteBookmark(getName());
    else
        throw uno::RuntimeException();
}
/*-- 10.12.98 10:14:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 10.12.98 10:14:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
    throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 10.12.98 10:14:41---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXBookmark::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwBookmark* pBkm = GetBookmark();
    OUString sRet;
    if(pBkm)
        sRet = pBkm->GetName();
    else if(bIsDescriptor)
        sRet = m_aName;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 10.12.98 10:14:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::setName(const OUString& rName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwBookmark* pBkm = GetBookmark();
    String sBkName(rName);
    String sOldName = getName();
    if(sOldName != sBkName && pBkm && USHRT_MAX == pDoc->findBookmark(sBkName))
    {
        KeyCode aCode;
        String sShortName;
        SwPaM aPam(pBkm->GetBookmarkPos());
        if(pBkm->GetOtherBookmarkPos())
        {
            aPam.SetMark();
            *aPam.GetMark() = *pBkm->GetOtherBookmarkPos();
        }

        SwRewriter aRewriter;

        {
            String aStr(SW_RES(STR_START_QUOTE));

            aStr += sOldName;
            aStr += String(SW_RES(STR_END_QUOTE));

            aRewriter.AddRule(UNDO_ARG1, aStr);
        }

        aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));

        {
            String aStr(SW_RES(STR_START_QUOTE));

            aStr += String(rName);
            aStr += String(SW_RES(STR_END_QUOTE));
            aRewriter.AddRule(UNDO_ARG3, aStr);
        }

        pDoc->StartUndo(UNDO_BOOKMARK_RENAME, &aRewriter);

        // --> OD 2007-10-23 #i81002#
        SwBookmark* pMark = pDoc->makeBookmark( aPam, aCode,
                                                sBkName, sShortName,
                                                pBkm->GetType() );
        // <--
        pMark->Add(this);
        GetDoc()->deleteBookmark( sOldName );

        pDoc->EndUndo(UNDO_BOOKMARK_RENAME, NULL);
    }
    else if(bIsDescriptor)
        m_aName = sBkName;
    else
        throw uno::RuntimeException();
}

/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
OUString SwXBookmark::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXBookmark");
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
sal_Bool SwXBookmark::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.Bookmark") ||
                !rServiceName.compareToAscii("com.sun.star.document.LinkTarget") ||
                    !rServiceName.compareToAscii("com.sun.star.text.TextContent");
;
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
uno::Sequence< OUString > SwXBookmark::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.Bookmark");
    pArr[1] = C2U("com.sun.star.document.LinkTarget");
    pArr[2] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 10.12.98 10:14:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXBookmark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetBookmark())
    {
        pDoc = 0;
        aLstnrCntnr.Disposing();
    }
}
/*-- 30.03.99 16:02:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXBookmark::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef;
    if(!aRef.is())
    {
        const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_BOOKMARK);
        uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(pMap);
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        aRef = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return aRef;
}
/*-- 30.03.99 16:02:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::setPropertyValue(const OUString& PropertyName, const uno::Any& /*aValue*/)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    throw IllegalArgumentException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + PropertyName, static_cast < cppu::OWeakObject * > ( this ), 0 );
    //hier gibt es nichts zu setzen
}
/*-- 30.03.99 16:02:59---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXBookmark::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
            aRet <<= getName();
    }
    return aRet;
}
/*-- 30.03.99 16:02:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 30.03.99 16:02:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
            throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 30.03.99 16:03:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
            throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 30.03.99 16:03:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXBookmark::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
