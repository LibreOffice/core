/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoobj.hxx,v $
 *
 * $Revision: 1.49 $
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
#ifndef _UNOOBJ_HXX
#define _UNOOBJ_HXX
#include <unoevtlstnr.hxx>
#include <unobaseclass.hxx>
#include <svtools/itemprop.hxx>
#include <svtools/svarray.hxx>
#include <frmfmt.hxx>
#include <flyenum.hxx>
#include <fldbas.hxx>
#include <fmtcntnt.hxx>
#include <pam.hxx>
#include <toxe.hxx>
#include <sortopt.hxx>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XWordCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XSentenceCursor.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
//#ifndef _COM_SUN_STAR_BEANS_XTOLERANTMULTIPROPERTYSET_HPP_
//#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
//#endif
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories

#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations
#include <cppuhelper/implbase7.hxx> // helper for implementations
#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/implbase10.hxx>
#include <cppuhelper/implbase12.hxx>    // helper for implementations
#include "TextCursorHelper.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/text/XFormField.hpp>

#include <unomid.h>
#include <tools/link.hxx>

#include <IDocumentBookmarkAccess.hxx>

class SwUnoCrsr;
class SwCursor;
class SwBookmark;
class SwFmtFtn;
class SwFmtRefMark;
class GetCurTxtFmtColl;
/* -----------------------------01.12.00 18:09--------------------------------

 ---------------------------------------------------------------------------*/
class SwParaSelection
{
    SwUnoCrsr* pUnoCrsr;
public:
    SwParaSelection(SwUnoCrsr* pCrsr);
    ~SwParaSelection();
};

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
    CURSOR_REDLINE,
    CURSOR_ALL,          // fuer Search&Replace
    CURSOR_SELECTION,    // create a paragraph enumeration from a text range or cursor
    CURSOR_SELECTION_IN_TABLE
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

SV_DECL_PTRARR(SwDependArr, SwDepend*, 2, 2)

SwPageDesc* GetPageDescByName_Impl(SwDoc& rDoc, const String& rName);
::com::sun::star::uno::Sequence< sal_Int8 > CreateUnoTunnelId();

// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         SwDependArr& rFrameArr,
                         const bool _bAtCharAnchoredObjs );

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
SV_DECL_PTRARR(XTextRangeArr, XTextRangeRefPtr, 4, 4)

void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew);

/* -----------------03.12.98 12:22-------------------
 *
 * --------------------------------------------------*/
