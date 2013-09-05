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

#ifndef _SD_CUSSHOW_HXX
#define _SD_CUSSHOW_HXX

#include <vector>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

class SdDrawDocument;
class SdPage;

/*************************************************************************
|*
|* CustomShow
|*
\************************************************************************/
class SD_DLLPUBLIC SdCustomShow
{
public:
    typedef ::std::vector<const SdPage*> PageVec;

private:
    PageVec       maPages;
    OUString        aName;
    SdDrawDocument* pDoc;

    // this is a weak reference to a possible living api wrapper for this custom show
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoCustomShow;

    // forbidden and not implemented
    SdCustomShow();

public:
    // single argument ctors shall be explicit
    explicit SdCustomShow(SdDrawDocument* pDrawDoc);
    SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow );

    virtual ~SdCustomShow();

    // @@@ copy ctor, but no copy assignment? @@@
    SdCustomShow( const SdCustomShow& rShow );

    /** Provides a direct access to the collection of the SdPage objects. */
    PageVec& PagesVector();
    /**
     * Replaces all occurences of pOldPage with pNewPage.
     * If pNewPage is 0 then removes all occurences of pOldPage.
     */
    void ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage );
    /** Removes all occurences of pPage. */
    void RemovePage( const SdPage* pPage );

    void   SetName(const OUString& rName);
    OUString GetName() const;

    SdDrawDocument* GetDoc() const { return pDoc; }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoCustomShow();
};

#endif      // _SD_CUSSHOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
