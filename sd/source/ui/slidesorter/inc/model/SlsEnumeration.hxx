/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsEnumeration.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-06 16:23:39 $
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

#ifndef SD_SLIDESORTER_ENUMERATION_HXX
#define SD_SLIDESORTER_ENUMERATION_HXX

#include <memory>

namespace sd { namespace slidesorter { namespace model {


/** Interface to generic enumerations.  Designed to operate on shared
    pointers.  Therefore GetNextElement() returns T and not T&.
*/
template <class T>
class Enumeration
{
public:
    virtual bool HasMoreElements (void) const = 0;
    /** Returns T instead of T& so that it can handle shared pointers.
    */
    virtual T GetNextElement (void) = 0;
    virtual void Rewind (void) = 0;
    virtual ::std::auto_ptr<Enumeration<T> > Clone (void) = 0;
};

} } } // end of namespace ::sd::slidesorter::model

#endif
