/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: object.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:41:30 $
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

#ifndef ARY_OBJECT_HXX
#define ARY_OBJECT_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/types.hxx>


namespace ary
{


/** Interface for every class, that is stored within the
    Autodoc Repository.
*/
class Object : public csv::ConstProcessorClient
{
  public:
    virtual             ~Object() {}

    /// @return Type id of most derived class.
    ClassId             AryClass() const;

  private:
    virtual ClassId     get_AryClass() const = 0;
};



inline ClassId
Object::AryClass() const
{
    return get_AryClass();
}


}   // namespace ary
#endif
