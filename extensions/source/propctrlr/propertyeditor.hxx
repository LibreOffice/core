/*************************************************************************
 *
 *  $RCSfile: propertyeditor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-19 14:08:31 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_

#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    class IBrowserControl;
    class IPropertyLineListener;
    struct OLineDescriptor;

    //========================================================================
    //= OPropertyEditor
    //========================================================================
    class OPropertyEditor : public Control
    {
    private:
                TabControl                  m_aTabControl;
                IPropertyLineListener*      m_pListener;
                sal_uInt16                  m_nNextId;
                Link                        m_aPageActivationHandler;

    protected:
                virtual void                Resize();

    public:
                                            OPropertyEditor (Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL);
                                            OPropertyEditor (Window* pParent, const ResId& rResId);

                                            ~OPropertyEditor();

                virtual sal_uInt16          CalcVisibleLines();
                virtual void                EnableUpdate();
                virtual void                DisableUpdate();

                virtual void                SetLineListener(IPropertyLineListener *);

                virtual void                SetHelpId( sal_uInt32 nHelpId );
                virtual sal_uInt16          AppendPage( const String& r,sal_uInt32 nHelpId=0);
                virtual void                SetPage( sal_uInt16 );
                virtual void                RemovePage(sal_uInt16 nID);
                virtual sal_uInt16          GetCurPage();
                virtual void                ClearAll();
                virtual void                ClearTable();

                virtual void                SetPropertyValue(const ::rtl::OUString & rEntryName, const ::rtl::OUString & rValue );
                virtual ::rtl::OUString     GetPropertyValue(const ::rtl::OUString & rEntryName ) const;
                virtual sal_uInt16          GetPropertyPos(const ::rtl::OUString& rEntryName ) const;
                virtual void                SetPropertyData(const ::rtl::OUString& rEntryName, void* pData);
                virtual IBrowserControl*    GetPropertyControl( const ::rtl::OUString& rEntryName );

                virtual sal_uInt16          InsertEntry(const OLineDescriptor&, sal_uInt16 nPos = EDITOR_LIST_APPEND);
                virtual void                ChangeEntry(const OLineDescriptor&, sal_uInt16 nPos);
                virtual sal_uInt16          AppendEntry(const OLineDescriptor&);

                virtual void                SetFirstVisibleEntry(sal_uInt16 nPos);
                virtual sal_uInt16          GetFirstVisibleEntry();

                virtual void                SetSelectedEntry(sal_uInt16 nPos);
                virtual sal_uInt16          GetSelectedEntry();

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

    protected:
        DECL_LINK(OnPageDeactivate, TabControl*);
        DECL_LINK(OnPageActivate, TabControl*);
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/12 11:31:33  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 11:50:25  fs
 ************************************************************************/

