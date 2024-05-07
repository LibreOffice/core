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

#include <memory>

#include <pastedlg.hxx>
#include <svtools/insdlg.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <tools/lineend.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

SvPasteObjectDialog::SvPasteObjectDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/pastespecial.ui"_ustr, u"PasteSpecialDialog"_ustr)
    , m_xFtObjectSource(m_xBuilder->weld_label(u"source"_ustr))
    , m_xLbInsertList(m_xBuilder->weld_tree_view(u"list"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xLbInsertList->set_size_request(m_xLbInsertList->get_approximate_digit_width() * 40,
                                      m_xLbInsertList->get_height_rows(6));
    m_xOKButton->set_sensitive(false);

    ObjectLB().connect_changed(LINK(this, SvPasteObjectDialog, SelectHdl));
    ObjectLB().connect_row_activated(LINK( this, SvPasteObjectDialog, DoubleClickHdl));
}

void SvPasteObjectDialog::SelectObject()
{
    if (m_xLbInsertList->n_children())
    {
        m_xLbInsertList->select(0);
        SelectHdl(*m_xLbInsertList);
    }
}

IMPL_LINK_NOARG(SvPasteObjectDialog, SelectHdl, weld::TreeView&, void)
{
    if (!m_xOKButton->get_sensitive())
        m_xOKButton->set_sensitive(true);
}

IMPL_LINK_NOARG(SvPasteObjectDialog, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

/*************************************************************************
|*    SvPasteObjectDialog::Insert()
*************************************************************************/
void SvPasteObjectDialog::Insert( SotClipboardFormatId nFormat, const OUString& rFormatName )
{
    aSupplementMap.insert( std::make_pair( nFormat, rFormatName ) );
}

void SvPasteObjectDialog::InsertUno(const OUString& sCmd, const OUString& sLabel)
{
    aExtraCommand.first = sCmd;
    aExtraCommand.second = sLabel;
}


void SvPasteObjectDialog::PreGetFormat( const TransferableDataHelper &rHelper )
{
    //TODO/LATER: why is the Descriptor never used?!
    TransferableObjectDescriptor aDesc;
    if (rHelper.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        (void)rHelper.GetTransferableObjectDescriptor(
                                SotClipboardFormatId::OBJECTDESCRIPTOR, aDesc);
    }
    const DataFlavorExVector* pFormats = &rHelper.GetDataFlavorExVector();

    // create and fill dialog box
    OUString aSourceName, aTypeName;
    SvGlobalName aEmptyNm;

    //ObjectLB().SetUpdateMode( false );
    ObjectLB().freeze();

    DataFlavorExVector::iterator aIter( const_cast<DataFlavorExVector&>(*pFormats).begin() ),
                                 aEnd( const_cast<DataFlavorExVector&>(*pFormats).end() );
    while( aIter != aEnd )
    {
        SotClipboardFormatId nFormat = (*aIter++).mnSotId;

        std::map< SotClipboardFormatId, OUString >::iterator itName =
            aSupplementMap.find( nFormat );

        // if there is an "Embed Source" or and "Embedded Object" on the
        // Clipboard we read the Description and the Source of this object
        // from an accompanied "Object Descriptor" format on the clipboard
        // Remember: these formats mostly appear together on the clipboard
        OUString aName;
        const OUString* pName = nullptr;
        if ( itName == aSupplementMap.end() )
        {
            SvPasteObjectHelper::GetEmbeddedName(rHelper,aName,aSourceName,nFormat);
            if ( !aName.isEmpty() )
                pName = &aName;
        }
        else
        {
            pName = &(itName->second);
        }

        if( pName )
        {
            aName = *pName;

            if( SotClipboardFormatId::EMBED_SOURCE == nFormat )
            {
                if( aDesc.maClassName != aEmptyNm )
                {
                    aSourceName = aDesc.maDisplayName;

                    if( aDesc.maClassName == aObjClassName )
                        aName = aObjName;
                    else
                        aName = aTypeName = aDesc.maTypeName;
                }
            }
            else if( SotClipboardFormatId::LINK_SOURCE == nFormat )
            {
                continue;
            }
            else if( aName.isEmpty() )
                aName = SvPasteObjectHelper::GetSotFormatUIName( nFormat );

            // Show RICHTEXT only in case RTF is not present.
            if (nFormat == SotClipboardFormatId::RICHTEXT &&
                std::any_of(pFormats->begin(), pFormats->end(),
                            [](const DataFlavorEx& rFlavor) {
                                return rFlavor.mnSotId == SotClipboardFormatId::RTF;
                            }))
            {
                continue;
            }

            if (ObjectLB().find_text(aName) == -1)
            {
                ObjectLB().append(OUString::number(static_cast<sal_uInt32>(nFormat)), aName);
            }
        }
    }

    if( aTypeName.isEmpty() && aSourceName.isEmpty() )
    {
        if( aDesc.maClassName != aEmptyNm )
        {
            aSourceName = aDesc.maDisplayName;
            aTypeName = aDesc.maTypeName;
        }

        if( aTypeName.isEmpty() && aSourceName.isEmpty() )
        {
            // global resource from svtools (former so3 resource)
            aSourceName = SvtResId(STR_UNKNOWN_SOURCE);
        }
    }

    ObjectLB().thaw();
    SelectObject();

    if( !aSourceName.isEmpty() )
    {
        if( !aTypeName.isEmpty() )
            aTypeName += "\n";

        aTypeName += aSourceName;
        aTypeName = convertLineEnd(aTypeName, GetSystemLineEnd());
    }

    m_xFtObjectSource->set_label(aTypeName);
}

SotClipboardFormatId SvPasteObjectDialog::GetFormatOnly()
{
    return static_cast<SotClipboardFormatId>(ObjectLB().get_selected_id().toUInt32());
}

SotClipboardFormatId SvPasteObjectDialog::GetFormat( const TransferableDataHelper& rHelper)
{
    //TODO/LATER: why is the Descriptor never used?!
    TransferableObjectDescriptor aDesc;
    if (rHelper.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        (void)rHelper.GetTransferableObjectDescriptor(
                                SotClipboardFormatId::OBJECTDESCRIPTOR, aDesc);
    }
    const DataFlavorExVector* pFormats = &rHelper.GetDataFlavorExVector();

    // create and fill dialog box
    OUString aSourceName, aTypeName;
    SotClipboardFormatId nSelFormat = SotClipboardFormatId::NONE;
    SvGlobalName aEmptyNm;

    ObjectLB().freeze();

    for (auto const& format : *pFormats)
    {
        SotClipboardFormatId nFormat = format.mnSotId;

        std::map< SotClipboardFormatId, OUString >::iterator itName =
            aSupplementMap.find( nFormat );

        // if there is an "Embed Source" or and "Embedded Object" on the
        // Clipboard we read the Description and the Source of this object
        // from an accompanied "Object Descriptor" format on the clipboard
        // Remember: these formats mostly appear together on the clipboard
        OUString aName;
        const OUString* pName = nullptr;
        if ( itName == aSupplementMap.end() )
        {
            SvPasteObjectHelper::GetEmbeddedName(rHelper,aName,aSourceName,nFormat);
            if ( !aName.isEmpty() )
                pName = &aName;
        }
        else
        {
            pName = &(itName->second);
        }

        if( pName )
        {
            aName = *pName;

            if( SotClipboardFormatId::EMBED_SOURCE == nFormat )
            {
                if( aDesc.maClassName != aEmptyNm )
                {
                    aSourceName = aDesc.maDisplayName;

                    if( aDesc.maClassName == aObjClassName )
                        aName = aObjName;
                    else
                        aName = aTypeName = aDesc.maTypeName;
                }
            }
            else if( SotClipboardFormatId::LINK_SOURCE == nFormat )
            {
                continue;
            }
            else if( aName.isEmpty() )
                aName = SvPasteObjectHelper::GetSotFormatUIName( nFormat );

            // Show RICHTEXT only in case RTF is not present.
            if (nFormat == SotClipboardFormatId::RICHTEXT &&
                std::any_of(pFormats->begin(), pFormats->end(),
                            [](const DataFlavorEx& rFlavor) {
                                return rFlavor.mnSotId == SotClipboardFormatId::RTF;
                            }))
            {
                continue;
            }

            if (ObjectLB().find_text(aName) == -1)
            {
                ObjectLB().append(OUString::number(static_cast<sal_uInt32>(nFormat)), aName);
            }
        }
    }

    if( aTypeName.isEmpty() && aSourceName.isEmpty() )
    {
        if( aDesc.maClassName != aEmptyNm )
        {
            aSourceName = aDesc.maDisplayName;
            aTypeName = aDesc.maTypeName;
        }

        if( aTypeName.isEmpty() && aSourceName.isEmpty() )
        {
            // global resource from svtools (former so3 resource)
            aSourceName = SvtResId(STR_UNKNOWN_SOURCE);
        }
    }

    if (!aExtraCommand.first.isEmpty())
    {
        ObjectLB().append(aExtraCommand.first, aExtraCommand.second);
    }

    ObjectLB().thaw();
    SelectObject();

    if( !aSourceName.isEmpty() )
    {
        if( !aTypeName.isEmpty() )
            aTypeName += "\n";

        aTypeName += aSourceName;
        aTypeName = convertLineEnd(aTypeName, GetSystemLineEnd());
    }

    m_xFtObjectSource->set_label(aTypeName);

    if (run() == RET_OK)
    {
        if (ObjectLB().get_selected_id().startsWithIgnoreAsciiCase(".uno"))
        {
            comphelper::dispatchCommand(aExtraCommand.first, {});
            nSelFormat = SotClipboardFormatId::NONE;
        }
        else
        {
            nSelFormat = static_cast<SotClipboardFormatId>(ObjectLB().get_selected_id().toUInt32());
        }
    }

    return nSelFormat;
}

void SvPasteObjectDialog::SetObjName( const SvGlobalName & rClass, const OUString & rObjName )
{
    aObjClassName = rClass;
    aObjName = rObjName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
