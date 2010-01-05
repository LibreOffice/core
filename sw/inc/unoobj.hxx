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
#include <unocrsr.hxx>
#include <svtools/itemprop.hxx>
#include <frmfmt.hxx>
#include <flyenum.hxx>
#include <fldbas.hxx>
#include <fmtcntnt.hxx>
#include <toxe.hxx>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories

#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations
#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/implbase10.hxx>
#include <cppuhelper/weakref.hxx>

#include <unomid.h>

#include <deque>
#include <boost/shared_ptr.hpp>


class SwFmtFtn;
class GetCurTxtFmtColl;


struct FrameDependSortListEntry {
    xub_StrLen nIndex;
    sal_uInt32 nOrder;
    ::boost::shared_ptr<SwDepend> pFrameDepend;
    FrameDependSortListEntry (xub_StrLen const i_nIndex,
                sal_uInt32 const i_nOrder, SwDepend * const i_pDepend)
        : nIndex(i_nIndex), nOrder(i_nOrder), pFrameDepend(i_pDepend) { }
};
typedef ::std::deque< FrameDependSortListEntry >
    FrameDependSortList_t;

typedef ::std::deque< ::boost::shared_ptr<SwDepend> >
    FrameDependList_t;


/* -----------------26.06.98 16:18-------------------
 *
 * --------------------------------------------------*/

SwPageDesc* GetPageDescByName_Impl(SwDoc& rDoc, const String& rName);
::com::sun::star::uno::Sequence< sal_Int8 > CreateUnoTunnelId();

// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         FrameDependSortList_t & rFrames,
                         const bool _bAtCharAnchoredObjs );

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

void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew);

/* -----------------03.12.98 12:22-------------------
 *
 * --------------------------------------------------*/
class OTextCursorHelper;
class SwXTextRange;
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
    const SfxItemPropertySet*   m_pPropSet;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishOrAppendParagraph(
                bool bFinish,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & CharacterAndParagraphProperties )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual void PrepareForAttach( ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const SwXTextRange* const pRange, const SwPaM * const pPam);

public: /*not protected because C++ is retarded*/
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
    virtual bool CheckForOwnMemberMeta(const SwXTextRange* const pRange,
            const SwPaM* const pPam, bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    //
    void            Invalidate() {bObjectValid = sal_False;}
    BOOL            IsValid()const {return bObjectValid;}

    CursorType      GetTextType() {return eCrsrType;}
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
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >
        m_xNextObject;    // created by hasMoreElements
    FrameDependList_t m_Frames;

    SwUnoCrsr*          GetCursor() const
    {return static_cast<SwUnoCrsr*>(const_cast<SwModify*>(GetRegisteredIn()));}

public:
    SwXParaFrameEnumeration(const SwPaM& rPaM,
        sal_uInt8 nParaFrameMode, SwFrmFmt* pFmt = 0);
    ~SwXParaFrameEnumeration();

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw( ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
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

#endif

