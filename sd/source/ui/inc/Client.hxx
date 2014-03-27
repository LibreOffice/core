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

#ifndef INCLUDED_SD_SOURCE_UI_INC_CLIENT_HXX
#define INCLUDED_SD_SOURCE_UI_INC_CLIENT_HXX


#include <sfx2/ipclient.hxx>
class SdrGrafObj;
class SdrOle2Obj;
class OutlinerParaObject;

namespace sd {

class ViewShell;


class Client : public SfxInPlaceClient
{
    ViewShell*      mpViewShell;
    SdrOle2Obj*     pSdrOle2Obj;
    SdrGrafObj*     pSdrGrafObj;
    OutlinerParaObject* pOutlinerParaObj;

    virtual void    ObjectAreaChanged() SAL_OVERRIDE;
    virtual void    RequestNewObjectArea( Rectangle& ) SAL_OVERRIDE;
    virtual void    ViewChanged() SAL_OVERRIDE;
    virtual void    MakeVisible() SAL_OVERRIDE;

public:
    Client (SdrOle2Obj* pObj, ViewShell* pSdViewShell, ::Window* pWindow);
    virtual ~Client (void);

    SdrOle2Obj*     GetSdrOle2Obj() const { return pSdrOle2Obj; }
    void            SetSdrGrafObj(SdrGrafObj* pObj) { pSdrGrafObj = pObj; }
    SdrGrafObj*     GetSdrGrafObj() const { return pSdrGrafObj; }
    void            SetOutlinerParaObj (OutlinerParaObject* pObj) { pOutlinerParaObj = pObj; }
    OutlinerParaObject*     GetOutlinerParaObject () const { return pOutlinerParaObj; }
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
