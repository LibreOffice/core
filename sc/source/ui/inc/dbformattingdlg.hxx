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
#ifndef SC_DBFORMATTINGDLG_HXX
#define SC_DBFORMATTINGDLG_HXX

#include "anyrefdg.hxx"
#include "dbdata.hxx"

class ScDbFormattingDlg : public ModalDialog
{
public:
    ScDbFormattingDlg( Window* pParent );
    ~ScDbFormattingDlg();
    virtual void SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool IsRefInputMode() const;
    virtual void SetActive();
    virtual sal_Bool Close();
};


#endif
