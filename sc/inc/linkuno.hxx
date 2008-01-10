/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linkuno.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:08:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_LINKUNO_HXX
#define SC_LINKUNO_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XDDELINK_HPP_
#include <com/sun/star/sheet/XDDELink.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDDELINKRESULTS_HPP_
#include <com/sun/star/sheet/XDDELinkResults.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDDELINKS_HPP_
#include <com/sun/star/sheet/XDDELinks.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINK_HPP_
#include <com/sun/star/sheet/XAreaLink.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINKS_HPP_
#include <com/sun/star/sheet/XAreaLinks.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif


class ScAreaLink;
class ScDocShell;
class ScTableLink;

typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XRefreshListener >* XRefreshListenerPtr;
SV_DECL_PTRARR_DEL( XRefreshListenerArr_Impl, XRefreshListenerPtr, 4, 4 )



class ScSheetLinkObj : public cppu::WeakImplHelper4<
                            com::sun::star::container::XNamed,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    String                  aFileName;
    XRefreshListenerArr_Impl aRefreshListeners;

    ScTableLink*            GetLink_Impl() const;
    void                    Refreshed_Impl();
    void                    ModifyRefreshDelay_Impl( sal_Int32 nRefresh );

public:
                            ScSheetLinkObj(ScDocShell* pDocSh, const String& rName);
    virtual                 ~ScSheetLinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // aus get/setPropertyValue gerufen:
    ::rtl::OUString         getFileName(void) const;
    void                    setFileName(const ::rtl::OUString& FileName);
    ::rtl::OUString         getFilter(void) const;
    void                    setFilter(const ::rtl::OUString& Filter);
    ::rtl::OUString         getFilterOptions(void) const;
    void                    setFilterOptions(const ::rtl::OUString& FilterOptions);
    sal_Int32               getRefreshDelay(void) const;
    void                    setRefreshDelay(sal_Int32 nRefreshDelay);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScSheetLinksObj : public cppu::WeakImplHelper4<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScSheetLinkObj*         GetObjectByIndex_Impl(INT32 nIndex);
    ScSheetLinkObj*         GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScSheetLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScSheetLinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScAreaLinkObj : public cppu::WeakImplHelper4<
                            com::sun::star::sheet::XAreaLink,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    USHORT                  nPos;
    XRefreshListenerArr_Impl aRefreshListeners;

    void    Modify_Impl( const ::rtl::OUString* pNewFile, const ::rtl::OUString* pNewFilter,
                         const ::rtl::OUString* pNewOptions, const ::rtl::OUString* pNewSource,
                         const com::sun::star::table::CellRangeAddress* pNewDest );
    void    ModifyRefreshDelay_Impl( sal_Int32 nRefresh );
    void    Refreshed_Impl();

public:
                            ScAreaLinkObj(ScDocShell* pDocSh, USHORT nP);
    virtual                 ~ScAreaLinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // aus get/setPropertyValue gerufen:
    ::rtl::OUString         getFileName(void) const;
    void                    setFileName(const ::rtl::OUString& FileName);
    ::rtl::OUString         getFilter(void) const;
    void                    setFilter(const ::rtl::OUString& Filter);
    ::rtl::OUString         getFilterOptions(void) const;
    void                    setFilterOptions(const ::rtl::OUString& FilterOptions);
    sal_Int32               getRefreshDelay(void) const;
    void                    setRefreshDelay(sal_Int32 nRefreshDelay);

                            // XAreaLink
    virtual ::rtl::OUString SAL_CALL getSourceArea() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSourceArea( const ::rtl::OUString& aSourceArea )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getDestArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDestArea( const ::com::sun::star::table::CellRangeAddress& aDestArea )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScAreaLinksObj : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XAreaLinks,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScAreaLinkObj*          GetObjectByIndex_Impl(INT32 nIndex);

public:
                            ScAreaLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScAreaLinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XAreaLinks
    virtual void SAL_CALL   insertAtPosition( const ::com::sun::star::table::CellAddress& aDestPos,
                                                const ::rtl::OUString& aFileName,
                                                const ::rtl::OUString& aSourceArea,
                                                const ::rtl::OUString& aFilter,
                                                const ::rtl::OUString& aFilterOptions )
                                            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                            throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//! order of XNamed and DDELink changed to avoid "duplicate comdat" symbols

class ScDDELinkObj : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XDDELink,
                            com::sun::star::container::XNamed,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::sheet::XDDELinkResults,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    String                  aAppl;
    String                  aTopic;
    String                  aItem;
    XRefreshListenerArr_Impl aRefreshListeners;

    void                    Refreshed_Impl();

public:
                            ScDDELinkObj(ScDocShell* pDocSh, const String& rA,
                                            const String& rT, const String& rI);
    virtual                 ~ScDDELinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDDELink
    virtual ::rtl::OUString SAL_CALL getApplication() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTopic() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getItem() throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDDELinkResults
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >
        SAL_CALL getResults(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setResults(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aResults )
        throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDDELinksObj : public cppu::WeakImplHelper4<
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::sheet::XDDELinks,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScDDELinkObj*           GetObjectByIndex_Impl(INT32 nIndex);
    ScDDELinkObj*           GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScDDELinksObj(ScDocShell* pDocSh);
    virtual                 ~ScDDELinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XDDELinks
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDDELink > SAL_CALL addDDELink(
        const ::rtl::OUString& aApplication, const ::rtl::OUString& aTopic,
        const ::rtl::OUString& aItem, ::com::sun::star::sheet::DDELinkMode nMode )
        throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};




#endif

