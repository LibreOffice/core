/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_ary.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:56:27 $
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

#ifndef ARY_X_ARY_HXX
#define ARY_X_ARY_HXX

//  VERSION:            Autodoc 2.2


// USED SERVICES
    // BASE CLASSES
#include <cosv/x.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{

class X_Ary : public csv::Exception
{
  public:
    enum E_Event
    {
        x_Any,
        x_MultipleRepository,
        x_MissingRepository,
        x_EntityNotFound,
        x_ConflictingNames,
        x_InvalidId,
        x_UnexpectedTypeOfObject,
        x_InvalidCast,
        x_CorruptData
    };

    // LIFECYCLE
                        X_Ary(
                            E_Event             i_eEvent );
    // INQUIRY
    E_Event             GetEvent() const;
    virtual void        GetInfo(
                            ostream &           o_rOutputMedium ) const;
  private:
    E_Event             eEvent;
};

// IMPLEMENTATION
inline X_Ary::E_Event
X_Ary::GetEvent() const
    { return eEvent; }


}   // namespace ary


#endif

