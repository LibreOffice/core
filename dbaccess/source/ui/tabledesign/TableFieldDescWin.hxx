/*************************************************************************
 *
 *  $RCSfile: TableFieldDescWin.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:26:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEFIELDDESCRIPTION_HXX
#define DBAUI_TABLEFIELDDESCRIPTION_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBAUI_TABLEFIELDDESCGENPAGE_HXX
#include "FieldDescGenWin.hxx"
#endif

class FixedText;
namespace dbaui
{
    class OFieldDescGenWin;
    class OTableDesignHelpBar;
    class OFieldPropTabCtrl;
    class OFieldDescription;
    //==================================================================
    // Ableitung von TabPage ist ein Trick von TH,
    // um Aenderungen der Systemfarben zu bemerken (Bug #53905)
    class OTableFieldDescWin : public TabPage
    {
    private:
        OTableDesignHelpBar*    m_pHelpBar;
        OFieldDescGenWin*       m_pGenPage;
        OFieldPropTabCtrl*      m_pTabControl;
        FixedText*              m_pHeader;

    protected:
        virtual void Resize();
        virtual void Paint( const Rectangle& rRect );

    public:
        OTableFieldDescWin( Window* pParent);
        virtual ~OTableFieldDescWin();

        virtual void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetReadOnly( BOOL bReadOnly );

        BOOL ChildHasFocus(){ return m_pGenPage->HasChildPathFocus(); }
        virtual void GetFocus(){ m_pGenPage->GetFocus(); }
        virtual void LoseFocus(){ m_pGenPage->LoseFocus(); }
        void SetControlText( USHORT nControlId, const String& rText )
                { m_pGenPage->SetControlText(nControlId,rText); }
        String GetControlText( USHORT nControlId )
                { return m_pGenPage->GetControlText(nControlId); }

        //  short GetFormatCategory(OFieldDescription* pFieldDescr) { return m_pGenPage ? m_pGenPage->GetFormatCategory(pFieldDescr) : -1; }
            // liefert zum am Feld eingestellten Format einen der CAT_xxx-Werte (CAT_NUMBER, CAT_DATE ...)
        void ActivatePropertyField(USHORT nVirtualField) { if (m_pGenPage) m_pGenPage->ActivatePropertyField(nVirtualField); }
            // Parameter ist einer der FIELD_PROPERTY_xxx-Werte, das entsprechende Control wird aktiviert, wenn vorhanden

        String  BoolStringPersistent(const String& rUIString) const { return m_pGenPage->BoolStringPersistent(rUIString); }
        String  BoolStringUI(const String& rPersistentString) const { return m_pGenPage->BoolStringUI(rPersistentString); }
    };
}
#endif // DBAUI_TABLEFIELDDESCRIPTION_HXX




