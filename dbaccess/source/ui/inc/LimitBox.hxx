/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LIMIT_BOX_HXX
#define LIMIT_BOX_HXX

#include <vcl/field.hxx>
#include <rtl/ustring.hxx>

namespace dbaui
{

/**
 * Input box to add limit to an SQL query (maximum number of result's rows)
 * This box is reachable on the Query Design Toolbar
 */
class LimitBox: public NumericBox
{
    public:
        LimitBox( Window* pParent, WinBits nStyle );
        virtual ~LimitBox();

        virtual OUString    CreateFieldText( sal_Int64 nValue ) const;

        virtual void        Reformat();
        virtual void        ReformatAll();

        virtual Size        GetOptimalSize() const;

    private:
        void LoadDefaultLimits();
};

} ///dbaui namespace

#endif ///LIMIT_BOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
