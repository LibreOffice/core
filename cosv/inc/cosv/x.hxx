/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:58:46 $
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

#ifndef CSV_X_HXX
#define CSV_X_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>


namespace csv
{

class Exception
{
  public:
    virtual             ~Exception() {}
    virtual void        GetInfo(
                            ostream &           o_rOutputMedium ) const = 0;
};


class X_Default : public Exception
{
  public:
                        X_Default(
                            const char *        i_sMessage )
                            :   sMessage(i_sMessage) {}
    virtual void        GetInfo(                // Implemented in comfunc.cxx
                            ostream &           o_rOutputMedium ) const;
  private:
    String              sMessage;
};


}   // namespace csv



#endif



