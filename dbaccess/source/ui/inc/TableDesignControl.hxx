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
#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#define DBAUI_TABLEDESIGNCONTROL_HXX

#include <svtools/tabbar.hxx>
#include <svtools/editbrowsebox.hxx>

#include "IClipBoardTest.hxx"
#include "TypeInfo.hxx"

namespace dbaui
{
    class OTableDesignView;

    class OTableRowView : public ::svt::EditBrowseBox, public IClipboardTest
    {
        friend class OTableDesignUndoAct;

    protected:
        long    m_nDataPos;    ///< currently needed row
        long    m_nCurrentPos; ///< current position of selected column

    private:
        sal_uInt16  m_nCurUndoActId;

    protected:
        sal_Bool    m_bCurrentModified;
        sal_Bool    m_bUpdatable;
        sal_Bool    m_bClipboardFilled;

    public:
        OTableRowView(Window* pParent);
        virtual ~OTableRowView();

        virtual void                SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo ) = 0;
        virtual void                SetCellData( long nRow, sal_uInt16 nColId, const ::com::sun::star::uno::Any& _rNewData ) = 0;
        virtual ::com::sun::star::uno::Any          GetCellData( long nRow, sal_uInt16 nColId ) = 0;
        virtual void                SetControlText( long nRow, sal_uInt16 nColId, const String& rText ) = 0;
        virtual String              GetControlText( long nRow, sal_uInt16 nColId ) = 0;

        virtual OTableDesignView* GetView() const = 0;

        sal_uInt16  GetCurUndoActId(){ return m_nCurUndoActId; }

        // IClipboardTest
        virtual void cut();
        virtual void copy();
        virtual void paste();

    protected:
        void Paste( long nRow );

        virtual void CopyRows()                             = 0;
        virtual void DeleteRows()                           = 0;
        virtual void InsertRows( long nRow )                = 0;
        virtual void InsertNewRows( long nRow )             = 0;

        virtual sal_Bool IsPrimaryKeyAllowed( long nRow )   = 0;
        virtual sal_Bool IsInsertNewAllowed( long nRow )    = 0;
        virtual sal_Bool IsDeleteAllowed( long nRow )       = 0;

        virtual sal_Bool IsUpdatable() const {return m_bUpdatable;}
        virtual void SetUpdatable( sal_Bool bUpdate=sal_True );

        virtual RowStatus GetRowStatus(long nRow) const;
        virtual void KeyInput(const KeyEvent& rEvt);
        virtual void Command( const CommandEvent& rEvt );

        virtual void Init();
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
