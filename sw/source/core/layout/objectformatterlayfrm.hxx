/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _OBJECTFORMATTERLAYFRM_HXX
#define _OBJECTFORMATTERLAYFRM_HXX

#include <objectformatter.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