class SwXTextRange;
class SwXTextCursor;
class SwXText : public ::com::sun::star::lang::XTypeProvider,
                public ::com::sun::star::text::XTextRangeCompare,
                public ::com::sun::star::text::XRelativeTextContentInsert,
                public ::com::sun::star::text::XRelativeTextContentRemove,
                public ::com::sun::star::beans::XPropertySet,
                public ::com::sun::star::lang::XUnoTunnel,
                public ::com::sun::star::text::XTextAppendAndConvert
{
    SwDoc*                      pDoc;
    BOOL                        bObjectValid;
    CursorType                  eCrsrType;
    const SfxItemPropertyMap*   _pMap;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishOrAppendParagraph(
                bool bFinish,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & CharacterAndParagraphProperties )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual const SwStartNode *GetStartNode() const;
public:
                SwXText(SwDoc* pDc, CursorType eType);
    virtual     ~SwXText();

    const SwDoc*            GetDoc()const {return pDoc;}
    SwDoc*                  GetDoc()    {return pDoc;}

    // wenn ein SwXText attached wird, wird das Doc gesetzt
    void                    SetDoc(SwDoc* pDc)
                                {DBG_ASSERT(!pDoc || !pDc, "Doc schon gesetzt?");
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


    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XParagraphAppend
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL appendParagraph( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL finishParagraph( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XTextPortionAppend
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL appendTextPortion( const ::rtl::OUString& Text, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XTextContentAppend
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL appendTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& TextContent, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XTextConvert
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > SAL_CALL convertToTextFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& Start, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& End, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& FrameProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextTable > SAL_CALL convertToTable( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > > > >& TableRanges,
       const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > >& CellProperties,
       const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& RowProperties,
       const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& TableProperties
       ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);


    //
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor()throw(::com::sun::star::uno::RuntimeException);
    INT16   ComparePositions(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange>& xPos1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange>& xPos2) throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);
    BOOL    CheckForOwnMember(const SwXTextRange* pRange1, const OTextCursorHelper* pCursor1)throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    //
    void            Invalidate() {bObjectValid = sal_False;}
    BOOL            IsValid()const {return bObjectValid;}

    CursorType      GetTextType() {return eCrsrType;}
};

/* -----------------03.12.98 12:16-------------------
 *
 * --------------------------------------------------*/
enum SwGetPropertyStatesCaller
{
    SW_PROPERTY_STATE_CALLER_DEFAULT,
    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION,
    SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY
};
#define CRSR_ATTR_MODE_TABLE        1   //attributes should be applied to a table selection
#define CRSR_ATTR_MODE_DONTREPLACE  2   //attributes should be added, not replaced

typedef cppu::WeakImplHelper12<
                                ::com::sun::star::text::XSentenceCursor,
                                ::com::sun::star::text::XWordCursor,
                                ::com::sun::star::text::XParagraphCursor,
                                ::com::sun::star::text::XRedline,
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::beans::XPropertyState,
                                ::com::sun::star::document::XDocumentInsertable,
                                ::com::sun::star::lang::XServiceInfo,
                                ::com::sun::star::util::XSortable,
                                ::com::sun::star::container::XContentEnumerationAccess,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::beans::XMultiPropertyStates
                            > SwXTextCursor_Base;
class SwXTextCursor : public SwXTextCursor_Base,
    public SwClient,
    public OTextCursorHelper
{
    SwEventListenerContainer    aLstnrCntnr;
    SfxItemPropertySet          aPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    SwSortOptions*              pLastSortOptions;

    CursorType                  eType;

    // --> FME 2006-03-07 #126177# We need to track if the RemoveCursor_Impl
    // user event has been posted. In this case we have to remove the user
    // event in ~SwXTextCursor().
    ULONG mnUserEventId;
    bool mbRemoveUserEvent;
    // <--

    void    DeleteAndInsert(const String& rText);

    DECL_STATIC_LINK( SwXTextCursor, RemoveCursor_Impl,
                      ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XInterface>* );

protected:
    virtual ~SwXTextCursor();
public:
    SwXTextCursor(::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, const SwPosition& rPos,
                    CursorType eSet, SwDoc* pDoc, const SwPosition* pMark = 0);
    SwXTextCursor(::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent, SwUnoCrsr* pSourceCrsr, CursorType eSet = CURSOR_ALL);

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    DECLARE_XINTERFACE()

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

    //ParagraphCursor - neu
    virtual BOOL SAL_CALL isStartOfParagraph(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL isEndOfParagraph(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoStartOfParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoEndOfParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoNextParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL gotoPreviousParagraph(BOOL Expand) throw( ::com::sun::star::uno::RuntimeException );

    //XRedline
    virtual void SAL_CALL makeRedline( const ::rtl::OUString& RedlineType, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& RedlineProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

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

    static void SetPropertyValue(
                        SwPaM& rPaM,
                        const SfxItemPropertySet& rPropSet,
                        const rtl::OUString& rPropertyName,
                        const ::com::sun::star::uno::Any& aValue,
                        const SfxItemPropertyMap* pMap = 0,
                        USHORT nAttrMode = 0)
                            throw(::com::sun::star::beans::UnknownPropertyException,
                                ::com::sun::star::beans::PropertyVetoException,
                                ::com::sun::star::lang::IllegalArgumentException,
                                ::com::sun::star::lang::WrappedTargetException,
                                ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::uno::Any  GetPropertyValue(   SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const ::rtl::OUString& rPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > GetPropertyStates(    SwPaM& rPaM, SfxItemPropertySet& rPropSet,      const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, SwGetPropertyStatesCaller eCaller = SW_PROPERTY_STATE_CALLER_DEFAULT)          throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
     static ::com::sun::star::beans::PropertyState GetPropertyState( SwPaM& rPaM, SfxItemPropertySet& rPropSet, const ::rtl::OUString& rPropertyName)       throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
     static void SetPropertyToDefault( SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const rtl::OUString& rPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Any  GetPropertyDefault( SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const ::rtl::OUString& rPropertyName)   throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XDocumentInsertable - neu
    virtual void SAL_CALL insertDocumentFromURL(const rtl::OUString& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aOptions) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

    //XSortable - neu
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor) throw( ::com::sun::star::uno::RuntimeException );

    //XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createContentEnumeration(const rtl::OUString& aServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XMultiPropertyStates
    //virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);


    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    virtual const SwPaM*        GetPaM() const;
    virtual SwPaM*              GetPaM();
    virtual const SwDoc*        GetDoc() const;
    virtual SwDoc*              GetDoc();
    SwUnoCrsr*          GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}
    const SwUnoCrsr*    GetCrsr()const{return (SwUnoCrsr*)GetRegisteredIn();}

    static void         SetCrsrAttr(SwPaM& rPam, const SfxItemSet& rSet, USHORT nAttrMode );
    // --> OD 2006-07-12 #i63870#
    // split third parameter <bCurrentAttrOnly> into new parameters <bOnlyTxtAttr>
    // and <bGetFromChrFmt> to get better control about resulting <SfxItemSet>
//    static void         GetCrsrAttr(SwPaM& rPam, SfxItemSet& rSet, BOOL bCurrentAttrOnly = FALSE);
    static void         GetCrsrAttr( SwPaM& rPam,
                                     SfxItemSet& rSet,
                                     BOOL bOnlyTxtAttr = FALSE,
                                     BOOL bGetFromChrFmt = TRUE );
    // <--
    static void         getTextFromPam(SwPaM& aCrsr, rtl::OUString& rBuffer);
    static SwFmtColl*   GetCurTxtFmtColl(SwPaM& rPam, BOOL bConditional);

    static void         SelectPam(SwPaM& rCrsr, sal_Bool bExpand);
    static void         SetString(SwCursor& rCrsr, const rtl::OUString& rString);

    static              ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                            createSortDescriptor(sal_Bool bFromTable);
    static sal_Bool     convertSortProperties(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor,
                            SwSortOptions& rSortOpt);

    // --> FME 2006-03-07 #126177#
    void DoNotRemoveUserEvent() { mbRemoveUserEvent = false; }
    // <--
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
*/
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
*/

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
protected:
    SwEventListenerContainer    aLstnrCntnr;
    SwDoc*                      pDoc;
    String                      m_aName;
    BOOL                        bIsDescriptor;
protected:
    virtual ~SwXBookmark();
public:
        SwXBookmark(SwBookmark* pBkm = 0, SwDoc* pDoc = 0);

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

    void attachToRangeEx(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange, IDocumentBookmarkAccess::BookmarkType eMark)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    SwBookmark*     GetBookmark() const { return (SwBookmark*)GetRegisteredIn(); }
    SwDoc*          GetDoc(){return pDoc;}
};

typedef cppu::ImplInheritanceHelper1< SwXBookmark, ::com::sun::star::text::XFormField > SwXFieldmark_BASE;

class SwXFieldmark : public SwXFieldmark_BASE
{
private:
    bool isReplacementObject;
public:
    SwXFieldmark(bool isReplacementObject, SwBookmark* pBkm = 0, SwDoc* pDoc = 0);

    virtual void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDescription(void)  throw( ::com::sun::star::uno::RuntimeException );
    virtual ::sal_Int16 SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getRes(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setType( ::sal_Int16 fieldType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRes( ::sal_Int16 res ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDescription( const ::rtl::OUString& description ) throw (::com::sun::star::uno::RuntimeException);

//    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException);

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
protected:
    virtual ~SwXBodyText();
public:
    SwXBodyText(SwDoc* pDoc);


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
class SW_DLLPUBLIC SwXTextRange : public cppu::WeakImplHelper8
<
    ::com::sun::star::text::XTextRange,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XContentEnumerationAccess,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::text::XRedline
>,  public SwClient
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
protected:
    virtual ~SwXTextRange();
public:
    SwXTextRange(SwPaM& rPam, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > & rxParent);
    SwXTextRange(SwFrmFmt& rFmt, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt, SwTableBox& rTblBox, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt, const SwStartNode& rStartNode, SwPaM& rPam);
    SwXTextRange(SwFrmFmt& rTblFmt);


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

    //XEnumerationAccess
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

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XRedline
    virtual void SAL_CALL makeRedline( const ::rtl::OUString& RedlineType, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& RedlineProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    BOOL                GetPositions(SwPaM& rToFill) const;
    const SwDoc*        GetDoc()const {return pDoc;}
    SwDoc*              GetDoc(){return pDoc;}

    SwBookmark* GetBookmark() const { return (SwBookmark*)GetRegisteredIn(); }

    static  BOOL        XTextRangeToSwPaM( SwUnoInternalPaM& rToFill,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange);
    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  CreateTextRangeFromPosition(SwDoc* pDoc,
                        const SwPosition& rPos, const SwPosition* pMark);
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
protected:
    virtual ~SwXTextRanges();
public:
    SwXTextRanges();
    SwXTextRanges(SwPaM* pCrsr);


    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
            throw(::com::sun::star::uno::RuntimeException);


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

class SwXTextSection;
class SwXTextSectionClient : public SwClient
{
    friend class SwXTextSection;
    SwXTextSection*                                                                 m_pSection;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XTextSection >    m_xReference;
    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    SwXTextSectionClient(
            SwSectionFmt& rFmt,
            SwXTextSection& rTextSection,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection > xSection );
    virtual ~SwXTextSectionClient();
    DECL_STATIC_LINK( SwXTextSectionClient, RemoveSectionClient_Impl,
                      SwXTextSectionClient* );

public:
    TYPEINFO();
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection > GetXTextSection();

    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection >
            CreateXTextSection(SwSectionFmt* pFmt = 0, BOOL bIndexHeader = FALSE );
    static SwXTextSectionClient* Create(
            SwXTextSection& rSection,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextSection > xSection,
            SwSectionFmt& rFmt );
};

struct SwTextSectionProperties_Impl;
class SwXTextSection : public cppu::WeakImplHelper7
<
    ::com::sun::star::text::XTextSection,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XMultiPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>/*,
    public SwClient*/
{
    friend class SwXTextSectionClient;
    SwEventListenerContainer        aLstnrCntnr;
    SfxItemPropertySet              aPropSet;

    BOOL                            m_bIsDescriptor;
    BOOL                            m_bIndexHeader;
    String                          m_sName;
    SwTextSectionProperties_Impl*   pProps;
    SwXTextSectionClient*           m_pClient;
protected:
    void SAL_CALL SetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL GetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    SwXTextSection(sal_Bool bWithFormat, sal_Bool bIndexHeader = FALSE);
    virtual ~SwXTextSection();
    void                            ResetClient() {m_pClient = 0;}
    void                            SetClient( SwXTextSectionClient* pClient ){m_pClient = pClient;}

public:
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

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

    //XMultiPropertySet
//    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

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
//   virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    SwSectionFmt*   GetFmt()const;
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
    virtual const SwStartNode *GetStartNode() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor()throw(::com::sun::star::uno::RuntimeException);
    virtual ~SwXFootnote();
public:
    SwXFootnote(BOOL bEndnote);
    SwXFootnote(SwDoc* pDoc, const SwFmtFtn& rFmt);


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

    const SwFmtFtn* FindFmt() const {return GetDoc() ? pFmtFtn : 0;}
    void            Invalidate();
};

/*-----------------07.04.98 08:10-------------------

--------------------------------------------------*/
class SwXParagraphEnumeration : public SwSimpleEnumerationBaseClass,
    public SwClient
{
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >           xParentText;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >    xNextPara;

    const SwTable *     pOwnTable;
    const SwStartNode * pOwnStartNode;  // start node of the cell the enumeration
                                        // belongs to.
                                        // Used to restrict the movement of the
                                        // UNO cursor to the cell and its
                                        // embedded tables.
    sal_Int32           nFirstParaStart;
    sal_Int32           nLastParaEnd;
    ULONG               nEndIndex;
    CursorType          eCursorType;
    BOOL                bFirstParagraph;

    SwUnoCrsr*          GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}

protected:
    virtual ~SwXParagraphEnumeration();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > SAL_CALL NextElement_Impl(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

public:
    SwXParagraphEnumeration(SwXText* pParent, SwPosition& rPos, CursorType eType);
    SwXParagraphEnumeration(SwXText* pParent, SwUnoCrsr* pCrsr, CursorType eType);



    //XEnumeration
    virtual BOOL SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void            Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    //non-Uno functions

    void                SetOwnTable( const SwTable* pTable )        { pOwnTable = pTable; }
    const SwTable*      GetOwnTable() const                         { return pOwnTable; }
    void                SetOwnStartNode( const SwStartNode* pNode ) { pOwnStartNode = pNode; }
    const SwStartNode*  GetOwnStartNode() const                     { return pOwnStartNode; }
};
/*-----------------07.04.98 08:15-------------------

--------------------------------------------------*/
class SwXParagraph : public cppu::WeakImplHelper9
<
//    ::com::sun::star::beans::XTolerantMultiPropertySet,
    ::com::sun::star::beans::XMultiPropertySet,
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
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >                   xParentText;
    SwEventListenerContainer    aLstnrCntnr;
    SfxItemPropertySet          aPropSet;
    rtl::OUString               m_sText;
    sal_Int32                   nSelectionStartPos;
    sal_Int32                   nSelectionEndPos;
    BOOL                        m_bIsDescriptor;


protected:
    void SAL_CALL SetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL GetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

//    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL GetPropertyValuesTolerant_Impl(
//        const ::com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames,
//        sal_Bool bDirectValuesOnly ) throw (::com::sun::star::uno::RuntimeException);

    virtual ~SwXParagraph();
public:
    SwXParagraph();
    SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr, sal_Int32 nSelStart = -1, sal_Int32 nSelEnd = - 1);


    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTolerantMultiPropertySet
//    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::SetPropertyTolerantFailed > SAL_CALL setPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
//    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::GetPropertyTolerantResult > SAL_CALL getPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
//    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL getDirectPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);

    //XMultiPropertySet
//    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

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

    SwUnoCrsr*      GetCrsr(){return (SwUnoCrsr*)GetRegisteredIn();}

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

    const sal_Int32     nStartPos;
    const sal_Int32     nEndPos;

    SwUnoCrsr*          GetCrsr() const { return (SwUnoCrsr*)GetRegisteredIn(); }
    SwXTextPortionEnumeration();
    void                CreatePortions();
protected:
    virtual ~SwXTextPortionEnumeration();
public:
    SwXTextPortionEnumeration(SwPaM& rParaCrsr,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  xParent,
            sal_Int32 nStart, sal_Int32 nEnd );


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
    virtual const SwStartNode *GetStartNode() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor()throw(::com::sun::star::uno::RuntimeException);
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
    void    InsertRefMark( SwPaM& rPam );
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
#endif


