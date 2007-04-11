/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urlcontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:42:04 $
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

#ifndef SVTOOLS_URLCONTROL_HXX
#define SVTOOLS_URLCONTROL_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileURLControl
    //=====================================================================
    class SVT_DLLPUBLIC OFileURLControl : public SvtURLBox
    {
    protected:
        String      m_sPreservedText;

    public:
        OFileURLControl(Window* _pParent);
        OFileURLControl(Window* _pParent, const ResId& _rId);

    protected:
        virtual long        PreNotify( NotifyEvent& rNEvt );
        virtual long        Notify( NotifyEvent& rNEvt );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_URLCONTROL_HXX

