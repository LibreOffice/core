/*************************************************************************
 *
 *  $RCSfile: TableDesignView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:45:19 $
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
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#define DBAUI_TABLEDESIGNVIEW_HXX

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif

namespace dbaui
{
    class OTableController;
    class OTableFieldDescWin;
    class OTableDescWin;
    class OTableEditorCtrl;
    //==================================================================
    class OTableBorderWindow : public Window
    {
        Splitter                            m_aHorzSplitter;
        OTableController*                   m_pController;
        OTableFieldDescWin*                 m_pFieldDescWin;
        OTableEditorCtrl*                   m_pEditorCtrl;

        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
        void ArrangeChilds( long nSplitPos ,Rectangle& rRect);
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OTableBorderWindow(Window* pParent);
        ~OTableBorderWindow();
        // window overloads
        virtual void Resize();
        virtual void GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pEditorCtrl; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pFieldDescWin; }
    };
    //==================================================================
    class OTableDesignView : public ODataView
    {
    private:
        ::com::sun::star::lang::Locale      m_aLocale;
        OTableBorderWindow*                 m_pWin;
        OTableController*                   m_pController;

    protected:


        // return the Rectangle where I can paint myself
        virtual void resizeControl(Rectangle& rRect);

    public:
        OTableDesignView(   Window* pParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
                            OTableController* _pController);
        virtual ~OTableDesignView();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pWin->GetEditorCtrl(); }
        OTableFieldDescWin*     GetDescWin()    const { return m_pWin->GetDescWin(); }
        OTableController*       getController() const { return m_pController; }

        ::com::sun::star::lang::Locale      getLocale() const { return m_aLocale;}

        virtual sal_Bool isCutAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);

        virtual void initialize();
        void reSync(); // resync window data with realdata

        DECL_LINK( SwitchHdl, Accelerator* );
    };
}
#endif // DBAUI_TABLEDESIGNVIEW_HXX

