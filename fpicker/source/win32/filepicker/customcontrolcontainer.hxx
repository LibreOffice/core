/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customcontrolcontainer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:43:07 $
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

#ifndef _CUSTOMCONTROLCONTAINER_HXX_
#define _CUSTOMCONTROLCONTAINER_HXX_

#ifndef _CUSTOMCONTROL_HXX_
#include "customcontrol.hxx"
#endif

#include <list>

//-----------------------------------
// A container for custom controls
// the container is resposible for
// the destruction of the custom
// controls
//-----------------------------------

class CCustomControlContainer : public CCustomControl
{
public:
    virtual ~CCustomControlContainer();

    virtual void SAL_CALL Align();
    virtual void SAL_CALL SetFont(HFONT hFont);

    virtual void SAL_CALL AddControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveAllControls();

private:
    typedef std::list<CCustomControl*> ControlContainer_t;

    ControlContainer_t  m_ControlContainer;
};

#endif
