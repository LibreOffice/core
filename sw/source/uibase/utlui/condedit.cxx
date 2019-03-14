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

#include <sot/formats.hxx>

#include <condedit.hxx>
#include <svx/dbaexchange.hxx>
#include <vcl/builderfactory.hxx>

using namespace ::svx;
using namespace ::com::sun::star::uno;

ConditionEdit::ConditionEdit(vcl::Window* pParent, WinBits nStyle)
    : Edit(pParent, nStyle)
    , DropTargetHelper(this)
    , bBrackets(true)
    , bEnableDrop(true)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeConditionEdit(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    BuilderUtils::ensureDefaultWidthChars(rMap);
    rRet = VclPtr<ConditionEdit>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}

// Drop possible, respectively format known?

sal_Int8 ConditionEdit::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return OColumnTransferable::canExtractColumnDescriptor
        ( GetDataFlavorExVector(),
                                ColumnTransferFormatFlags::COLUMN_DESCRIPTOR )
                ? DND_ACTION_COPY
                : DND_ACTION_NONE;
}

sal_Int8 ConditionEdit::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if( bEnableDrop )
    {
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

        const DataFlavorExVector& rVector = aData.GetDataFlavorExVector();
        if(OColumnTransferable::canExtractColumnDescriptor(rVector, ColumnTransferFormatFlags::COLUMN_DESCRIPTOR))
        {
                ODataAccessDescriptor aColDesc = OColumnTransferable::extractColumnDescriptor(
                                                                    aData);
                OUString sDBName;
                if (bBrackets)
                    sDBName += "[";
                OUString sTmp;
                sTmp = aColDesc.getDataSource();
                sDBName += sTmp;
                sDBName += ".";

                aColDesc[DataAccessDescriptorProperty::Command] >>= sTmp;
                sDBName += sTmp;
                sDBName += ".";

                aColDesc[DataAccessDescriptorProperty::ColumnName] >>= sTmp;
                sDBName += sTmp;
                if (bBrackets)
                    sDBName += "]";

                SetText( sDBName );
                nRet = DND_ACTION_COPY;
        }
    }
    return nRet;
}

SwConditionEdit::SwConditionEdit(std::unique_ptr<weld::Entry> xControl)
    : m_xControl(std::move(xControl))
    , m_aDropTargetHelper(*this)
    , bBrackets(true)
    , bEnableDrop(true)
{
}

sal_Int8 SwConditionEditDropTarget::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return OColumnTransferable::canExtractColumnDescriptor
        ( GetDataFlavorExVector(),
                                ColumnTransferFormatFlags::COLUMN_DESCRIPTOR )
                ? DND_ACTION_COPY
                : DND_ACTION_NONE;
}

SwConditionEditDropTarget::SwConditionEditDropTarget(SwConditionEdit& rEdit)
    : DropTargetHelper(rEdit.get_widget().get_drop_target())
    , m_rEdit(rEdit)
{
}

sal_Int8 SwConditionEditDropTarget::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if (m_rEdit.GetDropEnable())
    {
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

        const DataFlavorExVector& rVector = aData.GetDataFlavorExVector();
        if (OColumnTransferable::canExtractColumnDescriptor(rVector, ColumnTransferFormatFlags::COLUMN_DESCRIPTOR))
        {
            ODataAccessDescriptor aColDesc = OColumnTransferable::extractColumnDescriptor(
                                                                aData);
            OUString sDBName;
            bool bBrackets = m_rEdit.GetBrackets();
            if (bBrackets)
                sDBName += "[";
            OUString sTmp;
            sTmp = aColDesc.getDataSource();
            sDBName += sTmp;
            sDBName += ".";

            aColDesc[DataAccessDescriptorProperty::Command] >>= sTmp;
            sDBName += sTmp;
            sDBName += ".";

            aColDesc[DataAccessDescriptorProperty::ColumnName] >>= sTmp;
            sDBName += sTmp;
            if (bBrackets)
                sDBName += "]";

            m_rEdit.get_widget().set_text( sDBName );
            nRet = DND_ACTION_COPY;
        }
    }
    return nRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
