/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IDocumentDrawModelAccess.hxx,v $
 * $Revision: 1.4 $
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

#ifndef IDOCUMENTDRAWMODEL_HXX_INCLUDED
#define IDOCUMENTDRAWMODEL_HXX_INCLUDED

#include <svx/svdtypes.hxx>

class SdrModel;
class SdrPageView;

/** IDocumentDrawModelAccess
 */
class IDocumentDrawModelAccess
{
public:

    /** Draw Model and id accessors
     */
    virtual const SdrModel* GetDrawModel() const = 0;
    virtual SdrModel* GetDrawModel() = 0;
    virtual SdrModel* _MakeDrawModel() = 0;
    virtual SdrModel* GetOrCreateDrawModel() = 0;
    virtual SdrLayerID GetHeavenId() const = 0;
    virtual SdrLayerID GetHellId() const = 0;
    virtual SdrLayerID GetControlsId() const = 0;
    virtual SdrLayerID GetInvisibleHeavenId() const = 0;
    virtual SdrLayerID GetInvisibleHellId() const = 0;
    virtual SdrLayerID GetInvisibleControlsId() const = 0;

    /** method to notify drawing page view about the invisible layers
        OD 26.06.2003 #108784#
        @author OD
    */
    virtual void NotifyInvisibleLayers( SdrPageView& _rSdrPageView ) = 0;

    /** method to determine, if a layer ID belongs to the visible ones.
        OD 25.06.2003 #108784#
        Note: If given layer ID is unknown, method asserts and returns <false>.
        @author OD

        @param _nLayerId
        input parameter - layer ID, which has to be checked, if it belongs to
        the visible ones.

        @return bool, indicating, if given layer ID belongs to the visible ones.
    */
    virtual bool IsVisibleLayerId( const SdrLayerID& _nLayerId ) const = 0;

    /** method to determine, if the corresponding visible layer ID for a invisible one.

        OD 25.06.2003 #108784#
        Note: If given layer ID is a visible one, method returns given layer ID.
        Note: If given layer ID is unknown, method returns given layer ID.

        @author OD

        @param _nInvisibleLayerId
        input parameter - invisible layer ID for which the corresponding
        visible one has to be returned.

        @return sal_Int8, visible layer ID corresponding to given layer ID
    */
    virtual SdrLayerID GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId ) = 0;

    /** method to determine, if the corresponding invisible layer ID for a visible one.

        OD 25.06.2003 #108784#
        Note: If given layer ID is a invisible one, method returns given layer ID.
        Note: If given layer ID is unknown, method returns given layer ID.

        @author OD

        @param _nVisibleLayerId
        input parameter - visible layer ID for which the corresponding
        invisible one has to be returned.

        @return sal_Int8, invisible layer ID corresponding to given layer ID
    */
    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId ) = 0;

protected:

    virtual ~IDocumentDrawModelAccess() {};
};

#endif
