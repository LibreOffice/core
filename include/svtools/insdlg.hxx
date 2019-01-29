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
#ifndef INCLUDED_SVTOOLS_INSDLG_HXX
#define INCLUDED_SVTOOLS_INSDLG_HXX

#include <svtools/svtdllapi.h>
#include <tools/globname.hxx>
#include <sot/formats.hxx>

#include <vector>
#include <vcl/transfer.hxx>

class SvObjectServer
{
private:
    SvGlobalName    aClassName;
    OUString        aHumanName;

public:
    SvObjectServer( const SvGlobalName & rClassP, const OUString & rHumanP ) :
        aClassName( rClassP ),
        aHumanName( rHumanP ) {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const OUString &        GetHumanName() const { return aHumanName; }
};

typedef ::std::vector< SvObjectServer > SvObjectServerList_impl;

class SVT_DLLPUBLIC SvObjectServerList
{
private:
    SvObjectServerList_impl aObjectServerList;

public:
    const SvObjectServer *  Get( const OUString & rHumanName ) const;
    const SvObjectServer *  Get( const SvGlobalName & ) const;
    void                    Remove( const SvGlobalName & );
    void                    FillInsertObjects();
    size_t                  Count() const
                            {
                                return aObjectServerList.size();
                            }

    const SvObjectServer&   operator[]( size_t n ) const
                            {
                                return aObjectServerList[ n ];
                            }
};

class SVT_DLLPUBLIC SvPasteObjectHelper
{
public:
    static OUString GetSotFormatUIName( SotClipboardFormatId nId );
    static bool GetEmbeddedName(const TransferableDataHelper& rData, OUString& _rName, OUString& _rSource, SotClipboardFormatId const & _nFormat);
};

#endif // INCLUDED_SVTOOLS_INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
