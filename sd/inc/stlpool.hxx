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

#pragma once

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/implbase.hxx>
#include <vcl/font.hxx>
#include <map>
#include <string_view>
#include <vector>

#include "stlfamily.hxx"
#include "stlsheet.hxx"

#include "sddllapi.h"

class SdDrawDocument;
class SdPage;
class SvxNumberFormat;

typedef std::map< const SdPage*, SdStyleFamilyRef > SdStyleFamilyMap;

typedef ::cppu::ImplInheritanceHelper< SfxStyleSheetPool,
                                        css::lang::XServiceInfo,
                                        css::container::XIndexAccess,
                                        css::container::XNameAccess,
                                        css::lang::XComponent > SdStyleSheetPoolBase;

class SAL_DLLPUBLIC_RTTI SdStyleSheetPool final : public SdStyleSheetPoolBase, public SfxListener
{
    friend class SdDrawDocument;
public:
                        SdStyleSheetPool(SfxItemPool const& rPool, SdDrawDocument* pDocument);

    void                SetActualStyleSheet(SfxStyleSheetBase* pActStyleSheet)  { mpActualStyleSheet = pActStyleSheet; }
    SfxStyleSheetBase*  GetActualStyleSheet()                                   { return mpActualStyleSheet; }

    SfxStyleSheetBase*  GetTitleSheet(std::u16string_view rLayoutName);

                        // Caller has to delete the list
    void                CreateOutlineSheetList(std::u16string_view rLayoutName, std::vector<SfxStyleSheetBase*> &rOutlineStyles);

    /** creates all layout style sheets for the given layout name if they
        don't exist yet.

        @param rLayoutName  Must be the name of a master page
        @param bCheck       If set to true, the debug version will assert if a style
                            had to be created. This is used to assert errors in documents
                            when styles are missing.
    */
    SD_DLLPUBLIC void                CreateLayoutStyleSheets(std::u16string_view rLayoutName, bool bCheck = false );
    static void         CreateLayoutSheetNames(std::u16string_view rLayoutName, std::vector<OUString> &aNameList);
    void                CreateLayoutSheetList(std::u16string_view rLayoutName, SdStyleSheetVector& rLayoutSheets);
    void                CopyLayoutSheets(std::u16string_view rLayoutName, SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets);
    void                CopyGraphicSheets(SdStyleSheetPool& rSourcePool);
    void                CopyCellSheets(SdStyleSheetPool& rSourcePool);
    void                CopyTableStyles(SdStyleSheetPool const & rSourcePool);
    void                CopyCellSheets(SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets);
    void                RenameAndCopyGraphicSheets(SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets, std::u16string_view rRenameSuffix);

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
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, StyleSheetCopyResultVector& rCreatedSheets );
    void CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, StyleSheetCopyResultVector& rCreatedSheets, std::u16string_view rRenameSuffix );

    virtual rtl::Reference<SfxStyleSheetBase> Create(const OUString& rName, SfxStyleFamily eFamily, SfxStyleSearchBits nMask) override;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
