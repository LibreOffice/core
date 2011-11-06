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


#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#define _MOVEDFWDFRMSBYOBJPOS_HXX

#include <map>
#include <sal/types.h>

class SwTxtNode;
class SwTxtFrm;
// --> OD 2004-10-05 #i26945#
class SwRowFrm;
// <--

typedef std::map< const SwTxtNode*, const sal_uInt32 > NodeMap;
typedef std::map< const SwTxtNode*, const sal_uInt32 >::const_iterator NodeMapIter;
typedef NodeMap::value_type NodeMapEntry;


class SwMovedFwdFrmsByObjPos
{
    private:
        NodeMap maMovedFwdFrms;

    public:
        SwMovedFwdFrmsByObjPos();
        ~SwMovedFwdFrmsByObjPos();

        void Insert( const SwTxtFrm& _rMovedFwdFrmByObjPos,
                     const sal_uInt32 _nToPageNum );

        // --> OD 2005-01-12 #i40155#
        void Remove( const SwTxtFrm& _rTxtFrm );
        // <--

        bool FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                  sal_uInt32& _ornToPageNum ) const;

        // --> OD 2004-10-05 #i26945#
        bool DoesRowContainMovedFwdFrm( const SwRowFrm& _rRowFrm ) const;
        // <--

        void Clear() { maMovedFwdFrms.clear(); };
};

#endif
