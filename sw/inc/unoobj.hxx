/*************************************************************************
 *
 *  $RCSfile: unoobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-11-08 12:19:40 $
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
#ifndef _UNOOBJ_HXX
#define _UNOOBJ_HXX

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FLYENUM_HXX //autogen
#include <flyenum.hxx>
#endif
#ifndef _FLDBAS_HXX //autogen
#include <fldbas.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _TOXE_HXX
#include <toxe.hxx>
#endif
#ifndef _SORTOPT_HXX //autogen
#include <sortopt.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINSERTABLE_HPP_
#include <com/sun/star/document/XDocumentInsertable.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XWORDCURSOR_HPP_
#include <com/sun/star/text/XWordCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPARAGRAPHCURSOR_HPP_
#include <com/sun/star/text/XParagraphCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XSENTENCECURSOR_HPP_
#include <com/sun/star/text/XSentenceCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSORTABLE_HPP_
#include <com/sun/star/util/XSortable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGECOMPARE_HPP_
#include <com/sun/star/text/XTextRangeCompare.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XRELATIVETEXTCONTENTINSERT_HPP_
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XRELATIVETEXTCONTENTREMOVE_HPP_
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>   // helper for factories
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx> // helper for implementations
#endif
#ifndef _CPPUHELPER_IMPLBASE10_HXX_
#include <cppuhelper/implbase10.hxx>    // helper for implementations
#endif

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
#define C2S(cChar) UniString::CreateFromAscii(cChar)

class SwUnoCrsr;
class SwBookmark;
class SwFmtFtn;
class SwFmtFld;
class SwFmtRefMark;
class SwEvtLstnrArray;
class GetCurTxtFmtColl;
/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
enum CursorType
{
    CURSOR_INVALID,
    CURSOR_BODY,
    CURSOR_FRAME,
    CURSOR_TBLTEXT,
    CURSOR_FOOTNOTE,
    CURSOR_HEADER,
    CURSOR_FOOTER,
    CURSOR_ALL          // fuer Search&Replace
};

/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
#define PUNOPAM (_pStartCrsr)

#define FOREACHUNOPAM_START(pCrsr) \
    {\
        SwPaM *_pStartCrsr = pCrsr, *__pStartCrsr = _pStartCrsr; \
        do {

#define FOREACHUNOPAM_END() \
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr ); \
    }

/* -----------------26.06.98 16:18-------------------
 *
 * --------------------------------------------------*/
SwPageDesc* GetPageDescByName_Impl(SwDoc& rDoc, const String& rName);
::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  CreateTextRangeFromPosition(SwDoc* pDoc,
                        const SwPosition& rPos, const SwPosition* pMark);

::com::sun::star::uno::Sequence< sal_Int8 > CreateUnoTunnelId();

/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
class SwUnoInternalPaM : public SwPaM
{
    SwUnoInternalPaM( const SwUnoInternalPaM& );
public:
    SwUnoInternalPaM(SwDoc& rDoc);
    ~SwUnoInternalPaM();

    SwUnoInternalPaM&   operator=(const SwPaM& rPaM);
};

/*-----------------04.03.98 11:54-------------------
    Start/EndAction oder Start/EndAllAction
--------------------------------------------------*/
class UnoActionContext
{
        SwDoc*      pDoc;
    public:
        UnoActionContext(SwDoc* pDoc);
        ~UnoActionContext();

        void InvalidateDocument() {pDoc = 0;}
};
/* -----------------07.07.98 12:03-------------------
 *  Actions kurzfristig unterbrechen
 * --------------------------------------------------*/
class UnoActionRemoveContext
{
        SwDoc*      pDoc;
    public:
        UnoActionRemoveContext(SwDoc* pDoc);
        ~UnoActionRemoveContext();
};

/******************************************************************************
 *
 ******************************************************************************/
typedef com::sun::star::uno::Reference< com::sun::star::text::XTextRange > * XTextRangeRefPtr;
SV_DECL_PTRARR(XTextRangeArr, XTextRangeRefPtr, 4, 4);

void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew);

/* -----------------22.04.99 11:18-------------------
 *  Verwaltung der EventListener
 * --------------------------------------------------*/
typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > *  XEventListenerPtr;
SV_DECL_PTRARR(SwEvtLstnrArray, XEventListenerPtr, 4, 4);
class SwEventListenerContainer
{
    SwEvtLstnrArray*                            pListenerArr;
     ::com::sun::star::uno::XInterface*         pxParent;
    protected:
        SwEvtLstnrArray*                    GetListenerArray() {return pListenerArr;}
         ::com::sun::star::uno::XInterface* GetParent() {return     pxParent;}
    public:
        SwEventListenerContainer( ::com::sun::star::uno::XInterface* pxParent);
        ~SwEventListenerContainer();

        void    AddListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        BOOL    RemoveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        void    Disposing();
};
/* -----------------03.12.98 12:22-------------------
 *
 * --------------------------------------------------*/
class SwXTextRange;
class SwXTextCursor;
class SwXText : public ::com::sun::star::text::XText,
                ::com::sun::star::lang::XTypeProvider,
                ::com::sun::star::text::XTextRangeCompare,
                ::com::sun::star::text::XRelativeTextContentInsert,
                ::com::sun::star::text::XRelativeTextContentRemove

{
    SwDoc*              pDoc;
    BOOL                bObjectValid;
    CursorType          eCrsrType;
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor();

public:
                SwXText(SwDoc* pDc, CursorType eType) :
                        pDoc(pDc),bObjectValid(0 != pDc), eCrsrType(eType){}
    virtual     ~SwXText();

    const SwDoc*            GetDoc()const {return pDoc;}
    SwDoc*                  GetDoc()    {return pDoc;}

    // wenn ein SwXText attached wird, wird das Doc gesetzt
    void                    SetDoc(SwDoc* pDc)
                                {DBG_ASSERT(!pDoc || !pDc, "Doc schon gesetzt?")
                                    pDoc = pDc;
                                    bObjectValid = 0 != pDc;
                                }
    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    //XText
    virtual void SAL_CALL insertString(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, const rtl::OUString& aString, BOOL bAbsorb) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL insertControlCharacter(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, sal_Int16 nControlCharacter, BOOL bAbsorb) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL insertTextContent(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & xContent, BOOL bAbsorb) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeTextContent(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & xContent) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XTextRangeCompare
    sal_Int16 SAL_CALL compareRegionStarts( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL compareRegionEnds( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XRelativeTextContentInsert
    virtual void SAL_CALL insertTextContentBefore(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xNewContent, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xSuccessor) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertTextContentAfter(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xNewContent, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xPredecessor) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XRelativeTextContentRemove
    virtual void SAL_CALL removeTextContentBefore(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xSuccessor) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContentAfter(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent>& xPredecessor) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //
    INT16   ComparePositions(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange>& xPos1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange>& xPos2) throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);
    BOOL    CheckForOwnMember(const SwXTextRange* pRange1, const SwXTextCursor* pCursor1)throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    //
    void            Invalidate() {bObjectValid = sal_False;}
    BOOL            IsValid()const {return bObjectValid;}

    CursorType      GetTextType() {return eCrsrType;}
};

/* -----------------03.12.98 12:16-------------------
 *
 * --------------------------------------------------*/
class SwXTextCursor : public cppu::WeakImplHelper10
<
    ::com::sun::star::text::XSentenceCursor,
    ::com::sun::star::text::XWordCursor,
    ::com::sun::star::text::XParagraphCursor,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::document::XDocumentInsertable,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::util::XSortable,
    ::com::sun::star::container::XContentEnumerationAccess
>,
    public SwClient
{
    SwEventListenerContainer    aLstnrCntnr;
    SfxItemPropertySet          aPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    SwSortOptions*              pLastSortOptions;

    CursorType                  eType;

    void    DeleteAndInsert(const String& rText);

public:
    SwXTextCursor(::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, const SwPosition& rPos,
                    CursorType eSet, SwDoc* pDoc, const SwPosition* pMark = 0);
    SwXTextCursor(::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, SwUnoCrsr* pSourceCrsr, CursorType eSet = CURSOR_ALL);
    //invalid Cursor!
    SwXTextCursor(SwXText* pParent = 0);
    virtual ~SwXTextCursor();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTextCursor - neu
    virtual void SAL_CALL collapseToStart(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collapseToEnd(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL isCollapsed(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goLeft( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goRight( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoStart( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);

    //XWordCursor - neu
    virtual BOOL SAL_CALL isStartOfWord(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL isEndOfWord(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoNextWord( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoPreviousWord( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoEndOfWord( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoStartOfWord( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);

    //XSentenceCursor - neu
    virtual BOOL SAL_CALL isStartOfSentence(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL isEndOfSentence(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoNextSentence( BOOL Expand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoPreviousSentence( BOOL Expand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoStartOfSentence( BOOL Expand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL gotoEndOfSentence( BOOL Expand ) throw(::com::sun::star::uno::RuntimeException);

    //XLineCursor - alt
//  virtual BOOL goDown(sal_uInt16 nCount, BOOL Expand);
//    virtual BOOL goUp(sal_uInt16 nCount, BOOL Expand);
//    virtual BOOL isAtStartOfLine(void);
//    virtual BOOL isAtEndOfLine(void);
//    virtual void gotoEndOfLine(BOOL Expand);
//    virtual void gotoStartOfLine(BOOL Expand);

    //ParagraphCursor - neu
    virtual BOOL SAL_CALL isStartOfParagraph(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL isEndOfParagraph(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoStartOfParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoEndOfParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoNextParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoPreviousParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

     static void SwXTextCursor::SetPropertyValue(   SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue)      throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::uno::Any  SwXTextCursor::GetPropertyValue(    SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const ::rtl::OUString& rPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SwXTextCursor::GetPropertyStates( SwPaM& rPaM, SfxItemPropertySet& rPropSet,      const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames)            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::beans::PropertyState SwXTextCursor::GetPropertyState( SwPaM& rPaM, SfxItemPropertySet& rPropSet, const ::rtl::OUString& rPropertyName)        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
     static void SwXTextCursor::SetPropertyToDefault( SwPaM& rPaM, const SfxItemPropertySet& rPropSet,  const rtl::OUString& rPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Any  SwXTextCursor::GetPropertyDefault( SwPaM& rPaM, const SfxItemPropertySet& rPropSet,  const ::rtl::OUString& rPropertyName)   throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertySetCloner - alt
//    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  cloneProperties(void);
//    virtual void applyProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & PropertySet);

    //XDocumentInsertable - neu
    virtual void SAL_CALL insertDocumentFromURL(const rtl::OUString& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aOptions) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

    //XAutoTextInsertable - alt
//  virtual void insertAutoTextEntry(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextEntry > & ::com::sun::star::text::AutoTextEntry);

    //XSortable - neu
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor) throw( ::com::sun::star::uno::RuntimeException );

    //XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createContentEnumeration(const rtl::OUString& aServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    const SwPaM*        GetPaM() const;
    const SwDoc*        GetDoc()const;
    SwDoc*              GetDoc();
    SwUnoCrsr*          GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}
    const SwUnoCrsr*    GetCrsr()const{return (SwUnoCrsr*)GetRegisteredIn();}

    static void         SetCrsrAttr(SwPaM& rPam, const SfxItemSet& rSet, BOOL bTableMode = sal_False);
    static void         GetCrsrAttr(SwPaM& rPam, SfxItemSet& rSet, BOOL bCurrentAttrOnly = FALSE);
    static void         getTextFromPam(SwPaM& aCrsr, rtl::OUString& rBuffer);
    static SwFmtColl*   GetCurTxtFmtColl(SwPaM& rPam, BOOL bConditional);

    static void         SelectPam(SwPaM& rCrsr, sal_Bool bExpand);
    static void         SetString(SwUnoCrsr& rUnoCrsr, const rtl::OUString& rString);
};

/*-----------------20.03.98 07:47-------------------

--------------------------------------------------*/
/* das wird zunaechst nicht gebraucht - bisher fuer den XPropertySetCloner
class SwXPropertySet : public ::com::sun::star::beans::XPropertySet,
                        public UsrObject
{
    ::com::sun::star::uno::Sequence<::com::sun::star::uno::Any>*        pAny;
    ::com::sun::star::uno::Sequence< Property >*    pInfo;

public:
    // Eigentumsuebergang der Pointer!
    SwXPropertySet( ::com::sun::star::uno::Sequence<::com::sun::star::uno::Any>*        pAny,
                    ::com::sun::star::uno::Sequence<Property>*  pInfo );
    virtual ~SwXPropertySet();

    SMART_UNO_DECLARATION( SwXPropertySet, UsrObject );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

};
/*-----------------20.03.98 08:26-------------------

--------------------------------------------------*/
/* das wird zunaechst nicht gebraucht - bisher fuer den XPropertySetCloner

class SwXPropertySetInfo: public ::com::sun::star::beans::XPropertySetInfo, UsrObject
{
    ::com::sun::star::uno::Sequence< Property >*        pSeq;

public:
    SwXPropertySetInfo( const ::com::sun::star::uno::Sequence<Property>& rInfo );
    virtual ~SwXPropertySetInfo();

    SMART_UNO_DECLARATION( SwXPropertySetInfo, UsrObject );

    virtual ::com::sun::star::uno::Sequence< Property > getProperties(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual Property getPropertyByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL hasPropertyByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );
};

/*-----------------20.02.98 08:45-------------------

--------------------------------------------------*/
typedef
cppu::WeakImplHelper5
<
    ::com::sun::star::text::XTextContent,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>
SwRefBookmarkBaseClass;
class SwXBookmark : public SwRefBookmarkBaseClass,
    public SwClient
{
    SwEventListenerContainer    aLstnrCntnr;
    SwDoc*                      pDoc;
    String                      m_aName;
    BOOL                        bIsDescriptor;

public:
        SwXBookmark(SwBookmark* pBkm = 0, SwDoc* pDoc = 0);
        virtual ~SwXBookmark();
    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    SwBookmark*     GetBookmark() const { return (SwBookmark*)GetRegisteredIn(); }
    SwDoc*          GetDoc(){return pDoc;}
};

/*-----------------23.02.98 10:45-------------------

--------------------------------------------------*/

typedef cppu::WeakAggImplHelper2
<
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::lang::XServiceInfo
> SwXBodyTextBaseClass;
class SwXBodyText : public SwXBodyTextBaseClass,
                    public SwXText
{
public:
    SwXBodyText(SwDoc* pDoc);
    virtual ~SwXBodyText();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(){OWeakObject::acquire();}
    virtual void SAL_CALL release(  ) throw(){OWeakObject::release();}

    //XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  CreateTextCursor(BOOL bIgnoreTables = sal_False);
};
/*-----------------23.02.98 12:05-------------------

--------------------------------------------------*/
class SwXTextRange : public cppu::WeakImplHelper6
<
    ::com::sun::star::text::XTextRange,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XContentEnumerationAccess,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState
>,
                        public SwClient
{
    friend class SwXText;
    enum RangePosition
    {
        RANGE_IN_TEXT, // 'normaler' ::com::sun::star::text::TextRange
        RANGE_IN_FRAME,// TextPosition wurde mit einem Rahmen angelegt, der kein Uno-Objekt besitzt
        RANGE_IN_CELL, // TextPosition wurde mit einer Zelle angelegt, die kein Uno-Objekt besitzt
        RANGE_IS_TABLE, // Anker einer Tabelle
        RANGE_INVALID  // von NewInstance erzeugt
    } eRangePosition;

    SwDoc*              pDoc;
    SwTableBox*         pBox;
    const SwStartNode*  pBoxStartNode;
    SwDepend            aObjectDepend; //Format der Tabelle oder des Rahmens anmelden
    SfxItemPropertySet  aPropSet;
    //SwDepend  aFrameDepend;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >   xParentText;


    void    _CreateNewBookmark(SwPaM& rPam);
    //TODO: new exception type for protected content
    void    DeleteAndInsert(const String& rText) throw( ::com::sun::star::uno::RuntimeException );

public:
    SwXTextRange();
    SwXTextRange(SwPaM& rPam, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > & rxParent);
    SwXTextRange(SwFrmFmt& rFmt, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt, SwTableBox& rTblBox, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt, const SwStartNode& rStartNode, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt);

    virtual ~SwXTextRange();
    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createContentEnumeration(const rtl::OUString& aServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    BOOL                GetPositions(SwPaM& rToFill) const;
    const SwDoc*        GetDoc()const {return pDoc;}
    SwDoc*              GetDoc(){return pDoc;}

    SwBookmark* GetBookmark() const { return (SwBookmark*)GetRegisteredIn(); }

    static  BOOL        XTextRangeToSwPaM( SwUnoInternalPaM& rToFill,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange);
};

/* -----------------15.05.98 08:29-------------------
 *
 * --------------------------------------------------*/
typedef cppu::WeakImplHelper3
<
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::lang::XServiceInfo
>
SwXTextRangesBaseClass;
class SwXTextRanges : public SwXTextRangesBaseClass,
    public SwClient
{
    XTextRangeArr*      pRangeArr;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >           xParentText;
    XTextRangeArr*      GetRangesArray();
    SwUnoCrsr*          GetCrsr() const { return (SwUnoCrsr*)GetRegisteredIn(); }
public:
    SwXTextRanges();
    SwXTextRanges( SwUnoCrsr* pCrsr, ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent);
    SwXTextRanges(SwPaM* pCrsr);
    virtual ~SwXTextRanges();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);


    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    const SwUnoCrsr*    GetCursor() const { return (const SwUnoCrsr*)GetRegisteredIn(); }
};

/*-----------------09.03.98 13:57-------------------

--------------------------------------------------*/
struct SwTextSectionProperties_Impl;
class SwXTextSection : public cppu::WeakImplHelper6
<
    ::com::sun::star::text::XTextSection,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>,
    public SwClient
{
    SwEventListenerContainer        aLstnrCntnr;
    SfxItemPropertySet              aPropSet;

    BOOL                            m_bIsDescriptor;
    String                          m_sName;
    SwTextSectionProperties_Impl*   pProps;
public:
    SwXTextSection(SwSectionFmt* pFmt = 0);
    virtual ~SwXTextSection();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    TYPEINFO();

    //XTextSection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection >  SAL_CALL getParentSection(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection >  > SAL_CALL getChildSections(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    SwSectionFmt*   GetFmt()const {return (SwSectionFmt*)GetRegisteredIn();}
    static SwXTextSection* GetImplementation(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xRef );
};
/*-----------------12.02.98 08:01-------------------

--------------------------------------------------*/
typedef
cppu::WeakImplHelper5
<
    ::com::sun::star::text::XFootnote,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XUnoTunnel
>
SwXFootnoteBaseClass;
class SwXFootnote : public SwXFootnoteBaseClass,
    public SwXText,
    public SwClient
{
    friend class SwXFootnotes;

    SwEventListenerContainer    aLstnrCntnr;
    const SwFmtFtn*             pFmtFtn;
    BOOL                        m_bIsDescriptor;
    String                      m_sLabel;
    BOOL                        m_bIsEndnote;

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor();
public:
    SwXFootnote(BOOL bEndnote);
    SwXFootnote(SwDoc* pDoc, const SwFmtFtn& rFmt);
    virtual ~SwXFootnote();

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(){SwXFootnoteBaseClass::acquire();}
    virtual void SAL_CALL release(  ) throw(){SwXFootnoteBaseClass::release();}

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XFootnote
    virtual rtl::OUString SAL_CALL getLabel(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setLabel(const rtl::OUString& aLabel) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    const SwFmtFtn* FindFmt() const;
    void            Invalidate();
};

/*-----------------07.04.98 08:10-------------------

--------------------------------------------------*/
class SwXParagraphEnumeration : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XEnumeration,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >           xParentText;

    BOOL                bFirstParagraph;
    SwUnoCrsr*          GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}
    CursorType          eCursorType;
public:
    SwXParagraphEnumeration(SwXText* pParent, SwPosition& rPos, CursorType eType);
    SwXParagraphEnumeration(SwXText* pParent, SwUnoCrsr* pCrsr, CursorType eType);
    virtual ~SwXParagraphEnumeration();


    //XEnumeration
    virtual BOOL SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void            Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};
/*-----------------07.04.98 08:15-------------------

--------------------------------------------------*/
SV_DECL_PTRARR(SwDependArr, SwDepend*, 2, 2);
class SwXParagraph : public cppu::WeakImplHelper8
<
    ::com::sun::star::text::XTextRange,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::text::XTextContent,
    ::com::sun::star::container::XContentEnumerationAccess
>,
    public SwClient
{
    SwEventListenerContainer    aLstnrCntnr;
    SfxItemPropertySet          aPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    rtl::OUString               m_sText;
    BOOL                        m_bIsDescriptor;

    SwUnoCrsr*              GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}
public:
    SwXParagraph();
    SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr);
    virtual ~SwXParagraph();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - war XTextPortionEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createContentEnumeration(const rtl::OUString& aServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    BOOL            IsDescriptor() const {return m_bIsDescriptor;}

    static BOOL getDefaultTextContentValue(::com::sun::star::uno::Any& rAny,
        const rtl::OUString& rPropertyName, USHORT nWID = 0);
    static SwXParagraph* GetImplementation(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xRef );
    //falls es mal als Service erzeugt werden kann
    //void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    void attachToText(SwXText* pParent, SwUnoCrsr* pCrsr);
};
/* -----------------23.03.99 12:57-------------------
 *
 * --------------------------------------------------*/
#define PARAFRAME_PORTION_PARAGRAPH     0
#define PARAFRAME_PORTION_CHAR          1
#define PARAFRAME_PORTION_TEXTRANGE     2

class SwXParaFrameEnumeration : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XEnumeration,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >  xNextObject;  //hasMoreElements legt das Objekt schon an
    SwDependArr     aFrameArr;      //wird im Ctor gefuellt

    SwUnoCrsr*      GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}
    BOOL            CreateNextObject();
    void            FillFrame(SwUnoCrsr& rUnoCrsr);
public:
    SwXParaFrameEnumeration(const SwUnoCrsr& rUnoCrsr, sal_uInt8 nParaFrameMode, SwFrmFmt* pFmt = 0);
    ~SwXParaFrameEnumeration();

    //XEnumeration
    virtual BOOL SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};
/* -----------------29.05.98 14:42-------------------
 *
 * --------------------------------------------------*/

class SwXTextPortionEnumeration : public cppu::WeakImplHelper3
<
    ::com::sun::star::container::XEnumeration,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel
>,
    public SwClient
{
    XTextRangeArr       aPortionArr;    //all portions are created in the ctor
    SwDependArr         aFrameArr;      //wird im Ctor gefuellt - mit am Zeichen gebundenen Rahmen
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >           xParent;
    BOOL                bAtEnd;
    BOOL                bFirstPortion;


    SwUnoCrsr*          GetCrsr() const { return (SwUnoCrsr*)GetRegisteredIn(); }
    SwXTextPortionEnumeration();
    void                CreatePortions();
public:
    SwXTextPortionEnumeration(SwPaM& rParaCrsr,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent);
    virtual ~SwXTextPortionEnumeration();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XEnumeration
    virtual BOOL SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};

/* -----------------29.05.98 14:42-------------------
 *  Keine Anmeldung am Feld - Das Feld muss am Cursor stehen
 * --------------------------------------------------*/
enum SwTextPortionType
{
    PORTION_TEXT,
    PORTION_FIELD,
    PORTION_FRAME,
    PORTION_FOOTNOTE,
    PORTION_CONTROL_CHAR,
    PORTION_REFMARK_START,
    PORTION_REFMARK_END,
    PORTION_TOXMARK_START,
    PORTION_TOXMARK_END,
    PORTION_BOOKMARK_START,
    PORTION_BOOKMARK_END
};

class SwXTextPortion : public cppu::WeakImplHelper6
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::text::XTextRange,
    ::com::sun::star::text::XTextField,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::container::XContentEnumerationAccess,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    SwEventListenerContainer    aLstnrCntnr;
    SfxItemPropertySet          aPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >    xRefMark;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >    xTOXMark;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >    xBookmark;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >    xFootnote;
    const SwFmtFld*             pFmtFld;
    SwDepend                    aFrameDepend;
    SwFrmFmt*                   pFrameFmt;
    SwTextPortionType           ePortionType;
    sal_Int16                   nControlChar;

    BOOL                        bIsCollapsed;

    SwFmtFld*           GetFldFmt(BOOL bInit = sal_False);
    SwUnoCrsr*          GetCrsr() const { return (SwUnoCrsr*)GetRegisteredIn(); }
public:
    SwXTextPortion(const SwUnoCrsr* pPortionCrsr, ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, SwTextPortionType eType   );
    SwXTextPortion(const SwUnoCrsr* pPortionCrsr, ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, SwFrmFmt& rFmt );
    virtual ~SwXTextPortion();

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XTextField
    virtual rtl::OUString SAL_CALL getPresentation(BOOL bShowCommand) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createContentEnumeration(const rtl::OUString& aServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void                Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void                SetRefMark( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >  xMark)
                            {xRefMark = xMark;}

    void                SetTOXMark( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >  xMark)
                            {xTOXMark = xMark;}

    void                SetBookmark( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >  xMark)
                            {xBookmark = xMark;}

    void                SetFootnote( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >  xMark)
                            {xFootnote = xMark;}

    void                SetControlChar(sal_Int16 nSet) {nControlChar = nSet;}

    void                SetIsCollapsed(BOOL bSet) { bIsCollapsed = bSet;}

    //falls es mal als service erzeugt werden kann
    //void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
};
/* -----------------29.09.98 09:01-------------------
 *
 * --------------------------------------------------*/
/* os: 04.12.98 11:40 hier gibt es noch keine sinnvolle Definition
#define MAX_SORT_FIELDS 3 // das ist eher willkuerlich (wie im Dialog)
class SwXTextSortDescriptor : public XTextSortDescriptor,
                                public UsrObject
{
    SwSortOptions   aSortOptions;
    BOOL            bUseHeader;
public:
    SwXTextSortDescriptor(BOOL bUsedInTable);
    virtual ~SwXTextSortDescriptor();

    //XTextSortDescriptor
    virtual sal_Unicode getTextColumnSeparator(void) const;
    virtual void setTextColumnSeparator(sal_Unicode TextColumnSeparator_);

    //XSortDescriptor
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::SortField > getSortFields(void) const;
    virtual void setSortFields(const ::com::sun::star::uno::Sequence< ::com::sun::star::util::SortField >& SortFields_);
    virtual sal_uInt16 getMaxFieldCount(void) const;
    virtual ColumnsOrRows getOrientation(void) const;
    virtual void setOrientation(ColumnsOrRows Orientation_);
    virtual BOOL getUseHeader(void) const;
    virtual void setUseHeader(BOOL UseHeader_);

    const SwSortOptions&    GetSortOptions() const {return aSortOptions;}
    void                    SetSortOptions(const SwSortOptions& rSortOpt);
};
*/
/* -----------------25.08.98 11:02-------------------
 *
 * --------------------------------------------------*/
typedef
cppu::WeakImplHelper2
<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XEnumerationAccess
>
SwXHeadFootTextBaseClass;
class SwXHeadFootText : public SwXHeadFootTextBaseClass,
    public SwXText,
    public SwClient
{
    SwFrmFmt*           GetFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
    BOOL                bIsHeader;
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor();
public:
    SwXHeadFootText(SwFrmFmt& rHeadFootFmt, BOOL bHeader);
    ~SwXHeadFootText();

    TYPEINFO();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(){SwXHeadFootTextBaseClass::acquire();}
    virtual void SAL_CALL release(  ) throw(){SwXHeadFootTextBaseClass::release();}

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};
/* -----------------27.08.98 15:11-------------------
 *
 * --------------------------------------------------*/
class SwXReferenceMark : public SwRefBookmarkBaseClass,
    public SwClient
{
    SwEventListenerContainer    aLstnrCntnr;
    SwDoc*                      pDoc;
    const SwFmtRefMark*         pMark;
    String                      sMarkName;
    BOOL                        m_bIsDescriptor;

    BOOL    IsValid() const {return 0 != GetRegisteredIn();}
    void    InsertRefMark(SwPaM& rPam, SwDoc* pDoc);
public:
    SwXReferenceMark(SwDoc* pDoc, const SwFmtRefMark* pMark);
    ~SwXReferenceMark();

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    const SwFmtRefMark* GetMark()const {return pMark;}
    const String&       GetMarkName() const {return sMarkName;}
    SwDoc*              GetDoc() const{return pDoc;}
    void                Invalidate();
};
/* -----------------01.09.98 16:09-------------------
 *
 * --------------------------------------------------*/
// os: 04.12.98 11:44 - keine Definition
//class SwXRedline : public XRedline,
/*
class SwXRedline : public XRedline,
                    public UsrObject,
                        public SwClient
{
    SwDoc*              pDoc;
    const SwRedline*    pRedline;

    BOOL                IsValid() const;
public:
    SwXRedline(SwDoc* pDoc, const SwRedline* pRedline);
    virtual ~SwXRedline();

    //XRedline
    virtual sal_uInt16 getActionType(void) const;
    virtual rtl::OUString getAuthor(void) const;
    virtual ::com::sun::star::util::DateTime getCreationDateTime(void) const;
    virtual rtl::OUString getDescription(void) const;
    virtual void setDescription(const rtl::OUString& Description_);

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};


*/
#endif


