/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ary_disp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:37:04 $
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

#ifndef ARY_ARY_DISP_HXX
#define ARY_ARY_DISP_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/types.hxx>
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
}

class Display : public csv::ProcessorIfc
{
  public:
    virtual             ~Display() {}

    // OPERATIONS
    void                StartSlot();
    void                FinishSlot();

    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_Rid() is called
        with just the id as parameter.
    */
    void                DisplaySlot_Rid(
                            ary::Rid            i_nId );
//    void                DisplaySlot_Lid(
//                          ary::Lid            i_nId );
    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_LocalCe() is called
        with just the id as parameter.
    */
    void                DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &     i_sName );
    // INQUIRY
    const cpp::Gate *   Get_ReFinder() const;

  private:
    virtual void        do_StartSlot();
    virtual void        do_FinishSlot();


    virtual void        do_DisplaySlot_Rid(
                            ary::Rid            i_nId );
    virtual void        do_DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &     i_sName );
    virtual const cpp::Gate *
                        inq_Get_ReFinder() const = 0;
};


// IMPLEMENTATION


inline void
Display::StartSlot()
    { do_StartSlot(); }
inline void
Display::FinishSlot()
    { do_FinishSlot(); }
inline const cpp::Gate *
Display::Get_ReFinder() const
    { return inq_Get_ReFinder(); }




} // namespace ary
#endif
