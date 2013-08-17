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

#ifndef _DBAUI_INDEXFIELDSCONTROL_HXX_
#define _DBAUI_INDEXFIELDSCONTROL_HXX_

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
        ConstIndexFieldsIterator    m_aSeekRow;         // !!

        Link                        m_aModifyHdl;

        ::svt::ListBoxControl*      m_pSortingCell;
        ::svt::ListBoxControl*      m_pFieldNameCell;

        String                      m_sAscendingText;
        String                      m_sDescendingText;

        sal_Int32                   m_nMaxColumnsInIndex;
        sal_Bool                    m_bAddIndexAppendix;

    public:
        IndexFieldsControl( Window* _pParent, const ResId& _rId ,sal_Int32 _nMaxColumnsInIndex,sal_Bool _bAddIndexAppendix);
        ~IndexFieldsControl();

        void Init(const ::com::sun::star::uno::Sequence< OUString >& _rAvailableFields);

        void initializeFrom(const IndexFields& _rFields);
        void commitTo(IndexFields& _rFields);

        sal_Bool SaveModified();
        sal_Bool IsModified() const;

        const IndexFields&  GetSavedValue() const { return m_aSavedValue; }
        void                SaveValue() { m_aSavedValue = m_aFields; }

        void SetModifyHdl(const Link& _rHdl) { m_aModifyHdl = _rHdl; }
        Link GetModifyHdl() const { return m_aModifyHdl; }
        virtual OUString GetCellText(long _nRow,sal_uInt16 nColId) const;

    protected:
        // EditBrowseBox overridables
        virtual void PaintCell( OutputDevice& _rDev, const Rectangle& _rRect, sal_uInt16 _nColumnId ) const;
        virtual sal_Bool SeekRow(long nRow);
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);
        virtual sal_Bool IsTabAllowed(sal_Bool bForward) const;

        ::svt::CellController*  GetController(long _nRow, sal_uInt16 _nColumnId);
        void                InitController(::svt::CellControllerRef&, long _nRow, sal_uInt16 _nColumnId);

    protected:
        String GetRowCellText(const ConstIndexFieldsIterator& _rRow,sal_uInt16 nColId) const;
        sal_Bool implGetFieldDesc(long _nRow, ConstIndexFieldsIterator& _rPos);

        sal_Bool isNewField() const { return GetCurRow() >= (sal_Int32)m_aFields.size(); }

        DECL_LINK( OnListEntrySelected, ListBox* );

    private:
        using ::svt::EditBrowseBox::Init;
    };

}   // namespace dbaui

#endif // _DBAUI_INDEXFIELDSCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
