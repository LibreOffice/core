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

 #ifndef INCLUDED_SW_INC_IDOCUMENTSTATE_HXX
 #define INCLUDED_SW_INC_IDOCUMENTSTATE_HXX

 #include <tools/solar.h>

 /** Get information about the current document state
 */
 class IDocumentState
 {
 public:
    /** Must be called manually at changes of format.
    */
    virtual void SetModified() = 0;

    virtual void ResetModified() = 0;

    /** Changes of document?
    */
    virtual bool IsModified() const = 0;

    /** State of being loaded or not.
        Formerly DocInfo.
    */
    virtual bool IsLoaded() const = 0;

    virtual bool IsUpdateExpFld() const  = 0;

    virtual bool IsNewDoc() const = 0;

    virtual bool IsPageNums() const = 0;

    virtual void SetPageNums(bool b)    = 0;

    virtual void SetNewDoc(bool b) = 0;

    virtual void SetUpdateExpFldStat(bool b) = 0;

    virtual void SetLoaded(bool b = true) = 0;

 protected:
    virtual ~IDocumentState() {};
 };

 #endif // INCLUDED_SW_INC_IDOCUMENTSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
