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
#include <com/sun/star/sheet/XDataTransformations.hpp>

class ScDocument;

namespace sc {

class ExternalDataSource;

}

class ScExternalTransformationObj : public cppu::WeakImplHelper<css::sheet::XDataTransformations>
{
public:

    ScExternalTransformationObj(ScDocument* pDoc);
    virtual ~ScExternalTransformationObj();

    void SAL_CALL SetDatabase(const ::rtl::OUString& rDBName) override;
    void SAL_CALL Transform() override;
private:

    ScDocument* mpDoc;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
