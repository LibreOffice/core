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
#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#define DBAUI_TABLEDESIGNCONTROL_HXX

#ifndef _TABBAR_HXX //autogen
#include <svtools/tabbar.hxx>
#endif
#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include <svtools/editbrowsebox.hxx>
#endif

#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

#define TABPAGESIZE 70
namespace dbaui
{
    class OTableDesignView;
    class OTypeInfo;
    //==================================================================
    class OTableRowView : public ::svt::EditBrowseBox
                        ,public IClipboardTest
    {
        friend class OTableDesignUndoAct;

    protected:
        long    m_nDataPos;             // derzeit benoetigte Zeile
        long    m_nCurrentPos;          // Aktuelle Position der ausgewaehlten Column
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
#endif // DBAUI_TABLEDESIGNCONTROL_HXX


