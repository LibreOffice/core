/*************************************************************************
 *
 *  $RCSfile: IDocumentDrawModelAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:12:14 $
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

#ifndef IDOCUMENTDRAWMODEL_HXX_INCLUDED
#define IDOCUMENTDRAWMODEL_HXX_INCLUDED

#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx>
#endif

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
