/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_INC_STLPOOL_HXX
#define INCLUDED_SD_INC_STLPOOL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase.hxx>
#include <map>
#include <vector>

#include "stlfamily.hxx"
#include "stlsheet.hxx"

#include "sddllapi.h"

class SdStyleSheet;
class SdDrawDocument;
class SdPage;
class SfxStyleSheetBase;
class SvxNumberFormat;

typedef std::map< const SdPage*, SdStyleFamilyRef > SdStyleFamilyMap;

typedef ::cppu::ImplInheritanceHelper< SfxStyleSheetPool,
                                        css::lang::XServiceInfo,
                                        css::container::XIndexAccess,
                                        css::container::XNameAccess,
                                        css::lang::XComponent > SdStyleSheetPoolBase;

class SdStyleSheetPool final : public SdStyleSheetPoolBase, public SfxListener
{
    friend class SdDrawDocument;
public:
                        SdStyleSheetPool(SfxItemPool const& rPool, SdDrawDocument* pDocument);

    void                SetActualStyleSheet(SfxStyleSheetBase* pActStyleSheet)  { mpActualStyleSheet = pActStyleSheet; }
    SfxStyleSheetBase*  GetActualStyleSheet()                                   { return mpActualStyleSheet; }

    SfxStyleSheetBase*  GetTitleSheet(const OUString& rLayoutName);

                        // Caller has to delete the list
    void                CreateOutlineSheetList(const OUString& rLayoutName, std::vector<SfxStyleSheetBase*> &rOutlineStyles);

    /** creates all layout style sheets for the given layout name if they
        don't exist yet.

        @param rLayoutName  Must be the name of a master page
        @param bCheck       If set to true, the debug version will assert if a style
                            had to be created. This is used to assert errors in documents
                            when styles are missing.
    */
    SD_DLLPUBLIC void                CreateLayoutStyleSheets(const OUString& rLayoutName, bool bCheck = false );
    static void         CreateLayoutSheetNames(const OUString& rLayoutName, std::vector<OUString> &aNameList);
    void                CreateLayoutSheetList(const OUString& rLayoutName, SdStyleSheetVector& rLayoutSheets);
    void                CopyLayoutSheets(const OUString& rLayoutName, SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets );
    void                CopyGraphicSheets(SdStyleSheetPool& rSourcePool);
    void                CopyCellSheets(SdStyleSheetPool& rSourcePool);
    void                CopyTableStyles(SdStyleSheetPool const & rSourcePool);
    void                CopyCellSheets(SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets);
    void                RenameAndCopyGraphicSheets(SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets, OUString const &rRenameSuffix);

    void                CreatePseudosIfNecessary();
    void                UpdateStdNames();
    static void         PutNumBulletItem( SfxStyleSheetBase* pSheet, vcl::Font& rBulletFont );
    static vcl::Font    GetBulletFont();

    SdDrawDocument*     GetDoc() const { return mpDoc; }

    static  SdStyleSheetVector CreateChildList( SdStyleSheet const * pSheet );

    static void setDefaultOutlineNumberFormatBulletAndIndent(sal_uInt16 i, SvxNumberFormat &rNumberFormat);

public:
    /// @throws css::uno::RuntimeException
    void throwIfDisposed();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

private:
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily );
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, SdStyleSheetVector& rCreatedSheets );
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, SdStyleSheetVector& rCreatedSheets, const OUString &rRenameSuffix );

    virtual SfxStyleSheetBase* Create(const OUString& rName, SfxStyleFamily eFamily, sal_uInt16 nMask) override;

    using  SfxStyleSheetPool::Create;
    virtual ~SdStyleSheetPool() override;

    void AddStyleFamily( const SdPage* pPage );
    void RemoveStyleFamily( const SdPage* pPage );

    SfxStyleSheetBase*      mpActualStyleSheet;
    SdDrawDocument*         mpDoc;
    SdStyleFamilyRef        mxGraphicFamily;
    SdStyleFamilyRef        mxCellFamily;
    SdStyleFamilyMap        maStyleFamilyMap;
    css::uno::Reference< css::container::XNameAccess > mxTableFamily;
    OUString                msTableFamilyName;
};

#endif // INCLUDED_SD_INC_STLPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
