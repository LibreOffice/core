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
#ifndef _UNOIDX_HXX
#define _UNOIDX_HXX

#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXMARK_HPP_
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _TOXE_HXX
#include <toxe.hxx>
#endif
namespace binfilter {
class SwTOXBaseSection;
class SwTOXMark;
class SwTOXType;
/* -----------------07.12.98 10:08-------------------
 *
 * --------------------------------------------------*/
class SwDocIdxProperties_Impl;
class SwXIndexStyleAccess_Impl;
class SwXIndexTokenAccess_Impl;

class SwXDocumentIndex : public cppu::WeakImplHelper5
<
    ::com::sun::star::text::XDocumentIndex,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>,
    public SwClient
{
    friend class SwXIndexStyleAccess_Impl;
    friend class SwXIndexTokenAccess_Impl;

    SwEventListenerContainer	aLstnrCntnr;
    const SfxItemPropertyMap*	_pMap;
    SwDoc* 						m_pDoc;
    const SwTOXBaseSection*		pBase;
    TOXTypes 					eTOXType;

    //Descriptor-Interface
    sal_Bool					bIsDescriptor;
    SwDocIdxProperties_Impl*	pProps;

    SwXIndexStyleAccess_Impl*	pStyleAccess;
    SwXIndexTokenAccess_Impl*	pTokenAccess;

    SwSectionFmt*	GetFmt()const {return (SwSectionFmt*)GetRegisteredIn();}

    sal_Bool							IsDescriptor() const {return bIsDescriptor;}
    const SwDocIdxProperties_Impl*	GetProperties_Impl() const { return pProps;}
    SwDocIdxProperties_Impl*		GetProperties_Impl() { return pProps;}
    TOXTypes 						GetTOXType() const {return eTOXType;}

    SwXIndexStyleAccess_Impl*	GetStyleAccess() const {return pStyleAccess;}
    void 						SetStyleAccess(SwXIndexStyleAccess_Impl* pSet)
                                        {pStyleAccess = pSet;}

    SwXIndexTokenAccess_Impl*	GetTokenAccess() const {return pTokenAccess;}
    void 						SetTokenAccess(SwXIndexTokenAccess_Impl* pSet)
                                        {pTokenAccess = pSet;}
protected:
    virtual ~SwXDocumentIndex();
public:
    SwXDocumentIndex(const SwTOXBaseSection* = 0, SwDoc* = 0);
    //Descriptor-Ctor
    SwXDocumentIndex(TOXTypes eToxType, SwDoc& rDoc);
    

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XDocumentIndex
    virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL update(  ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XNamed
    virtual ::rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const ::rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
};
/* -----------------07.12.98 10:08-------------------
 *
 * --------------------------------------------------*/
class SwXDocumentIndexMark : public cppu::WeakImplHelper4
<
    ::com::sun::star::text::XDocumentIndexMark,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XUnoTunnel
>,
        public SwClient
{
    SwEventListenerContainer	aLstnrCntnr;
    SwDepend					aTypeDepend;
    SwDoc* 						m_pDoc;
    const SwTOXMark* 			m_pTOXMark;
    const SfxItemPropertyMap*	_pMap;

    sal_Bool					bIsDescriptor;
    sal_Bool					bMainEntry;
    TOXTypes 					eType;
    sal_uInt16 						nLevel;
    String						sAltText;
    String 						sPrimaryKey;
    String 						sSecondaryKey;
    String                      sTextReading;
    String                      sPrimaryKeyReading;
    String                      sSecondaryKeyReading;
    String 						sUserIndexName;

    void 				InitMap(TOXTypes eToxType);
protected:
    virtual ~SwXDocumentIndexMark();
public:
    SwXDocumentIndexMark(TOXTypes eToxType);
    SwXDocumentIndexMark(const SwTOXType* pType,
                        const SwTOXMark* pMark,
                        SwDoc* pDc);
    

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getMarkEntry(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setMarkEntry(const ::rtl::OUString& aIndexEntry) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

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

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    SwTOXType* 			GetTOXType() const {return (SwTOXType*)aTypeDepend.GetRegisteredIn();}
    const SwTOXMark* 	GetTOXMark() const {return m_pTOXMark;}

    static SwXDocumentIndexMark*	GetObject(SwTOXType* pType, const SwTOXMark* pMark,
                                                SwDoc* pDoc);
    void				Invalidate();
};
/* -----------------05.05.99 12:27-------------------
 *
 * --------------------------------------------------*/

class SwXDocumentIndexes : public SwCollectionBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXDocumentIndexes();
public:
    SwXDocumentIndexes(SwDoc* pDoc);
    

//	SMART_UNO_DECLARATION( SwXDocumentIndexes, UsrObject );

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  > 	getIdlClasses(void)

//											throw( ::com::sun::star::uno::RuntimeException );
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//	virtual sal_Bool			queryInterface( ::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  & rOut );


    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static SwXDocumentIndex* GetObject(const SwTOXBaseSection* pTOX);
};
/* -----------------13.09.99 16:39-------------------

 --------------------------------------------------*/
class SwXIndexStyleAccess_Impl : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XIndexReplace,
    ::com::sun::star::lang::XServiceInfo
>
{
    SwXDocumentIndex& 	rParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XDocumentIndex > 	xParent;
protected:
    virtual ~SwXIndexStyleAccess_Impl();
public:
    SwXIndexStyleAccess_Impl(SwXDocumentIndex& rParentIdx);
    

    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

/* -----------------13.09.99 16:39-------------------

 --------------------------------------------------*/
class SwXIndexTokenAccess_Impl : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XIndexReplace,
    ::com::sun::star::lang::XServiceInfo
>
{
    SwXDocumentIndex& 	rParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XDocumentIndex > 	xParent;
    sal_Int32 				nCount;
protected:
    virtual ~SwXIndexTokenAccess_Impl();
public:
    SwXIndexTokenAccess_Impl(SwXDocumentIndex& rParentIdx);
    


    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

} //namespace binfilter
#endif


