/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: x_ary.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_X_ARY_HXX
#define ARY_X_ARY_HXX




// USED SERVICES
    // BASE CLASSES
#include <cosv/x.hxx>
    // OTHER



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
                            std::ostream &      o_rOutputMedium ) const;
  private:
    E_Event             eEvent;
};

// IMPLEMENTATION
inline X_Ary::E_Event
X_Ary::GetEvent() const
    { return eEvent; }




}   // namespace ary
#endif
