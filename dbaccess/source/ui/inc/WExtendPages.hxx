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
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#define DBAUI_WIZ_EXTENDPAGES_HXX

#include "WTypeSelect.hxx"

class SvStream;
namespace dbaui
{
    // Wizard Page: OWizHTMLExtend
    class OWizHTMLExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser*   createReader(sal_Int32 _nRows);
    public:
        OWizHTMLExtend(Window* pParent, SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizHTMLExtend( _pParent, _rInput ); }

        virtual ~OWizHTMLExtend(){}
    };
    // Wizard Page: OWizRTFExtend
    class OWizRTFExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizRTFExtend(Window* pParent,SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizRTFExtend( _pParent, _rInput ); }

        virtual ~OWizRTFExtend(){}
    };

    // Wizard Page: OWizNormalExtend
    class OWizNormalExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizNormalExtend(Window* pParent);
    };
}
#endif // DBAUI_WIZ_EXTENDPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
