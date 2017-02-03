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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_CUSTOMCONTROLCONTAINER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_CUSTOMCONTROLCONTAINER_HXX

#include "customcontrol.hxx"

#include <list>


// A container for custom controls
// the container is responsible for
// the destruction of the custom
// controls


class CCustomControlContainer : public CCustomControl
{
public:
    virtual ~CCustomControlContainer() override;

    virtual void SAL_CALL Align() override;
    virtual void SAL_CALL SetFont(HFONT hFont) override;

    virtual void SAL_CALL AddControl(CCustomControl* aCustomControl) override;
    virtual void SAL_CALL RemoveControl(CCustomControl* aCustomControl) override;
    virtual void SAL_CALL RemoveAllControls() override;

private:
    typedef std::list<CCustomControl*> ControlContainer_t;

    ControlContainer_t  m_ControlContainer;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
