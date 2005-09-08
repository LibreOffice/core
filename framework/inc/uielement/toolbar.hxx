/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbar.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:47:46 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________


#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_

#include <vcl/toolbox.hxx>

namespace framework
{

class ToolBarManager;
class ToolBar : public ToolBox
{
    public:
                        ToolBar( Window* pParent, WinBits nWinBits );
        virtual         ~ToolBar();

        virtual void    Command ( const CommandEvent& rCEvt );
        virtual void    StateChanged( StateChangedType nType );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        // Toolbar manager
        void            SetToolBarManager( ToolBarManager* pTbMgr );
        ToolBarManager* GetToolBarManager() const { return m_pToolBarManager; }

        // Provide additional handlers to support external implementations
        void            SetCommandHdl( const Link& aLink ) { m_aCommandHandler = aLink; }
        const Link&     GetCommandHdl() const { return m_aCommandHandler; }
        void            SetStateChangedHdl( const Link& aLink ) { m_aStateChangedHandler = aLink; }
        const Link&     GetStateChangedHdl() const { return m_aStateChangedHandler; }
        void            SetDataChangedHdl( const Link& aLink ) { m_aDataChangedHandler = aLink; }
        const Link&     GetDataChangedHdl() { return m_aDataChangedHandler; }

    private:
        Link            m_aCommandHandler;
        Link            m_aStateChangedHandler;
        Link            m_aDataChangedHandler;
        ToolBarManager* m_pToolBarManager;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_
