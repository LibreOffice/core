/*************************************************************************
 *
 *  $RCSfile: QueryTextView.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:10:01 $
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
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#define DBAUI_QUERYVIEW_TEXT_HXX

#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif


class Splitter;

namespace dbaui
{
    // tempoaray class until the beamer is implemented
    class OBeamer : public Window
    {
    public:
        OBeamer(Window* _pParent) : Window(_pParent){}
    };

    class OQueryViewSwitch;
    class OQueryContainerWindow : public Window
    {
        OQueryViewSwitch*   m_pView;
        OBeamer*            m_pBeamer;
        Splitter*           m_pSplitter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xBeamer;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >        m_xMe;              // our own UNO representation

        DECL_LINK( SplitHdl, void* );
    public:
        OQueryContainerWindow(Window* pParent, OQueryController* _pController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        ~OQueryContainerWindow();

        virtual void        Resize();
        void showBeamer(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
        void hideBeamer();
        void initialize(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
        OQueryViewSwitch* getView() { return m_pView; }
        void switchView();
    };
    // end of temp classes

    class OSqlEdit;
    class OQueryTextView : public Window
    {
        friend class OQueryViewSwitch;
        OSqlEdit*   m_pEdit;
        ToolBox*    m_pToolBox; // the toolbox is owned by OQueryDesignView
    public:
        OQueryTextView(Window* pParent,ToolBox* _pToolBox);
        virtual ~OQueryTextView();

        virtual sal_Bool isCutAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement);
        virtual ::rtl::OUString getStatement();
        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);
    protected:
        virtual void Resize();
    };
}
#endif // DBAUI_QUERYVIEW_TEXT_HXX


