/*************************************************************************
 *
 *  $RCSfile: objectformatterlayfrm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:11:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj );
        virtual bool DoFormatObjs();

        static SwObjectFormatterLayFrm* CreateObjFormatter(
                                                SwLayoutFrm& _rAnchorLayFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction );
};

#endif
