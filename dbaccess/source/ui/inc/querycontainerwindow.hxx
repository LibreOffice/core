/*************************************************************************
 *
 *  $RCSfile: querycontainerwindow.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:01:24 $
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

#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#define DBAUI_QUERYCONTAINERWINDOW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef _SV_DOCKWIN_HXX
#include <vcl/dockwin.hxx>
#endif

class FixedLine;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= OBeamer
    //=====================================================================
    // tempoaray class until the beamer is implemented
    class OBeamer : public DockingWindow
    {
    public:
        OBeamer(Window* _pParent) : DockingWindow(_pParent,0){}
    };

    //=====================================================================
    //= OQueryContainerWindow
    //=====================================================================
    class OQueryContainerWindow : public ODataView
    {
        OQueryViewSwitch*   m_pViewSwitch;
        OBeamer*            m_pBeamer;
        Splitter*           m_pSplitter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xBeamer;

        DECL_LINK( SplitHdl, void* );
    public:
        OQueryContainerWindow(Window* pParent, OQueryController* _pController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        ~OQueryContainerWindow();

        virtual void Construct();

        virtual long        PreNotify( NotifyEvent& rNEvt );

        // show the beamer
        void    showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
            // called when the beamer has been disposed
        void    disposingPreview();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                getPreviewFrame() const { return m_xBeamer; }

        void initialize(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);

        OQueryDesignView*   getDesignView() { return m_pViewSwitch->getDesignView(); }

        sal_Bool isCutAllowed()     { return m_pViewSwitch->isCutAllowed(); }
        sal_Bool isPasteAllowed()   { return m_pViewSwitch->isPasteAllowed(); }
        sal_Bool isCopyAllowed()    { return m_pViewSwitch->isCopyAllowed(); }
        void copy()                 { m_pViewSwitch->copy(); }
        void cut()                  { m_pViewSwitch->cut(); }
        void paste()                { m_pViewSwitch->paste(); }

        void clear()                                                        { m_pViewSwitch->clear(); }
        sal_Bool isSlotEnabled( sal_Int32 _nSlotId )                        { return m_pViewSwitch->isSlotEnabled( _nSlotId ); }
        void     setSlotEnabled( sal_Int32 _nSlotId, sal_Bool _bEnable )    { m_pViewSwitch->setSlotEnabled( _nSlotId, _bEnable ); }
        void     setNoneVisbleRow(sal_Int32 _nRows)                         { m_pViewSwitch->setNoneVisbleRow( _nRows); }

        void setReadOnly( sal_Bool _bReadOnly )                             { m_pViewSwitch->setReadOnly( _bReadOnly ); }

        sal_Bool checkStatement()                                           { return m_pViewSwitch->checkStatement( ); }
        ::rtl::OUString getStatement()                                      { return m_pViewSwitch->getStatement( ); }
        void setStatement( const ::rtl::OUString& _rsStatement )            { m_pViewSwitch->setStatement( _rsStatement ); }

        void initialize()                                                   { m_pViewSwitch->initialize(); }
        void SaveUIConfig()                                                 { m_pViewSwitch->SaveUIConfig(); }
        void reset()                                                        { m_pViewSwitch->reset(); }

        sal_Bool switchView();
        virtual void GetFocus();

    protected:
        // re-arrange the controls belonging to the document itself
        virtual void resizeAll( const Rectangle& _rPlayground );

        // arrange dericed classes controls in the rectangle given
        virtual void resizeDocumentView(Rectangle& _rPlayground);
    };
    // end of temp classes

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_QUERYCONTAINERWINDOW_HXX

