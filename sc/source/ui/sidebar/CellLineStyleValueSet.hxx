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
#ifndef SC_SIDEBAR_CELL_LINE_STYLE_VALUE_SET_HXX
#define SC_SIDEBAR_CELL_LINE_STYLE_VALUE_SET_HXX

#include <svtools/valueset.hxx>
#include <vcl/image.hxx>

#define CELL_LINE_STYLE_ENTRIES 9

namespace sc { namespace sidebar {

class CellLineStyleValueSet : public ValueSet
{
private:
    VirtualDevice*  pVDev;
    sal_uInt16      nSelItem;
    OUString        maStrUnit[CELL_LINE_STYLE_ENTRIES];
    Image           imgCus;
    bool            bCusEnable;
public:
    CellLineStyleValueSet( Window* pParent, const ResId& rResId);
    virtual ~CellLineStyleValueSet();

    void SetUnit(const OUString* str);
    void SetSelItem(sal_uInt16 nSel);
    void SetImage(Image img);
    virtual void UserDraw( const UserDrawEvent& rUDEvt );
};

} } // end of namespace svx::sidebar

#endif // SC_SIDEBAR_CELL_LINE_STYLE_VALUE_SET_HXX

// eof
