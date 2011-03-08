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
#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#define _MOVEDFWDFRMSBYOBJPOS_HXX

#include <map>
#include <sal/types.h>

class SwTxtNode;
class SwTxtFrm;
// --> #i26945#
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

        // --> #i40155#
        void Remove( const SwTxtFrm& _rTxtFrm );
        // <--

        bool FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                  sal_uInt32& _ornToPageNum ) const;

        // --> #i26945#
        bool DoesRowContainMovedFwdFrm( const SwRowFrm& _rRowFrm ) const;
        // <--

        void Clear() { maMovedFwdFrms.clear(); };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
