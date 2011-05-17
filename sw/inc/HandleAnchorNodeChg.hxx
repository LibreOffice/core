/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _HANDLEANCHORNODECHG_HXX
#define _HANDLEANCHORNODECHG_HXX

class SwFlyFrmFmt;
class SwFmtAnchor;
class SwFlyFrm;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
