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


#ifndef _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#define _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>

namespace objectpositioning
{
    class SwToLayoutAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position type TO_LAYOUT
            Point       maRelPos;

            // --> OD 2004-06-17 #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrmAnchorPos;

        public:
            SwToLayoutAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwToLayoutAnchoredObjectPosition();

            /** calculate position for object

                OD 30.07.2003 #110978#

                @author OD
            */
            virtual void CalcPosition();

            /** calculated relative position for object

                @author OD
            */
            Point GetRelPos() const;
    };
} // namespace objectpositioning

#endif
