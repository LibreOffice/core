/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _DBAUI_INDEXFIELDSCONTROL_HXX_
#define _DBAUI_INDEXFIELDSCONTROL_HXX_

#include "moduledbu.hxx"
#include <svtools/editbrowsebox.hxx>
#include "indexcollection.hxx"
#include <com/sun/star/uno/Sequence.hxx>

//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    // IndexFieldsControl
    //==================================================================
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

        void Init(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rAvailableFields);

        void initializeFrom(const IndexFields& _rFields);
        void commitTo(IndexFields& _rFields);

        sal_Bool SaveModified();
        sal_Bool IsModified() const;

        const IndexFields&  GetSavedValue() const { return m_aSavedValue; }
        void                SaveValue() { m_aSavedValue = m_aFields; }

        void SetModifyHdl(const Link& _rHdl) { m_aModifyHdl = _rHdl; }
        Link GetModifyHdl() const { return m_aModifyHdl; }
        virtual String GetCellText(long _nRow,sal_uInt16 nColId) const;

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

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXFIELDSCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
