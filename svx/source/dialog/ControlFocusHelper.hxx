/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlFocusHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 14:58:03 $
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
#ifndef SVX_CONTROLL_FUCUS_HELPER_HXX
#define SVX_CONTROLL_FUCUS_HELPER_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

// class SvxControlFocusHelper ---------------------------------------------

class SvxControlFocusHelper : public Control
{
private:
    Control*        m_pFocusCtrl;

public:
    SvxControlFocusHelper( Window* pParent, const ResId& rId ) :
        Control( pParent, rId ), m_pFocusCtrl( NULL ) {}

    void            SetFocusControl( Control* pCtrl ) { m_pFocusCtrl = pCtrl; }

    virtual long    Notify( NotifyEvent& rNEvt );
};

#endif  // SVX_CONTROLL_FUCUS_HELPER_HXX
