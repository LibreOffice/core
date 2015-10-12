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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSTATEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSTATEMANAGER_HXX

#include <IDocumentState.hxx>
#include <boost/noncopyable.hpp>

class SwDoc;


namespace sw {

class DocumentStateManager : public IDocumentState,
                             public ::boost::noncopyable
{

public:
    DocumentStateManager( SwDoc& i_rSwdoc );

    void SetModified() override;
    void ResetModified() override;
    bool IsModified() const override;
    bool IsInCallModified() const override;
    bool IsUpdateExpField() const override;
    bool IsNewDoc() const override;
    void SetNewDoc(bool b) override;
    void SetUpdateExpFieldStat(bool b) override;
    void SetLoaded(bool b) override;

private:
    SwDoc& m_rDoc;

    bool mbModified      ;    //< TRUE: document has changed.
    bool mbLoaded        ;    //< TRUE: Doc loaded.
    bool mbUpdateExpField  ;    //< TRUE: Update expression fields.
    bool mbNewDoc        ;    //< TRUE: new Doc.
    bool mbInCallModified;    //< TRUE: in Set/Reset-Modified link.
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
