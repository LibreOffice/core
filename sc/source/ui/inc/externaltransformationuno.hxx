/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_EXTERNALTRANSFORMATION_HXX
#define INCLUDED_SC_INC_EXTERNALTRANSFORMATION_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/sheet/XDataTransformation.hpp>

class ScDocShell;

namespace sc
{
class ExternalDataSource;
}

class ScExternalTransformationObj : public cppu::WeakImplHelper<css::sheet::XDataTransformation>
{
public:
    ScExternalTransformationObj(ScDocShell* pDocShell, SCTAB nTab);
    virtual ~ScExternalTransformationObj() override;

    // XDataTransformation
    virtual void SAL_CALL Transform() override;

private:
    SCTAB const mnTab;
    ScDocShell* mpDocShell;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
