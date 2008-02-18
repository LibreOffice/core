/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_DataEditor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:54:20 $
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

#ifndef _SCH_DGRDATA_HXX
#define _SCH_DGRDATA_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <memory>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
    }
}}}

namespace chart
{

class DataBrowser;

class DataEditor : public ModalDialog
{
public:
    DataEditor( Window* pParent,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartDocument > & xChartDoc,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataEditor();

    // Window
    virtual void Resize();

    // Dialog
    virtual BOOL Close();

    void SetReadOnly( bool bReadOnly );
    bool ApplyChangesToModel();

private:
    bool                            m_bReadOnly;
    ::std::auto_ptr< DataBrowser >  m_apBrwData;
    ToolBox                         m_aTbxData;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ImageList       m_aToolboxImageList;
    ImageList       m_aToolboxImageListHighContrast;

    /// handles actions of the toolbox
    DECL_LINK( ToolboxHdl, void* );
    /// is called, if the cursor of the table has moved
    DECL_LINK( BrowserCursorMovedHdl, void*);
    /// this is called if MiscOptions change, esp. High-Contrast mode
    DECL_LINK( MiscHdl, void* );
    /// is called when the contents of the edit cell changes
    DECL_LINK( CellModified, void* );
    /// is called when the width of any column in the edit browsebox has changed
    DECL_LINK( BrowserColumnResized, void* );
    /// is called when the browser view was scrolled, @todo: only call on horizontal scroll
    DECL_LINK( BrowserContentScrolled, void* );

    void UpdateData();
    void ApplyImageList();
    /// moved and resizes the series name control etc. to fit the dimensions of the edit browsebox
//     void ImplAdjustHeaderControls( bool bRefreshFromModel );

    /** notifySystemWindow adds or remove the given window pToRegister at the Systemwindow found when search pWindow.
        @param  pWindow
            The window which is used to search for the SystemWindow.
        @param  pToRegister
            The window which should be added or removed on the TaskPaneList.
        @param  rMemFunc
            The member function which should be called at the SystemWindow when found.
            Possible values are:
            ::comphelper::mem_fun(&TaskPaneList::AddWindow)
            ::comphelper::mem_fun(&TaskPaneList::RemoveWindow)

        @note this code is taken from dbaccess/source/ui/inc/UITools.hxx
    */
    void notifySystemWindow( Window* pWindow,
                             Window* pToRegister,
                             ::comphelper::mem_fun1_t< TaskPaneList, Window* > rMemFunc );

    void AdaptBrowseBoxSize();
};

} // namespace chart

#endif  // _SCH_DGRDATA_HXX
