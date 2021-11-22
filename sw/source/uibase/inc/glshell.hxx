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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLSHELL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLSHELL_HXX
#include <wdocsh.hxx>

class SwGlosDocShell final : public SwDocShell
{
    OUString          m_aLongName;
    OUString          m_aShortName;
    OUString          m_aGroupName;

    virtual bool Save() override;

public:
    SFX_DECL_INTERFACE(SW_GLOSDOCSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SwGlosDocShell( bool bNewShow);
    virtual ~SwGlosDocShell() override;

    void            Execute( SfxRequest& );
    void            GetState( SfxItemSet& );
    void            SetLongName( const OUString& rLongName )
                        { m_aLongName = rLongName; }
    void            SetShortName( const OUString& rShortName )
                        { m_aShortName = rShortName; }
    void            SetGroupName( const OUString& rGroupName )
                        { m_aGroupName = rGroupName; }
};

class SwWebGlosDocShell final : public SwWebDocShell
{
    OUString          m_aLongName;
    OUString          m_aShortName;
    OUString          m_aGroupName;

    virtual bool Save() override;

public:
    SFX_DECL_INTERFACE(SW_WEBGLOSDOCSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SwWebGlosDocShell();
    virtual ~SwWebGlosDocShell() override;

    void            Execute( SfxRequest& );
    void            GetState( SfxItemSet& );
    void            SetLongName( const OUString& rLongName )
                        { m_aLongName = rLongName; }
    void            SetShortName( const OUString& rShortName )
                        { m_aShortName = rShortName; }
    void            SetGroupName( const OUString& rGroupName )
                        { m_aGroupName = rGroupName; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
