/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_CUSSHOW_HXX
#define _SD_CUSSHOW_HXX

#include <tools/list.hxx>
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
class SD_DLLPUBLIC SdCustomShow : public List
{
private:
    String          aName;
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

    void   SetName(const String& rName);
    String GetName() const;

    SdDrawDocument* GetDoc() const { return pDoc; }

    void ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage );
    void RemovePage( const SdPage* pPage );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoCustomShow();
};

#endif      // _SD_CUSSHOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
