/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namesort.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:41:19 $
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

#ifndef ARY_NAMESORT_HXX
#define ARY_NAMESORT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace ary
{

/** Provides sensible sorting of ASCII names in programming languages.

    @descr
    Names are compared case insensitive first. Only after they appear
    equal that way, there is an additional case sensitive comparison.
    The second comparison sorts upper case before lower case.

*/
struct LesserName
{
    bool                operator()(
                            const String &      i_s1,
                            const String &      i_s2 ) const;
  private:
    // DATA

    static const csv::CharOrder_Table
                        aOrdering1_;
    static const csv::CharOrder_Table
                        aOrdering2_;
};

inline bool
LesserName::operator()( const String &      i_s1,
                        const String &      i_s2 ) const
{
    int result = i_s1.compare(aOrdering1_,i_s2);
    if (result == 0)
        result = i_s1.compare(aOrdering2_,i_s2);
    return result < 0;
}


}   // namespace ary
#endif
