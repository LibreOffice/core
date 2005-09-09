/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectformatterlayfrm.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:21:20 $
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
#ifndef _OBJECTFORMATTERLAYFRM_HXX
#define _OBJECTFORMATTERLAYFRM_HXX

#ifndef _OBJECTFORMATTER_HXX
#include <objectformatter.hxx>
#endif

class SwLayoutFrm;

// -----------------------------------------------------------------------------
// Format floating screen objects, which are anchored at a given anchor text frame
// and registered at the given page frame.
// -----------------------------------------------------------------------------
class SwObjectFormatterLayFrm : public SwObjectFormatter
{
    private:
        // anchor layout frame
        SwLayoutFrm& mrAnchorLayFrm;

        SwObjectFormatterLayFrm( SwLayoutFrm& _rAnchorLayFrm,
                                 const SwPageFrm& _rPageFrm,
                                 SwLayAction* _pLayAction );

        /** method to format all anchored objects, which are registered at
            the page frame, whose 'anchor' isn't on this page frame and whose
            anchor frame is valid.

            OD 2004-07-02 #i28701#

            @author OD

            @return boolean
            indicates, if format was successfull
        */
        bool _AdditionalFormatObjsOnPage();

    protected:

        virtual SwFrm& GetAnchorFrm();

    public:
        virtual ~SwObjectFormatterLayFrm();

        // --> OD 2005-01-10 #i40147# - add parameter <_bCheckForMovedFwd>.
        // Not relevant for objects anchored at layout frame.
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false );
        // <--
        virtual bool DoFormatObjs();

        static SwObjectFormatterLayFrm* CreateObjFormatter(
                                                SwLayoutFrm& _rAnchorLayFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction );
};

#endif
