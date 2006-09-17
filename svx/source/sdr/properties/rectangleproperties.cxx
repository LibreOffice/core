/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rectangleproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:44:30 $
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

#ifndef _SDR_PROPERTIES_RECTANGLEPROPERTIES_HXX
#include <svx/sdr/properties/rectangleproperties.hxx>
#endif

#ifndef _SVDORECT_HXX
#include <svdorect.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        RectangleProperties::RectangleProperties(SdrObject& rObj)
        :   TextProperties(rObj)
        {
        }

        RectangleProperties::RectangleProperties(const RectangleProperties& rProps, SdrObject& rObj)
        :   TextProperties(rProps, rObj)
        {
        }

        RectangleProperties::~RectangleProperties()
        {
        }

        BaseProperties& RectangleProperties::Clone(SdrObject& rObj) const
        {
            return *(new RectangleProperties(*this, rObj));
        }

        void RectangleProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrRectObj& rObj = (SdrRectObj&)GetSdrObject();

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.SetXPolyDirty();
        }

        // set a new StyleSheet and broadcast
        void RectangleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrRectObj& rObj = (SdrRectObj&)GetSdrObject();

            // call parent
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.SetXPolyDirty();
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
