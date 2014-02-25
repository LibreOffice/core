/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_MOVEDFWDFRMSBYOBJPOS_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_MOVEDFWDFRMSBYOBJPOS_HXX

#include <map>
#include <sal/types.h>

class SwTxtNode;
class SwTxtFrm;
// --> #i26945#
class SwRowFrm;

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

        bool FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                  sal_uInt32& _ornToPageNum ) const;

        // --> #i26945#
        bool DoesRowContainMovedFwdFrm( const SwRowFrm& _rRowFrm ) const;

        void Clear() { maMovedFwdFrms.clear(); };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
