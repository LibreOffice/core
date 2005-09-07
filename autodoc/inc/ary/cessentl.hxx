/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cessentl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:50:24 $
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

#ifndef ARY_CESSENTL_HXX
#define ARY_CESSENTL_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/ids.hxx>
#include <ary/docu.hxx>

namespace udm
{
class Agent;
}




namespace ary
{


class CeEssentials
{   // Non inline functions are implemented in ceworker.cxx .
  public:
    // LIFECYCLE
                        CeEssentials();
                        CeEssentials(
                            Cid                 i_nId,
                            const udmstri &     i_sLocalName,
                            Cid                 i_nOwner,
                            Lid                 i_nLocation );
                        ~CeEssentials();
    // INQUIRY
    Cid                 Id() const;
    const udmstri &     LocalName() const;
    Cid                 Owner() const;
    Lid                 Location() const;
    const ary::Documentation &
                        Info() const;

    // ACCESS
    void                SetInfo(
                            DYN ary::Documentation &
                                                let_drInfo )
                                                { pInfo = &let_drInfo; }
  private:
    String              sLocalName;
    Cid                 nId;
    Cid                 nOwner;
    Lid                 nLocation;
    Dyn<ary::Documentation>
                        pInfo;
};



inline Cid
CeEssentials::Id() const
    { return nId; }
inline const udmstri    &
CeEssentials::LocalName() const
    { return sLocalName; }
inline Cid
CeEssentials::Owner() const
    { return nOwner; }
inline Lid
CeEssentials::Location() const
    { return nLocation; }
inline const ary::Documentation &
CeEssentials::Info() const
    { return pInfo ? *pInfo : ary::Documentation::Null_(); }


}   // namespace ary


#endif


