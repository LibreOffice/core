/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "editeng/fieldupdater.hxx"
#include "editeng/flditem.hxx"
#include "editobj2.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace editeng {

class FieldUpdaterImpl
{
    EditTextObjectImpl& mrObj;
public:
    FieldUpdaterImpl(EditTextObject& rObj) : mrObj(static_cast<EditTextObjectImpl&>(rObj)) {}
    FieldUpdaterImpl(const FieldUpdaterImpl& r) : mrObj(r.mrObj) {}

    void updateTableFields(int nTab)
    {
        SfxItemPool* pPool = mrObj.GetPool();
        EditTextObjectImpl::ContentInfosType& rContents = mrObj.GetContents();
        for (size_t i = 0; i < rContents.size(); ++i)
        {
            ContentInfo& rContent = rContents[i];
            ContentInfo::XEditAttributesType& rAttribs = rContent.GetAttribs();
            for (size_t j = 0; j < rAttribs.size(); ++j)
            {
                XEditAttribute& rAttr = rAttribs[j];
                const SfxPoolItem* pItem = rAttr.GetItem();
                if (pItem->Which() != EE_FEATURE_FIELD)
                    // This is not a field item.
                    continue;

                const SvxFieldItem* pFI = static_cast<const SvxFieldItem*>(pItem);
                const SvxFieldData* pData = pFI->GetField();
                if (pData->GetClassId() != text::textfield::Type::TABLE)
                    // This is not a table field.
                    continue;

                // Create a new table field with the new ID, and set it to the
                // attribute object.
                SvxFieldItem aNewItem(SvxTableField(nTab), EE_FEATURE_FIELD);
                rAttr.SetItem(pPool->Put(aNewItem));
            }
        }
    }
};

FieldUpdater::FieldUpdater(EditTextObject& rObj) : mpImpl(new FieldUpdaterImpl(rObj)) {}
FieldUpdater::FieldUpdater(const FieldUpdater& r) : mpImpl(new FieldUpdaterImpl(*r.mpImpl)) {}

FieldUpdater::~FieldUpdater()
{
    delete mpImpl;
}

void FieldUpdater::updateTableFields(int nTab)
{
    mpImpl->updateTableFields(nTab);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
