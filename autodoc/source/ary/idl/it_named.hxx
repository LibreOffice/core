/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_named.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:54:41 $
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

#ifndef ARY_IDL_IT_NAMED_HXX
#define ARY_IDL_IT_NAMED_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_type.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{




/** Represents types with a name - in opposite to e.g. sequences,
    which do not have one.
*/
class Named_Type : public Type
{
  public:
    // LIFECYCLE
    virtual             ~Named_Type()           {}

    // INQUIRY
    const String &      Name() const            { return sName; }

  protected:
                        Named_Type(
                            const String &      i_sName )
                                                : sName(i_sName) { }
  private:
    // DATA
    String              sName;
};



}   // namespace idl
}   // namespace ary


#endif

