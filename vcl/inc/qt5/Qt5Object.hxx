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

#pragma once

#include <salobj.hxx>
#include <vcl/sysdata.hxx>

#include <memory>

#include <QtGui/QRegion>

class QWidget;
class Qt5Frame;

class Qt5Object : public SalObject
{
    SystemEnvData m_aSystemData;
    std::unique_ptr<QWidget> m_pQWidget;
    Qt5Frame* m_pParent;
    QRegion m_pRegion;

public:
    Qt5Object(Qt5Frame* pParent, bool bShow);
    virtual ~Qt5Object() override;

    virtual void ResetClipRegion() override;
    virtual void BeginSetClipRegion(sal_uLong nRects) override;
    virtual void UnionClipRegion(long nX, long nY, long nWidth, long nHeight) override;
    virtual void EndSetClipRegion() override;

    virtual void SetPosSize(long nX, long nY, long nWidth, long nHeight) override;
    virtual void Show(bool bVisible) override;

    virtual void SetForwardKey(bool bEnable) override;

    virtual const SystemEnvData* GetSystemData() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
