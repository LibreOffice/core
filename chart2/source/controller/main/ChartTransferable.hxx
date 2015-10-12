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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTTRANSFERABLE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTTRANSFERABLE_HXX

#include <svtools/transfer.hxx>

namespace com { namespace sun { namespace star {
namespace graphic {
    class XGraphic;
}
}}}

class SdrModel;
class SdrObject;

namespace chart
{

class ChartTransferable : public TransferableHelper
{
public:
    explicit ChartTransferable( SdrModel* pDrawModel, SdrObject* pSelectedObj, bool bDrawing = false );
    virtual ~ChartTransferable();

protected:

    // implementation of TransferableHelper methods
    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) override;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xMetaFileGraphic;
    SdrModel* m_pMarkedObjModel;
    bool m_bDrawing;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTTRANSFERABLE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
