/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_LIMITBOX_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_LIMITBOX_HXX

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
        LimitBox( vcl::Window* pParent, WinBits nStyle );

        virtual OUString    CreateFieldText( sal_Int64 nValue ) const override;

        virtual void        Reformat() override;
        virtual void        ReformatAll() override;

        virtual Size        GetOptimalSize() const override;

    private:
        void LoadDefaultLimits();
};

} ///dbaui namespace

#endif /// INCLUDED_DBACCESS_SOURCE_UI_INC_LIMITBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
