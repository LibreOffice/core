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


#ifndef _HANDLEANCHORNODECHG_HXX
#define _HANDLEANCHORNODECHG_HXX

class SwFlyFrmFmt;
class SwFmtAnchor;
class SwFlyFrm;

// OD 2006-02-28 #125892#
// helper class to track change of anchor node of at-paragraph respectively
// at-character anchored fly frames
// if such a change happens, it has to be checked, if the count of the anchor
// frames also change. if yes, a re-creation of the fly frames is needed:
// - deletion of existing fly frames before the intrinsic anchor node changes
// - creation of new fly frames after the intrinsic anchor node change.
class SwHandleAnchorNodeChg
{
public:
    /** checks, if re-creation of fly frames for an anchor node change at the
        given fly frame format is necessary, and performs the first part.

        @author OD

        @param _rFlyFrmFmt
        reference to the fly frame format instance, which is handled.

        @param _rNewAnchorFmt
        new anchor attribute, which will be applied at the given fly frame format

        @param _pKeepThisFlyFrm
        optional parameter - pointer to a fly frame of the given fly frame format,
        which isn't deleted, if re-creation of fly frames is necessary.
    */
    SwHandleAnchorNodeChg( SwFlyFrmFmt& _rFlyFrmFmt,
                           const SwFmtAnchor& _rNewAnchorFmt,
                           SwFlyFrm* _pKeepThisFlyFrm = 0L );

    /** calls <SwFlyFrmFmt::MakeFrms>, if re-creation of fly frames is necessary.

        @author OD
    */
    ~SwHandleAnchorNodeChg();

private:
    // fly frame format, which is tracked for a anchor node change.
    SwFlyFrmFmt& mrFlyFrmFmt;
    // internal flag, which indicates that the certain anchor node change occurs
    // and that re-creation of fly frames is necessary.
    bool mbAnchorNodeChanged;

    // no copy-constructor
    SwHandleAnchorNodeChg( const SwHandleAnchorNodeChg& );
    // no assignment-operator
    void operator=( const SwHandleAnchorNodeChg );
};
#endif
