/*************************************************************************
 *
 *  $RCSfile: brwview.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-19 12:43:13 $
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

#ifndef _SBX_BRWVIEW_HXX
#define _SBX_BRWVIEW_HXX

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _TOOLS_RESID_HXX //autogen wg. ResId
#include <tools/resid.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif

// =========================================================================
class ResMgr;
class Splitter;

namespace dbaui
{
    class DBTreeListModel;
    class DBTreeView;
    class SbaGridControl;

    class UnoDataBrowserView : public ODataView
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >                 m_xGrid;            // our grid's UNO representation
        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        SbaGridControl*         m_pVclControl;  // our grid's VCL representation

        DECL_LINK( SplitHdl, void* );
    // attribute access
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >             getGridControl() const  { return m_xGrid; }
        SbaGridControl*         getVclControl() const   { return m_pVclControl; }

    public:
        UnoDataBrowserView(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~UnoDataBrowserView();

        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);

        /** as columns may be hidden there is a difference between a columns model pos and its view pos
            so we you may use these translation function
        */
        sal_uInt16 Model2ViewPos(sal_uInt16 nPos) const;
        sal_uInt16 View2ModelPos(sal_uInt16 nPos) const;
        /// for the same reason the view column count isn't the same as the model column count
        sal_uInt16 ViewColumnCount() const;

        void setSplitter(Splitter* _pSplitter);
        void setTreeView(DBTreeView* _pTreeView);

        virtual long PreNotify( NotifyEvent& rNEvt );
    protected:
        virtual void GetFocus();
        virtual void resizeControl(Rectangle& rRect);
    };
}
#endif // _SBX_BRWVIEW_HXX

