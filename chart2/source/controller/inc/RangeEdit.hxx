/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: RangeEdit.hxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2008-03-06 16:45:59 $
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
#ifndef CHART2RANGEEDIT_HXX
#define CHART2RANGEEDIT_HXX

#include <vcl/edit.hxx>
#include <vcl/accel.hxx>
#include <tools/link.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class RangeEdit : public Edit
{
public:
    explicit            RangeEdit( Window* pParent, const ResId& rResId);
    virtual             ~RangeEdit();

    // KeyInput method
    void                SetKeyInputHdl( const Link& rKeyInputLink );
    virtual void        KeyInput( const KeyEvent& rKEvt );

private:
    Link                m_aKeyInputLink;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
