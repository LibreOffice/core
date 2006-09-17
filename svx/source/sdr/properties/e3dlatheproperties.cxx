/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dlatheproperties.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:41:52 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_PROPERTIES_E3DLATHEPROPERTIES_HXX
#include <svx/sdr/properties/e3dlatheproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _E3D_LATHE3D_HXX
#include <lathe3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        E3dLatheProperties::E3dLatheProperties(SdrObject& rObj)
        :   E3dCompoundProperties(rObj)
        {
        }

        E3dLatheProperties::E3dLatheProperties(const E3dLatheProperties& rProps, SdrObject& rObj)
        :   E3dCompoundProperties(rProps, rObj)
        {
        }

        E3dLatheProperties::~E3dLatheProperties()
        {
        }

        BaseProperties& E3dLatheProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dLatheProperties(*this, rObj));
        }

        void E3dLatheProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            // call parent
            E3dCompoundProperties::PostItemChange(nWhich);

            // handle value change
            E3dLatheObj& rObj = (E3dLatheObj&)GetSdrObject();

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
                case SDRATTR_3DOBJ_PERCENT_DIAGONAL:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_BACKSCALE:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_END_ANGLE:
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
