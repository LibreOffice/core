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

#include "Resource.h"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include "UNOXWrapper.h"

/**
 * CAccTableCell implements the IAccessibleTableCell interface.
 */
class ATL_NO_VTABLE CAccTableCell : public CComObjectRoot,
                                    public CComCoClass<CAccTableCell, &CLSID_AccTableCell>,
                                    public IAccessibleTableCell,
                                    public CUNOXWrapper

{
public:
    CAccTableCell();
    virtual ~CAccTableCell() {}

    BEGIN_COM_MAP(CAccTableCell)
    COM_INTERFACE_ENTRY(IAccessibleTableCell)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL, SmartQI_)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    static HRESULT WINAPI SmartQI_(void* pv, REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return static_cast<CAccTableCell*>(pv)->SmartQI(iid, ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if (m_pOuterUnknown)
            return OuterQueryInterface(iid, ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    STDMETHOD(put_XInterface)(hyper pXInterface) override;

    // IAccessibleTableCell interfaces
    STDMETHOD(get_columnExtent)(long*) override;
    STDMETHOD(get_columnHeaderCells)(IUnknown***, long*) override { return E_FAIL; }
    STDMETHOD(get_columnIndex)(long*) override;
    STDMETHOD(get_rowExtent)(long*) override;
    STDMETHOD(get_rowHeaderCells)(IUnknown***, long*) override { return E_FAIL; }
    STDMETHOD(get_rowIndex)(long*) override;
    STDMETHOD(get_isSelected)(boolean*) override;
    STDMETHOD(get_rowColumnExtents)(long*, long*, long*, long*, boolean*) { return E_FAIL; }
    STDMETHOD(get_table)(IUnknown**) { return E_FAIL; }

private:
    css::uno::Reference<css::accessibility::XAccessibleTable> m_xTable;
    sal_Int32 m_nIndexInParent;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
