/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HandleAnchorNodeChg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-21 15:27:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
