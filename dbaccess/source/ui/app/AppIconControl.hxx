/*************************************************************************
 *
 *  $RCSfile: AppIconControl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:29:46 $
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
 *  Contributor(s): Ocke Janssen
 *
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

