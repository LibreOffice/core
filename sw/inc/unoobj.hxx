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
#include <fldbas.hxx>
#include <fmtcntnt.hxx>
#include <toxe.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories

#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/implbase10.hxx>
#include <cppuhelper/weakref.hxx>

#include <unomid.h>

#include <deque>
#include <boost/shared_ptr.hpp>


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

#endif

