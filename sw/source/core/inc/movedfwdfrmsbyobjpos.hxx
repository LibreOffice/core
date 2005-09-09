/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: movedfwdfrmsbyobjpos.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:50:51 $
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
#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#define _MOVEDFWDFRMSBYOBJPOS_HXX

#include <map>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
