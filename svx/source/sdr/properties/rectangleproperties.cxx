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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/rectangleproperties.hxx>
#include <svx/svdorect.hxx>

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
            // call parent
            TextProperties::ItemSetChanged(rSet);
        }

        // set a new StyleSheet and broadcast
        void RectangleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
