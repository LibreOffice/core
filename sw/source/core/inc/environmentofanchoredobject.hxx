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


#ifndef _ENVIRONMENTOFANCHOREDOBJECT_HXX
#define _ENVIRONMENTOFANCHOREDOBJECT_HXX

class SwFrm;
class SwLayoutFrm;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject
    {
        private:
            const bool mbFollowTextFlow;

        public:
            /** construtor

                OD 05.11.2003

                @author OD

                @param _bFollowTextFlow
                input parameter - indicates, if the anchored object, for which
                this environment is instantiated, follow the text flow or not
            */
            SwEnvironmentOfAnchoredObject( const bool _bFollowTextFlow );

            /** destructor

                OD 05.11.2003

                @author OD
            */
            ~SwEnvironmentOfAnchoredObject();

            /** determine environment layout frame for possible horizontal object
                positions respectively for alignment to 'page areas'

                OD 05.11.2003
                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                otherwise it's the page frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                OD 2005-01-20 #118546# - no exception any more. Thus remove
                parameter <_bForPageAlignment>

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame, at which the horizontal position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the horizontal environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetHoriEnvironmentLayoutFrm( const SwFrm& _rHoriOrientFrm ) const;

            /** determine environment layout frame for possible vertical object
                positions respectively for alignments to 'page areas'

                OD 05.11.2003
                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                - header/footer frame, if anchored inside page header/footer
                - footnote frame, if anchored inside footnote
                otherwise it's the document body frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                OD 2005-01-20 #118546# - no exception any more. Thus remove
                parameter <_bForPageAlignment>

                @author OD

                @param _rVertOrientFrm
                input parameter - frame, at which the vertical position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the vertical environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetVertEnvironmentLayoutFrm( const SwFrm& _rVertOrientFrm ) const;
    };
} // namespace objectpositioning

#endif
