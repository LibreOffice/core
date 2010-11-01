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
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#define DBAUI_WIZ_EXTENDPAGES_HXX

#include "WTypeSelect.hxx"

class SvStream;
namespace dbaui
{
    // ========================================================
    // Wizard Page: OWizHTMLExtend
    // ========================================================
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
    // ========================================================
    // Wizard Page: OWizRTFExtend
    // ========================================================
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

    // ========================================================
    // Wizard Page: OWizNormalExtend
    // ========================================================
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
