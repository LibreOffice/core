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
#ifndef SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX
#define SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX

#include <svtools/valueset.hxx>
#include <vcl/image.hxx>

namespace svx { namespace sidebar {

class LineWidthValueSet
    : public ValueSet
{
public:
    LineWidthValueSet (Window* pParent, const ResId& rResId);
    virtual ~LineWidthValueSet (void);

    void SetUnit(OUString* str);
    void SetSelItem(sal_uInt16 nSel);
    sal_uInt16 GetSelItem();
    void SetImage(Image img);
    void SetCusEnable(bool bEnable);

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

protected:
    VirtualDevice*  pVDev;
    sal_uInt16          nSelItem;
    OUString*      strUnit;
    Image           imgCus;
    bool            bCusEnable;
};

} } // end of namespace svx::sidebar

#endif // SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX

// eof
