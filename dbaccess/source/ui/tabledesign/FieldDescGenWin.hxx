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
#ifndef DBAUI_TABLEFIELDDESCGENPAGE_HXX
#define DBAUI_TABLEFIELDDESCGENPAGE_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif

namespace dbaui
{
    class OTableDesignHelpBar;
    class OFieldDescription;
    class OTableFieldControl;
    class OTableEditorCtrl;
    //==================================================================
    class OFieldDescGenWin : public TabPage
                            ,public IClipboardTest
    {

        OTableFieldControl  *m_pFieldControl;
    protected:
        virtual void Resize();

    public:
        OFieldDescGenWin( Window* pParent, OTableDesignHelpBar* pHelpBar );
        virtual ~OFieldDescGenWin();

        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetControlText( sal_uInt16 nControlId, const String& rText );
        String GetControlText( sal_uInt16 nControlId );
        void SetReadOnly( sal_Bool bReadOnly );
#if OSL_DEBUG_LEVEL > 0
        OTableEditorCtrl* GetEditorCtrl();
#endif
        //  short GetFormatCategory(OFieldDescription* pFieldDescr);
            // liefert zum am Feld eingestellten Format einen der CAT_xxx-Werte (CAT_NUMBER, CAT_DATE ...)

        String BoolStringPersistent(const String& rUIString) const;
        String BoolStringUI(const String& rPersistentString) const;

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();

        inline OTableFieldControl*  getFieldControl() const { return m_pFieldControl; }
    };
}
#endif // DBAUI_TABLEFIELDDESCGENPAGE_HXX

