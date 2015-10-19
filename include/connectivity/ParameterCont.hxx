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
#ifndef INCLUDED_CONNECTIVITY_PARAMETERCONT_HXX
#define INCLUDED_CONNECTIVITY_PARAMETERCONT_HXX

#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace dbtools
{

    //= OParameterContinuation

    class OOO_DLLPUBLIC_DBTOOLS OParameterContinuation : public comphelper::OInteraction< css::sdb::XInteractionSupplyParameters >
    {
        css::uno::Sequence< css::beans::PropertyValue >       m_aValues;

    public:
        OParameterContinuation() { }

        css::uno::Sequence< css::beans::PropertyValue >   getValues() const { return m_aValues; }

        // XInteractionSupplyParameters
        virtual void SAL_CALL setParameters( const css::uno::Sequence< css::beans::PropertyValue >& _rValues ) throw(css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~OParameterContinuation() { }
    private:
        OParameterContinuation(const OParameterContinuation&) = delete;
        void operator =(const OParameterContinuation&) = delete;
    };
} // dbtools
#endif // INCLUDED_CONNECTIVITY_PARAMETERCONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
