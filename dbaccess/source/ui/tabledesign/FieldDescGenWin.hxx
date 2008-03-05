/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FieldDescGenWin.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:06:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        void SetControlText( USHORT nControlId, const String& rText );
        String GetControlText( USHORT nControlId );
        void SetReadOnly( BOOL bReadOnly );
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

