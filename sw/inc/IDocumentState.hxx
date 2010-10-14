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

 #ifndef IDOCUMENTSTATE_HXX_INCLUDED
 #define IDOCUMENTSTATE_HXX_INCLUDED

 #include <tools/solar.h>

 /** Get information about the current document state
 */
 class IDocumentState
 {
 public:
       /** Bei Formataenderungen muss das zu Fuss gerufen werden!
    */
    virtual void SetModified() = 0;

    /**
    */
    virtual void ResetModified() = 0;

    /** Dokumentaenderungen?
    */
    virtual bool IsModified() const = 0;

    /** Zustaende ueber Ladezustand
        frueher DocInfo
    */
    virtual bool IsLoaded() const = 0;

    /**
    */
    virtual bool IsUpdateExpFld() const  = 0;

    /**
    */
    virtual bool IsNewDoc() const = 0;

    /**
    */
    virtual bool IsPageNums() const = 0;

    /**
    */
    virtual void SetPageNums(bool b)    = 0;

    /**
    */
    virtual void SetNewDoc(bool b) = 0;

    /**
    */
    virtual void SetUpdateExpFldStat(bool b) = 0;

    /**
    */
    virtual void SetLoaded(bool b = sal_True) = 0;

 protected:
    virtual ~IDocumentState() {};
 };

 #endif // IDOCUMENTSTATE_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
