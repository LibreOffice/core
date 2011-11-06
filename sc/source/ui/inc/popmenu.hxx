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



#ifndef SC_POPMENU_HXX
#define SC_POPMENU_HXX

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include "scdllapi.h"

class SC_DLLPUBLIC ScPopupMenu : public PopupMenu
{
private:
    sal_uInt16  nSel;
    sal_Bool    bHit;
protected:
    virtual void Select();
public:
    ScPopupMenu() : nSel(0),bHit(sal_False) {}
    ScPopupMenu(const ResId& rRes) : PopupMenu(rRes),nSel(0),bHit(sal_False) {}
    sal_uInt16  GetSelected() const { return nSel; }
    sal_Bool    WasHit() const      { return bHit; }
};



#endif

