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

#ifndef _SD_STLPOOL_HXX
#define _SD_STLPOOL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase7.hxx>
#include <map>
#include <vector>

#include <stlfamily.hxx>
#include <stlsheet.hxx>

#include <sddllapi.h>

class SdStyleSheet;
class SdDrawDocument;
class SdPage;

typedef std::map< const SdPage*, SdStyleFamilyRef > SdStyleFamilyMap;

typedef ::cppu::ImplInheritanceHelper4< SfxStyleSheetPool,
                                        ::com::sun::star::lang::XServiceInfo,
                                        ::com::sun::star::container::XIndexAccess,
                                        ::com::sun::star::container::XNameAccess,
                                        ::com::sun::star::lang::XComponent > SdStyleSheetPoolBase;

class SdStyleSheetPool : public SdStyleSheetPoolBase, public SfxListener
{
    friend class SdDrawDocument;
public:
                        SdStyleSheetPool(SfxItemPool const& rPool, SdDrawDocument* pDocument);

    void                SetActualStyleSheet(SfxStyleSheetBase* pActStyleSheet)  { mpActualStyleSheet = pActStyleSheet; }
    SfxStyleSheetBase*  GetActualStyleSheet()                                   { return mpActualStyleSheet; }

    SfxStyleSheetBase*  GetTitleSheet(const String& rLayoutName);

                        // Caller muss Liste loeschen
    List*               CreateOutlineSheetList(const String& rLayoutName);

    /** creates all layout style sheets for the givin layout name if they
        don't exist yet.

        @param rLayoutName  Must be the name of a master page
        @param bCheck       If set to true, the debug version will assert if a style
                            had to be created. This is used to assert errors in documents
                            when styles are missing.
    */
    SD_DLLPUBLIC void                CreateLayoutStyleSheets(const String& rLayoutName, sal_Bool bCheck = sal_False );
    List*               CreateLayoutSheetNames(const String& rLayoutName) const;
    void                CreateLayoutSheetList(const String& rLayoutName, SdStyleSheetVector& rLayoutSheets);
    void                CopyLayoutSheets(const String& rLayoutName, SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets );
    void                CopyGraphicSheets(SdStyleSheetPool& rSourcePool);
    void                CopyCellSheets(SdStyleSheetPool& rSourcePool);
    void                CopyTableStyles(SdStyleSheetPool& rSourcePool);

    void                CreatePseudosIfNecessary();
    void                UpdateStdNames();
    static void         PutNumBulletItem( SfxStyleSheetBase* pSheet, Font& rBulletFont );
    Font                GetBulletFont() const;

    SdDrawDocument*     GetDoc() const { return mpDoc; }

    static  SdStyleSheetVector CreateChildList( SdStyleSheet* pSheet );


public:
    void throwIfDisposed() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire (void) throw ();
    virtual void SAL_CALL release (void) throw ();
protected:
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily );

    virtual SfxStyleSheetBase* Create(const String& rName, SfxStyleFamily eFamily, sal_uInt16 nMask);
    virtual SfxStyleSheetBase* Create(const SdStyleSheet& rStyle);

    using  SfxStyleSheetPool::Create;
    virtual ~SdStyleSheetPool();

    void AddStyleFamily( const SdPage* pPage );
    void RemoveStyleFamily( const SdPage* pPage );

private:
    SfxStyleSheetBase*      mpActualStyleSheet;
    SdDrawDocument*         mpDoc;
    SdStyleFamilyRef        mxGraphicFamily;
    SdStyleFamilyRef        mxCellFamily;
    SdStyleFamilyMap        maStyleFamilyMap;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > mxTableFamily;
    rtl::OUString           msTableFamilyName;
};

#endif     // _SD_STLPOOL_HXX
