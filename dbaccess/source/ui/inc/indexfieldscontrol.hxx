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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXFIELDSCONTROL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXFIELDSCONTROL_HXX

#include "moduledbu.hxx"
#include <svtools/editbrowsebox.hxx>
#include "indexcollection.hxx"
#include <com/sun/star/uno/Sequence.hxx>

namespace dbaui
{

    // IndexFieldsControl
    class IndexFieldsControl : public ::svt::EditBrowseBox
    {
        OModuleClient        m_aModuleClient;
    protected:
        IndexFields                 m_aSavedValue;

        IndexFields                 m_aFields;          // !! order matters !!
        IndexFields::const_iterator m_aSeekRow;         // !!

        Link<>                      m_aModifyHdl;

        VclPtr< ::svt::ListBoxControl>      m_pSortingCell;
        VclPtr< ::svt::ListBoxControl>      m_pFieldNameCell;

        OUString                    m_sAscendingText;
        OUString                    m_sDescendingText;

        sal_Int32                   m_nMaxColumnsInIndex;
        bool                        m_bAddIndexAppendix;

    public:
        IndexFieldsControl( vcl::Window* _pParent, WinBits nWinStyle);
        virtual ~IndexFieldsControl();
        virtual void dispose() override;

        void Init(const css::uno::Sequence< OUString >& _rAvailableFields, sal_Int32 _nMaxColumnsInIndex,bool _bAddIndexAppendix);

        void initializeFrom(const IndexFields& _rFields);
        void commitTo(IndexFields& _rFields);

        bool SaveModified() override;
        bool IsModified() const override;

        const IndexFields&  GetSavedValue() const { return m_aSavedValue; }
        void                SaveValue() { m_aSavedValue = m_aFields; }

        void SetModifyHdl(const Link<>& _rHdl) { m_aModifyHdl = _rHdl; }
        virtual OUString GetCellText(long _nRow,sal_uInt16 nColId) const override;

    protected:
        // EditBrowseBox overridables
        virtual void PaintCell( OutputDevice& _rDev, const Rectangle& _rRect, sal_uInt16 _nColumnId ) const override;
        virtual bool SeekRow(long nRow) override;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;
        virtual bool IsTabAllowed(bool bForward) const override;

        ::svt::CellController*  GetController(long _nRow, sal_uInt16 _nColumnId) override;
        void                InitController(::svt::CellControllerRef&, long _nRow, sal_uInt16 _nColumnId) override;

    protected:
        OUString GetRowCellText(const IndexFields::const_iterator& _rRow,sal_uInt16 nColId) const;
        bool implGetFieldDesc(long _nRow, IndexFields::const_iterator& _rPos);

        bool isNewField() const { return GetCurRow() >= (sal_Int32)m_aFields.size(); }

        DECL_LINK( OnListEntrySelected, void* );

    private:
        using ::svt::EditBrowseBox::Init;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXFIELDSCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
