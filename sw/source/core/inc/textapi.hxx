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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_TEXTAPI_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TEXTAPI_HXX

#include <editeng/unoedsrc.hxx>
#include <editeng/unotext.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoforou.hxx>

class SwDoc;

struct SwTextAPIEditSource_Impl;
class SwTextAPIEditSource : public SvxEditSource
{
    SwTextAPIEditSource_Impl* pImpl;

    virtual SvxEditSource*      Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;
    explicit            SwTextAPIEditSource( const SwTextAPIEditSource& rSource );

public:
                        SwTextAPIEditSource(SwDoc* pDoc);
    virtual             ~SwTextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
    void                SetString( const OUString& rText );
    OutlinerParaObject* CreateText();
    OUString            GetText();
};

class SwTextAPIObject : public SvxUnoText
{
    SwTextAPIEditSource* pSource;
public:
                        SwTextAPIObject( SwTextAPIEditSource* p);
    virtual             ~SwTextAPIObject() throw();
    void                DisposeEditSource() { pSource->Dispose(); }
    OutlinerParaObject* CreateText() { return pSource->CreateText(); }
    void                SetString( const OUString& rText ) { pSource->SetString( rText ); }
    void                SetText( OutlinerParaObject& rText ) { pSource->SetText( rText ); }
    OUString            GetText() { return pSource->GetText(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
