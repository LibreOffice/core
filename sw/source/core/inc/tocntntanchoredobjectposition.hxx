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


#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#define _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>

class SwFrm;
class SwTxtFrm;
class SwLayoutFrm;
class SwRect;

namespace objectpositioning
{
    class SwToCntntAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position
            const SwLayoutFrm* mpVertPosOrientFrm;
            // --> OD 2004-06-17 #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrmAnchorPos;

            // data for calculation of position
            bool          mbAnchorToChar;
            const SwFrm*  mpToCharOrientFrm;
            const SwRect* mpToCharRect;
            SwTwips       mnToCharTopOfLine;

            virtual bool IsAnchoredToChar() const;
            virtual const SwFrm* ToCharOrientFrm() const;
            virtual const SwRect* ToCharRect() const;
            // OD 12.11.2003 #i22341#
            virtual SwTwips ToCharTopOfLine() const;

            // method to cast <SwAnchoredObjectPosition::GetAnchorFrm()> to
            // the needed type
            SwTxtFrm&       GetAnchorTxtFrm() const;

        // *********************************************************************
            /** determine frame for horizontal position

                OD 04.08.2003
                if the given proposed frame is a content frame, the proposed
                frame is returned.
                otherwise (given proposed frame is a layout frame),
                the lower content frames of the proposed frame are checked
                for the first, that the anchor or a follow of the anchor.
                If none is found, the proposed frame is returned.

                @author OD

                @param _pProposedFrm
                input parameter - proposed frame for horizontal position

                @return constant reference to <SwFrm> object, at which the
                horizontal position is determined.
            */
            const SwFrm& _GetHoriVirtualAnchor( const SwLayoutFrm& _pProposedFrm ) const;

        public:
            SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwToCntntAnchoredObjectPosition();

            /** calculate position of object

                @author OD
            */
            virtual void CalcPosition();

            /** frame, at which the vertical position is oriented at

                @author OD
            */
            const SwLayoutFrm& GetVertPosOrientFrm() const;
    };
} // namespace objectpositioning

#endif
