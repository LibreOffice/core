/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef IDOCUMENTDRAWMODEL_HXX_INCLUDED
#define IDOCUMENTDRAWMODEL_HXX_INCLUDED

#include <svx/svdtypes.hxx>

class SwDrawModel;
class SdrPageView;

/** IDocumentDrawModelAccess
 */
class IDocumentDrawModelAccess
{
public:

    /** Draw Model and id accessors
     */
    virtual const SwDrawModel* GetDrawModel() const = 0;
    virtual SwDrawModel* GetDrawModel() = 0;
    virtual SwDrawModel* _MakeDrawModel() = 0;
    virtual SwDrawModel* GetOrCreateDrawModel() = 0;
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
