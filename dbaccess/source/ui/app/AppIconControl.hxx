/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppIconControl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:21:37 $
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
#ifndef DBAUI_APPICONCONTROL_HXX
#define DBAUI_APPICONCONTROL_HXX

#ifndef _ICNVW_HXX
#include <svtools/ivctrl.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif


namespace dbaui
{
    class IControlActionListener;
    //==================================================================
    class OApplicationIconControl   :public SvtIconChoiceCtrl
                                    ,public DropTargetHelper
    {
        Point                       m_aMousePos;
        IControlActionListener*     m_pActionListener;

    public:
        OApplicationIconControl(Window* _pParent);
        virtual ~OApplicationIconControl();

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        IControlActionListener* getControlActionListener( ) const { return m_pActionListener; }

    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );
    };
}
#endif // DBAUI_APPICONCONTROL_HXX

