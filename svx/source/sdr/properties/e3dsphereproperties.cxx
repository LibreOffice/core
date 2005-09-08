/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dsphereproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:13:48 $
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

#ifndef _SDR_PROPERTIES_E3DSPHEREPROPERTIES_HXX
#include <svx/sdr/properties/e3dsphereproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _E3D_SPHERE3D_HXX
#include <sphere3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        E3dSphereProperties::E3dSphereProperties(SdrObject& rObj)
        :   E3dCompoundProperties(rObj)
        {
        }

        E3dSphereProperties::E3dSphereProperties(const E3dSphereProperties& rProps, SdrObject& rObj)
        :   E3dCompoundProperties(rProps, rObj)
        {
        }

        E3dSphereProperties::~E3dSphereProperties()
        {
        }

        BaseProperties& E3dSphereProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dSphereProperties(*this, rObj));
        }

        void E3dSphereProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            // call parent
            E3dCompoundProperties::PostItemChange(nWhich);

            // handle value change
            E3dSphereObj& rObj = (E3dSphereObj&)GetSdrObject();

            switch(nWhich)
            {
                case SDRATTR_3DOBJ_HORZ_SEGS:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_VERT_SEGS:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
