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
#include <editeng/outlobj.hxx>

class SwDoc;

struct SwTextAPIEditSource_Impl;
class SwTextAPIEditSource : public SvxEditSource
{
    SwTextAPIEditSource_Impl* m_pImpl;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;
    explicit            SwTextAPIEditSource( const SwTextAPIEditSource& rSource );

public:
                        SwTextAPIEditSource(SwDoc* pDoc);
    virtual             ~SwTextAPIEditSource() override;

    void                Dispose();
    void                SetText( OutlinerParaObject const & rText );
    void                SetString( const OUString& rText );
    std::unique_ptr<OutlinerParaObject> CreateText();
    OUString            GetText() const;
};

class SwTextAPIObject : public SvxUnoText
{
    std::unique_ptr<SwTextAPIEditSource> m_pSource;
public:
                        SwTextAPIObject( std::unique_ptr<SwTextAPIEditSource> p);
    virtual             ~SwTextAPIObject() throw() override;
    void                DisposeEditSource() { m_pSource->Dispose(); }
    std::unique_ptr<OutlinerParaObject> CreateText() { return m_pSource->CreateText(); }
    void                SetString( const OUString& rText ) { m_pSource->SetString( rText ); }
    void                SetText( OutlinerParaObject const & rText ) { m_pSource->SetText( rText ); }
    OUString            GetText() const { return m_pSource->GetText(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
