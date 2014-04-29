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
            indicates, if format was successful
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
