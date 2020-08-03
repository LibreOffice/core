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

#include <svtools/editbrowsebox.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include "indexes.hxx"

namespace dbaui
{

    class DbaMouseDownListBoxController;

    // IndexFieldsControl
    class IndexFieldsControl final : public ::svt::EditBrowseBox
    {
        IndexFields                 m_aSavedValue;

        IndexFields                 m_aFields;          // !! order matters !!
        IndexFields::const_iterator m_aSeekRow;         // !!

        Link<IndexFieldsControl&,void>      m_aModifyHdl;

        VclPtr< ::svt::ListBoxControl>      m_pSortingCell;
        VclPtr< ::svt::ListBoxControl>      m_pFieldNameCell;

        OUString                    m_sAscendingText;
        OUString                    m_sDescendingText;

        bool                        m_bAddIndexAppendix;

    public:
        IndexFieldsControl(const css::uno::Reference<css::awt::XWindow> &rParent);
        virtual ~IndexFieldsControl() override;
        virtual void dispose() override;

        void Init(const css::uno::Sequence< OUString >& _rAvailableFields, bool _bAddIndexAppendix);

        void initializeFrom(const IndexFields& _rFields);
        void commitTo(IndexFields& _rFields);

        bool SaveModified() override;
        using EditBrowseBox::IsModified;

        const IndexFields&  GetSavedValue() const { return m_aSavedValue; }
        void                SaveValue() { m_aSavedValue = m_aFields; }

        void SetModifyHdl(const Link<IndexFieldsControl&,void>& _rHdl) { m_aModifyHdl = _rHdl; }
        virtual OUString GetCellText(long _nRow,sal_uInt16 nColId) const override;

    private:
        // EditBrowseBox overridables
        virtual void PaintCell( OutputDevice& _rDev, const tools::Rectangle& _rRect, sal_uInt16 _nColumnId ) const override;
        virtual bool SeekRow(long nRow) override;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;
        virtual bool IsTabAllowed(bool bForward) const override;

        ::svt::CellController*  GetController(long _nRow, sal_uInt16 _nColumnId) override;
        void                InitController(::svt::CellControllerRef&, long _nRow, sal_uInt16 _nColumnId) override;

        OUString GetRowCellText(const IndexFields::const_iterator& _rRow,sal_uInt16 nColId) const;
        bool implGetFieldDesc(long _nRow, IndexFields::const_iterator& _rPos);

        bool isNewField() const { return GetCurRow() >= static_cast<sal_Int32>(m_aFields.size()); }

        DECL_LINK( OnListEntrySelected, DbaMouseDownListBoxController&, void );

        using ::svt::EditBrowseBox::Init;
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
